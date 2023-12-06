#include "backend/x64/code-generate-x64.h"
#include "backend/x64/lower-posix-x64.h"
#include "backend/barrier-set.h"
#include "backend/constants-pool.h"
#include "backend/linkage-symbols.h"
#include "backend/registers-configuration.h"
#include "backend/zero-slot-allocator.h"
#include "ir/node.h"
#include "ir/type.h"
#include "ir/operator.h"
#include "ir/utils.h"
#include "ir/base-test.h"
#include "base/io.h"

namespace yalx::backend {

class X64CodeGeneratorTest : public ir::BaseTest {
public:
    X64CodeGeneratorTest(): const_pool_(&arena_), linkage_(&arena_), all_(&arena_) {}

    void SetUp() override {
        bool ok = true;
        IRGen("tests/32-code-lower", &all_, &ok);
        ASSERT_TRUE(ok);
    }

    std::string GenTo(const char *pkg_name, const char *name) {
        auto mod = FindModuleOrNull(pkg_name);
        auto ir_fun = mod->FindFunOrNull(name);
        auto lo_fun = IRLowing(ir_fun);
        CodeSlotAllocating(lo_fun);
        return GenTo(mod, lo_fun);
    }

    std::string GenTo(ir::Module *root, InstructionFunction *fun) {
        base::ArenaMap<std::string_view, InstructionFunction *> funs(arena());
        funs[fun->symbol()->ToSlice()] = fun;

        std::string buf;
        auto file = base::NewMemoryWritableFile(&buf);
        base::PrintingWriter printer{file, true};
        X64CodeGenerator gen(funs, root, &const_pool_, &linkage_, &printer);
        gen.EmitFunction(fun);
        return buf;
    }

    InstructionFunction *IRLowing(ir::Function *fun) {
        X64PosixLower lower(&arena_, RegistersConfiguration::OfPosixX64(), &linkage_,
                            &const_pool_, BarrierSet::OfYGCPosixX64());
        return lower.VisitFunction(fun);
    }

    void CodeSlotAllocating(InstructionFunction *fun) {
        ZeroSlotAllocator allocator{arena(), RegistersConfiguration::OfPosixX64(), fun};
        allocator.Run();
    }

    ir::Module *FindModuleOrNull(std::string_view full_name) const {
        if (auto iter = all_.find(full_name); iter != all_.end()) {
            return iter->second;
        } else {
            return nullptr;
        }
    }

    static std::string PrintTo(InstructionFunction *fun) {
        std::string buf;
        auto file = base::NewMemoryWritableFile(&buf);
        base::PrintingWriter printer{file, true};
        fun->PrintTo(&printer);
        return buf;
    }

protected:
    Linkage linkage_;
    ConstantsPool const_pool_;
    base::ArenaMap<std::string_view, ir::Module *> all_;
}; // class X64CodeGeneratorTest

TEST_F(X64CodeGeneratorTest, Sanity) {
    auto expected = GenTo("main:main", "issue01_returning_one");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue01_returning_one
main_Zomain_Zdissue01_returning_one:
.cfi_startproc
Lblk0:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl $1, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

// issue02_simple_add AddSubSanity
TEST_F(X64CodeGeneratorTest, AddSubSanity) {
    auto expected = GenTo("main:main", "issue02_simple_add");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue02_simple_add
main_Zomain_Zdissue02_simple_add:
.cfi_startproc
Lblk0:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $16, %rsp
    movl $1, %eax
    addl $2, %eax
    movl %eax, -4(%rbp)
    movl -4(%rbp), %r13d
    movl %r13d, 28(%rbp)
    addq $16, %rsp
    popq %rbp
    retq
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

TEST_F(X64CodeGeneratorTest, CallValArgsFun) {
    auto expected = GenTo("main:main", "issue09_call_val_args_fun");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue09_call_val_args_fun
main_Zomain_Zdissue09_call_val_args_fun:
.cfi_startproc
Lblk0:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $64, %rsp
    leaq -24(%rbp), %rax
    movq %rax, -32(%rbp)
    addq $32, %rsp
    movl $1, %esi
    movl $2, %edx
    movq -32(%rbp), %rdi
    callq main_Zomain_ZdVertx2_ZdVertx2_Z4constructor
    subq $32, %rsp
    leaq -24(%rbp), %rdi
    callq main_Zomain_Zdissue06_returning_val
    addq $64, %rsp
    popq %rbp
    retq
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

// auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue10_get_fields");
TEST_F(X64CodeGeneratorTest, GetValFields) {
    auto expected = GenTo("main:main", "issue10_get_fields");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue10_get_fields
main_Zomain_Zdissue10_get_fields:
.cfi_startproc
Lblk0:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $48, %rsp
    leaq -24(%rbp), %rax
    movq %rax, -32(%rbp)
    addq $16, %rsp
    movl $2, %esi
    movl $3, %edx
    movq -32(%rbp), %rdi
    callq main_Zomain_ZdVertx2_ZdVertx2_Z4constructor
    subq $16, %rsp
    movl -8(%rbp), %eax
    movl %eax, -36(%rbp)
    movl -4(%rbp), %eax
    movl %eax, -40(%rbp)
    movl -36(%rbp), %eax
    addl -40(%rbp), %eax
    movl %eax, -44(%rbp)
    movl -44(%rbp), %r13d
    movl %r13d, 28(%rbp)
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

// FindModuleOrNull("main:main")->FindFunOrNull("issue13_simple_load_barrier");
TEST_F(X64CodeGeneratorTest, SimpleLoadBarrier) {
    auto expected = GenTo("main:main", "issue13_simple_load_barrier");
    puts(expected.c_str());
}

} // namespace yalx::backend


