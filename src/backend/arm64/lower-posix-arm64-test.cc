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
        Arm64PosixLower lower(&arena_, RegistersConfiguration::OfPosixArm64(), &linkage_, &const_pool_,
                            BarrierSet::OfYGCPosixArm64());
        return lower.VisitFunction(fun);
    }

    void CodeSlotAllocating(InstructionFunction *fun) {
        ZeroSlotAllocator allocator{arena(), RegistersConfiguration::OfPosixArm64(), fun};
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

TEST_F(Arm64PosixLowerTest, SimpleArgs2) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue04_simple_args");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue04_simple_args:
L0:
    {dword $0}, {dword $1} = ArchFrameEnter (#16)
    Move {dword fp-4} <- {dword $0}
    Move {dword fp-8} <- {dword $1}
    Move {dword fp+24} <- {dword fp-4}
    Move {dword fp+28} <- {dword fp-8}
    ArchFrameExit {dword fp-8}, {dword fp-4}(#16)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

TEST_F(Arm64PosixLowerTest, SimpleArgs3) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue05_simple_args");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue05_simple_args:
L0:
    {dword $0}, {dword $1}, {none $2} = ArchFrameEnter (#32)
    Move {dword fp-4} <- {dword $0}
    Move {dword fp-8} <- {dword $1}
    Move {qword fp-32} <- {qword $2+0} // move value: main:main.Vertx2
    Move {qword fp-24} <- {qword $2+8} // move value: main:main.Vertx2
    Move {qword fp-16} <- {qword $2+16} // move value: main:main.Vertx2
    Move {dword fp+24} <- {dword fp-8}
    Move {dword fp+28} <- {dword fp-4}
    ArchFrameExit {dword fp-4}, {dword fp-8}(#32)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

TEST_F(Arm64PosixLowerTest, ReturnVal1) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue06_returning_val");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue06_returning_val:
L0:
    {none $0} = ArchFrameEnter (#32)
    Move {qword fp-24} <- {qword $0+0} // move value: main:main.Vertx2
    Move {qword fp-16} <- {qword $0+8} // move value: main:main.Vertx2
    Move {qword fp-8} <- {qword $0+16} // move value: main:main.Vertx2
    Move {qword fp+24} <- {qword fp-24} // move value: main:main.Vertx2
    Move {qword fp+32} <- {qword fp-16} // move value: main:main.Vertx2
    Move {qword fp+40} <- {qword fp-8} // move value: main:main.Vertx2
    ArchFrameExit {none fp-24}(#32)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

TEST_F(Arm64PosixLowerTest, CallNonArgsFun) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue07_call_non_args_fun");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue07_call_non_args_fun:
L0:
    ArchFrameEnter (#32)
    ArchBeforeCall (#16, #0, #0)
    {dword fp-4} = ArchCall (<main_Zomain_Zdissue01_returning_one>, #4)
    ArchAfterCall (#16, #0, #0)
    Move {dword $1} <- #1
    Move {dword $2} <- {dword fp-4}
    {dword $0} = Arm64Add {dword $1}, {dword $2}
    Move {dword fp-20} <- {dword $0}
    Move {dword fp+28} <- {dword fp-20}
    ArchFrameExit {dword fp-20}(#32)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

TEST_F(Arm64PosixLowerTest, CallTwoArgsFun) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue08_call_two_args_fun");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue08_call_two_args_fun:
L0:
    ArchFrameEnter (#32)
    ArchBeforeCall (#16, #0, #0)
    Move {dword $0} <- #1
    Move {dword $1} <- #2
    {dword fp-4}, {dword fp-8} = ArchCall {dword $0}, {dword $1}(<main_Zomain_Zdissue04_simple_args>, #8)
    ArchAfterCall (#16, #0, #0)
    Move {dword $1} <- {dword fp-4}
    Move {dword $2} <- {dword fp-8}
    {dword $0} = Arm64Add {dword $1}, {dword $2}
    Move {dword fp-20} <- {dword $0}
    Move {dword fp+28} <- {dword fp-20}
    ArchFrameExit {dword fp-20}(#32)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

TEST_F(Arm64PosixLowerTest, CallValArgsFun) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue09_call_val_args_fun");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue09_call_val_args_fun:
L0:
    ArchFrameEnter (#64)
    {ptr fp-24} = ArchStackAlloc #24
    {ptr $0} = ArchLoadEffectAddress {none fp-24}
    Move {ptr fp-32} <- {ptr $0}
    ArchBeforeCall (#32, #0, #32)
    Move {dword $1} <- #1
    Move {dword $2} <- #2
    Move {ptr $0} <- {ptr fp-32}
    ArchCall {ptr $0}, {dword $1}, {dword $2}(<main_Zomain_ZdVertx2_ZdVertx2_Z4constructor>, #0)
    ArchAfterCall (#32, #0, #32)
    ArchBeforeCall (#0, #0, #32)
    Move {none $0} <- &{none fp-24}
    {ptr fp-56} = ArchCall {none $0}(<main_Zomain_Zdissue06_returning_val>, #24)
    ArchAfterCall (#0, #0, #32)
    ArchFrameExit (#64)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

TEST_F(Arm64PosixLowerTest, GetValFields) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue10_get_fields");
    ASSERT_TRUE(ir_fun != nullptr);

    //puts(PrintTo(ir_fun).c_str());

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue10_get_fields:
L0:
    ArchFrameEnter (#48)
    {ptr fp-24} = ArchStackAlloc #24
    {ptr $0} = ArchLoadEffectAddress {none fp-24}
    Move {ptr fp-32} <- {ptr $0}
    ArchBeforeCall (#16, #0, #32)
    Move {dword $1} <- #2
    Move {dword $2} <- #3
    Move {ptr $0} <- {ptr fp-32}
    ArchCall {ptr $0}, {dword $1}, {dword $2}(<main_Zomain_ZdVertx2_ZdVertx2_Z4constructor>, #0)
    ArchAfterCall (#16, #0, #32)
    {dword $0} = ArchStackLoad {none fp-24}, #16
    Move {dword fp-36} <- {dword $0}
    {dword $0} = ArchStackLoad {none fp-24}, #20
    Move {dword fp-40} <- {dword $0}
    Move {dword $1} <- {dword fp-36}
    Move {dword $2} <- {dword fp-40}
    {dword $0} = Arm64Add {dword $1}, {dword $2}
    Move {dword fp-44} <- {dword $0}
    Move {dword fp+28} <- {dword fp-44}
    ArchFrameExit {dword fp-44}(#48)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

TEST_F(Arm64PosixLowerTest, OverflowArgsFun) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue11_overflow_args");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue11_overflow_args:
L0:
    {dword $0}, {dword $1}, {dword $2}, {dword $3}, {dword $4}, {dword $5}, {dword $6}, {dword $7}, {dword fp+20} = ArchFrameEnter (#48)
    Move {dword fp-4} <- {dword $0}
    Move {dword fp-8} <- {dword $1}
    Move {dword fp-12} <- {dword $2}
    Move {dword fp-16} <- {dword $3}
    Move {dword fp-20} <- {dword $4}
    Move {dword fp-24} <- {dword $5}
    Move {dword fp-28} <- {dword $6}
    Move {dword fp-32} <- {dword $7}
    Move {dword $1} <- {dword fp-4}
    Move {dword $2} <- {dword fp-8}
    {dword $0} = Arm64Add {dword $1}, {dword $2}
    Move {dword fp-36} <- {dword $0}
    Move {dword $1} <- {dword fp-32}
    Move {dword $2} <- {dword fp+20}
    {dword $0} = Arm64Add {dword $1}, {dword $2}
    Move {dword fp-40} <- {dword $0}
    Move {dword fp+24} <- {dword fp-40}
    Move {dword fp+28} <- {dword fp-36}
    ArchFrameExit {dword fp-36}, {dword fp-40}(#48)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

TEST_F(Arm64PosixLowerTest, CallOverflowArgsFun) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue12_call_overflow_args_fun");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue12_call_overflow_args_fun:
L0:
    ArchFrameEnter (#32)
    ArchBeforeCall (#16, #4, #0)
    Move {dword $0} <- #1
    Move {dword $1} <- #2
    Move {dword $2} <- #3
    Move {dword $3} <- #4
    Move {dword $4} <- #5
    Move {dword $5} <- #6
    Move {dword $6} <- #7
    Move {dword $7} <- #8
    Move {dword fp-12} <- #9
    {dword fp-4}, {dword fp-8} = ArchCall {dword $0}, {dword $1}, {dword $2}, {dword $3}, {dword $4}, {dword $5}, {dword $6}, {dword $7}, {dword fp-12}(<main_Zomain_Zdissue11_overflow_args>, #12)
    ArchAfterCall (#16, #4, #0)
    Move {dword $1} <- {dword fp-4}
    Move {dword $2} <- {dword fp-8}
    {dword $0} = Arm64Add {dword $1}, {dword $2}
    Move {dword fp-20} <- {dword $0}
    Move {dword fp+28} <- {dword fp-20}
    ArchFrameExit {dword fp-20}(#32)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

TEST_F(Arm64PosixLowerTest, SimpleLoadBarrier) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue13_simple_load_barrier");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue13_simple_load_barrier:
L0:
    ArchFrameEnter (#64)
    {ptr fp-32} = ArchStackAlloc #32
    {ptr $0} = ArchLoadEffectAddress {none fp-32}
    Move {ptr fp-40} <- {ptr $0}
    ArchBeforeCall (#16, #0, #40)
    Move {ref $1} <- <literals:0>
    Move {ref $2} <- <literals:1>
    Move {dword $3} <- #0
    Move {ptr $0} <- {ptr fp-40}
    ArchCall {ptr $0}, {ref $1}, {ref $2}, {dword $3}(<main_Zomain_ZdIdent2_ZdIdent2_Z4constructor>, #0)
    ArchAfterCall (#16, #0, #40)
    {ref $0} = ArchStackLoad {none fp-32}, #16
    Move {ref fp-56} <- {ref $0}
    {qword $19} = Arm64Adrp <YGC_ADDRESS_BAD_MASK>
    {qword $19} = Arm64AddOff {qword $19}, <YGC_ADDRESS_BAD_MASK>
    {qword $19} = Arm64Ldr {qword $19+0}
    Move {ref $0} <- {ref fp-56}
    {qword $19} = Arm64Tst {ref $0}
    Arm64B_eq <Jpt_0>
    ArchBeforeCall {qword $0}, {qword $0}, {qword $1}, {qword $26}
    {ptr $0} = ArchLoadEffectAddress {none fp-32}, #16
    {ref $0} = ArchCallNative <ygc_barrier_load_on_field>, {ptr $0}
    Move {ref fp-56} <- {ref $0}
    ArchAfterCall {qword $0}, {qword $0}, {qword $1}, {qword $26}
Jpt_0:
    Move {ref fp+24} <- {ref fp-56}
    ArchFrameExit {ref fp-56}(#64)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

} // namespace yalx::backend