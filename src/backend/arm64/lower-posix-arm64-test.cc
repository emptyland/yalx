#include "backend/arm64/lower-posix-arm64.h"
#include "backend/constants-pool.h"
#include "backend/linkage-symbols.h"
#include "backend/barrier-set.h"
#include "backend/registers-configuration.h"
#include "backend/zero-slot-allocator.h"
#include "ir/node.h"
#include "ir/type.h"
#include "ir/operator.h"
#include "ir/utils.h"
#include "ir/base-test.h"
#include "base/io.h"

namespace yalx::backend {

class Arm64PosixLowerTest : public ir::BaseTest {
public:
    Arm64PosixLowerTest(): const_pool_(&arena_), linkage_(&arena_), all_(&arena_) {}

    void SetUp() override {
        bool ok = true;
        IRGen("tests/32-code-lower", &all_, &ok);
        ASSERT_TRUE(ok);
    }

    InstructionFunction *IRLowing(ir::Function *fun) {
        Arm64PosixLower lower(&arena_, RegistersConfiguration::OfPosixX64(), &linkage_, &const_pool_,
                            BarrierSet::OfYGCPosixArm64());
        return lower.VisitFunction(fun);
    }

    void CodeSlotAllocating(InstructionFunction *fun) {
        ZeroSlotAllocator allocator{arena(), RegistersConfiguration::OfPosixX64(), fun};
        allocator.Run();
    }

    static std::string PrintTo(InstructionFunction *fun) {
        std::string buf;
        auto file = base::NewMemoryWritableFile(&buf);
        base::PrintingWriter printer{file, true};
        fun->PrintTo(&printer);
        return buf;
    }

    static std::string PrintTo(ir::Function *fun) {
        std::string buf;
        auto file = base::NewMemoryWritableFile(&buf);
        base::PrintingWriter printer{file, true};
        ir::PrintingContext ctx{0};
        fun->PrintTo(&ctx, &printer);
        return buf;
    }

    ir::Module *FindModuleOrNull(std::string_view full_name) const {
        if (auto iter = all_.find(full_name); iter != all_.end()) {
            return iter->second;
        } else {
            return nullptr;
        }
    }

protected:
    Linkage linkage_;
    ConstantsPool const_pool_;
    base::ArenaMap<std::string_view, ir::Module *> all_;
}; // class Arm64PosixLowerTest

TEST_F(Arm64PosixLowerTest, Sanity) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue01_returning_one");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

#if defined(YALX_OS_LINUX)
    ASSERT_STREQ("main_Zomain_Zdissue01_returning_one", lo_fun->symbol()->data());
#endif

#if defined(YALX_OS_DARWIN)
    ASSERT_STREQ("_main_Zomain_Zdissue01_returning_one", lo_fun->symbol()->data());
#endif

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue01_returning_one:
L0:
    ArchFrameEnter (#0)
    Move {dword fp+28} <- #1
    ArchFrameExit #1(#0)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

TEST_F(Arm64PosixLowerTest, AddSubSanity) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue02_simple_add");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    //puts(PrintTo(lo_fun).c_str());
    static constexpr char z[] = R"(main_Zomain_Zdissue02_simple_add:
L0:
    ArchFrameEnter (#16)
    Move {dword $1} <- #1
    {dword $0} = Arm64Add {dword $1}, #2
    Move {dword fp-4} <- {dword $0}
    Move {dword fp+28} <- {dword fp-4}
    ArchFrameExit {dword fp-4}(#16)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

TEST_F(Arm64PosixLowerTest, ReturningTwo) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue03_returning_two");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue03_returning_two:
L0:
    ArchFrameEnter (#0)
    Move {dword fp+24} <- #2
    Move {dword fp+28} <- #1
    ArchFrameExit #1, #2(#0)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

} // namespace yalx::backend