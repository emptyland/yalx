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

} // namespace yalx::backend