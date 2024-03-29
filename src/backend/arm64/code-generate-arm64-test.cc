#include "backend/arm64/code-generate-arm64.h"
#include "backend/arm64/lower-posix-arm64.h"
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

class Arm64CodeGeneratorTest : public ir::BaseTest {
public:
    Arm64CodeGeneratorTest(): const_pool_(&arena_), linkage_(&arena_), all_(&arena_) {}

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
        Arm64CodeGenerator gen(funs, RegistersConfiguration::OfPosixArm64(), root, &const_pool_, &linkage_, &printer);
        gen.EmitFunction(fun);
        return buf;
    }

    InstructionFunction *IRLowing(ir::Function *fun) {
        Arm64PosixLower lower(&arena_, RegistersConfiguration::OfPosixArm64(), &linkage_,
                            &const_pool_, BarrierSet::OfYGCPosixArm64());
        return lower.VisitFunction(fun);
    }

    void CodeSlotAllocating(InstructionFunction *fun) {
        ZeroSlotAllocator allocator{arena(), RegistersConfiguration::OfPosixArm64(), fun};
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
}; // class Arm64CodeGeneratorTest

TEST_F(Arm64CodeGeneratorTest, Sanity) {
    auto expected = GenTo("main:main", "issue01_returning_one");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue01_returning_one
main_Zomain_Zdissue01_returning_one:
.cfi_startproc
Lblk0:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w19, #1
    str w19, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

TEST_F(Arm64CodeGeneratorTest, AddSubSanity) {
    auto expected = GenTo("main:main", "issue02_simple_add");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue02_simple_add
main_Zomain_Zdissue02_simple_add:
.cfi_startproc
Lblk0:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w1, #1
    add w0, w1, #2
    stur w0, [fp, #-4]
    ldur w19, [fp, #-4]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

// issue03_returning_two
TEST_F(Arm64CodeGeneratorTest, ReturningTwo) {
    auto expected = GenTo("main:main", "issue03_returning_two");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue03_returning_two
main_Zomain_Zdissue03_returning_two:
.cfi_startproc
Lblk0:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w19, #2
    str w19, [fp, #24]
    mov w19, #1
    str w19, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

TEST_F(Arm64CodeGeneratorTest, SimpleArgs2) {
    auto expected = GenTo("main:main", "issue04_simple_args");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue04_simple_args
main_Zomain_Zdissue04_simple_args:
.cfi_startproc
Lblk0:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur w0, [fp, #-4]
    stur w1, [fp, #-8]
    ldur w19, [fp, #-4]
    str w19, [fp, #24]
    ldur w19, [fp, #-8]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

TEST_F(Arm64CodeGeneratorTest, CallNonArgsFun) {
    auto expected = GenTo("main:main", "issue07_call_non_args_fun");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue07_call_non_args_fun
main_Zomain_Zdissue07_call_non_args_fun:
.cfi_startproc
Lblk0:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    add sp, sp, #16
    bl main_Zomain_Zdissue01_returning_one
    sub sp, sp, #16
    mov w1, #1
    ldur w2, [fp, #-4]
    add w0, w1, w2
    stur w0, [fp, #-20]
    ldur w19, [fp, #-20]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

TEST_F(Arm64CodeGeneratorTest, CallTwoArgsFun) {
    auto expected = GenTo("main:main", "issue08_call_two_args_fun");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue08_call_two_args_fun
main_Zomain_Zdissue08_call_two_args_fun:
.cfi_startproc
Lblk0:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    add sp, sp, #16
    mov w0, #1
    mov w1, #2
    bl main_Zomain_Zdissue04_simple_args
    sub sp, sp, #16
    ldur w1, [fp, #-4]
    ldur w2, [fp, #-8]
    add w0, w1, w2
    stur w0, [fp, #-20]
    ldur w19, [fp, #-20]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

TEST_F(Arm64CodeGeneratorTest, CallValArgsFun) {
    auto expected = GenTo("main:main", "issue09_call_val_args_fun");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue09_call_val_args_fun
main_Zomain_Zdissue09_call_val_args_fun:
.cfi_startproc
Lblk0:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    sub x0, fp, #24
    stur x0, [fp, #-32]
    add sp, sp, #32
    mov w1, #1
    mov w2, #2
    ldur x0, [fp, #-32]
    bl main_Zomain_ZdVertx2_ZdVertx2_Z4constructor
    sub sp, sp, #32
    leaq x0, [fp, #-24]
    bl main_Zomain_Zdissue06_returning_val
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

TEST_F(Arm64CodeGeneratorTest, GetValFields) {
    auto expected = GenTo("main:main", "issue10_get_fields");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue10_get_fields
main_Zomain_Zdissue10_get_fields:
.cfi_startproc
Lblk0:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    sub x0, fp, #24
    stur x0, [fp, #-32]
    add sp, sp, #16
    mov w1, #2
    mov w2, #3
    ldur x0, [fp, #-32]
    bl main_Zomain_ZdVertx2_ZdVertx2_Z4constructor
    sub sp, sp, #16
    ldur w0, [fp, #-8]
    stur w0, [fp, #-36]
    ldur w0, [fp, #-4]
    stur w0, [fp, #-40]
    ldur w1, [fp, #-36]
    ldur w2, [fp, #-40]
    add w0, w1, w2
    stur w0, [fp, #-44]
    ldur w19, [fp, #-44]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

// issue11_overflow_args
TEST_F(Arm64CodeGeneratorTest, OverflowArgsFun) {
    auto expected = GenTo("main:main", "issue11_overflow_args");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue11_overflow_args
main_Zomain_Zdissue11_overflow_args:
.cfi_startproc
Lblk0:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur w0, [fp, #-4]
    stur w1, [fp, #-8]
    stur w2, [fp, #-12]
    stur w3, [fp, #-16]
    stur w4, [fp, #-20]
    stur w5, [fp, #-24]
    stur w6, [fp, #-28]
    stur w7, [fp, #-32]
    ldur w1, [fp, #-4]
    ldur w2, [fp, #-8]
    add w0, w1, w2
    stur w0, [fp, #-36]
    ldur w1, [fp, #-32]
    ldr w2, [fp, #20]
    add w0, w1, w2
    stur w0, [fp, #-40]
    ldur w19, [fp, #-40]
    str w19, [fp, #24]
    ldur w19, [fp, #-36]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

// issue12_call_overflow_args_fun
TEST_F(Arm64CodeGeneratorTest, CallOverflowArgsFun) {
    auto expected = GenTo("main:main", "issue12_call_overflow_args_fun");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue12_call_overflow_args_fun
main_Zomain_Zdissue12_call_overflow_args_fun:
.cfi_startproc
Lblk0:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    add sp, sp, #16
    mov w0, #1
    mov w1, #2
    mov w2, #3
    mov w3, #4
    mov w4, #5
    mov w5, #6
    mov w6, #7
    mov w7, #8
    mov w19, #9
    stur w19, [fp, #-12]
    bl main_Zomain_Zdissue11_overflow_args
    sub sp, sp, #16
    ldur w1, [fp, #-4]
    ldur w2, [fp, #-8]
    add w0, w1, w2
    stur w0, [fp, #-20]
    ldur w19, [fp, #-20]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

// issue13_simple_load_barrier
TEST_F(Arm64CodeGeneratorTest, SimpleLoadBarrier) {
    auto expected = GenTo("main:main", "issue13_simple_load_barrier");
    static constexpr char z[] = R"(.global main_Zomain_Zdissue13_simple_load_barrier
main_Zomain_Zdissue13_simple_load_barrier:
.cfi_startproc
Lblk0:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    sub x0, fp, #32
    stur x0, [fp, #-40]
    add sp, sp, #16
    adrp x19, Kstr.0@PAGE
    add x19, x19, Kstr.0@PAGEOFF
    ldr x1, [x19, #0]
    adrp x19, Kstr.1@PAGE
    add x19, x19, Kstr.1@PAGEOFF
    ldr x2, [x19, #0]
    mov w3, #0
    ldur x0, [fp, #-40]
    bl main_Zomain_ZdIdent2_ZdIdent2_Z4constructor
    sub sp, sp, #16
    ldur x0, [fp, #-16]
    stur x0, [fp, #-56]
    adrp x19, YGC_ADDRESS_BAD_MASK@PAGE
    add x19, x19, YGC_ADDRESS_BAD_MASK@PAGEOFF
    ldr x19, [x19, #0]
    ldur x0, [fp, #-56]
    tst x19, x0
    b.eq Jpt_0
    sub sp, sp, #32
    str x0, [sp, #0]
    str x0, [sp, #8]
    str x1, [sp, #16]
    str x26, [sp, #24]
    sub x0, fp, #32
    bl ygc_barrier_load_on_field
    stur x0, [fp, #-56]
    ldr x0, [sp, #0]
    ldr x0, [sp, #8]
    ldr x1, [sp, #16]
    ldr x26, [sp, #24]
    add sp, sp, #32
Jpt_0:
    ldur x19, [fp, #-56]
    str x19, [fp, #24]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.cfi_endproc
)";
    ASSERT_EQ(z, expected) << expected;
}

} // namespace yalx::backend