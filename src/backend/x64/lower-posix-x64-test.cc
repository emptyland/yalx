#include "backend/x64/lower-posix-x64.h"
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

class X64PosixLowerTest : public ir::BaseTest {
public:
    X64PosixLowerTest(): const_pool_(&arena_), linkage_(&arena_), all_(&arena_) {}

    void SetUp() override {
        bool ok = true;
        IRGen("tests/32-code-lower", &all_, &ok);
        ASSERT_TRUE(ok);
    }

    InstructionFunction *IRLowing(ir::Function *fun) {
        X64PosixLower lower(&arena_, RegistersConfiguration::OfPosixX64(), &linkage_, &const_pool_,
                            BarrierSet::OfYGCPosixX64());
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
};

TEST_F(X64PosixLowerTest, Sanity) {

    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue01_returning_one");
    ASSERT_TRUE(ir_fun != nullptr);

    //puts(PrintTo(ir_fun).c_str());

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

TEST_F(X64PosixLowerTest, AddSubSanity) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue02_simple_add");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    //puts(PrintTo(lo_fun).c_str());
    static constexpr char z[] = R"(main_Zomain_Zdissue02_simple_add:
L0:
    ArchFrameEnter (#16)
    Move {dword $0} <- #1
    {dword $0} = X64Add32 #2
    Move {dword fp-4} <- {dword $0}
    Move {dword fp+28} <- {dword fp-4}
    ArchFrameExit {dword fp-4}(#16)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

// issue03_returning_two
TEST_F(X64PosixLowerTest, ReturningTwo) {
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

// issue04_simple_args
TEST_F(X64PosixLowerTest, SimpleArgs2) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue04_simple_args");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue04_simple_args:
L0:
    {dword $7}, {dword $6} = ArchFrameEnter (#16)
    Move {dword fp-4} <- {dword $7}
    Move {dword fp-8} <- {dword $6}
    Move {dword fp+24} <- {dword fp-4}
    Move {dword fp+28} <- {dword fp-8}
    ArchFrameExit {dword fp-8}, {dword fp-4}(#16)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

// issue05_simple_args
TEST_F(X64PosixLowerTest, SimpleArgs3) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue05_simple_args");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue05_simple_args:
L0:
    {dword $7}, {dword $6}, {none $2} = ArchFrameEnter (#32)
    Move {dword fp-4} <- {dword $7}
    Move {dword fp-8} <- {dword $6}
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

TEST_F(X64PosixLowerTest, ReturnVal1) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue06_returning_val");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue06_returning_val:
L0:
    {none $7} = ArchFrameEnter (#32)
    Move {qword fp-24} <- {qword $7+0} // move value: main:main.Vertx2
    Move {qword fp-16} <- {qword $7+8} // move value: main:main.Vertx2
    Move {qword fp-8} <- {qword $7+16} // move value: main:main.Vertx2
    Move {qword fp+24} <- {qword fp-24} // move value: main:main.Vertx2
    Move {qword fp+32} <- {qword fp-16} // move value: main:main.Vertx2
    Move {qword fp+40} <- {qword fp-8} // move value: main:main.Vertx2
    ArchFrameExit {none fp-24}(#32)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

// issue07_call_non_args_fun
TEST_F(X64PosixLowerTest, CallNonArgsFun) {
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
    Move {dword $0} <- #1
    {dword $0} = X64Add32 {dword fp-4}
    Move {dword fp-20} <- {dword $0}
    Move {dword fp+28} <- {dword fp-20}
    ArchFrameExit {dword fp-20}(#32)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

// issue08_call_two_args_fun
TEST_F(X64PosixLowerTest, CallTwoArgsFun) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue08_call_two_args_fun");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue08_call_two_args_fun:
L0:
    ArchFrameEnter (#32)
    ArchBeforeCall (#16, #0, #0)
    Move {dword $7} <- #1
    Move {dword $6} <- #2
    {dword fp-4}, {dword fp-8} = ArchCall {dword $7}, {dword $6}(<main_Zomain_Zdissue04_simple_args>, #8)
    ArchAfterCall (#16, #0, #0)
    Move {dword $0} <- {dword fp-4}
    {dword $0} = X64Add32 {dword fp-8}
    Move {dword fp-20} <- {dword $0}
    Move {dword fp+28} <- {dword fp-20}
    ArchFrameExit {dword fp-20}(#32)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

// issue09_call_val_args_fun
TEST_F(X64PosixLowerTest, CallValArgsFun) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue09_call_val_args_fun");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue09_call_val_args_fun:
L0:
    ArchFrameEnter (#64)
    {ptr fp-24} = ArchStackAlloc #24
    {ptr $0} = X64Lea {none fp-24}
    Move {ptr fp-32} <- {ptr $0}
    ArchBeforeCall (#32, #0, #32)
    Move {dword $6} <- #1
    Move {dword $2} <- #2
    Move {ptr $7} <- {ptr fp-32}
    ArchCall {ptr $7}, {dword $6}, {dword $2}(<main_Zomain_ZdVertx2_ZdVertx2_Z4constructor>, #0)
    ArchAfterCall (#32, #0, #32)
    ArchBeforeCall (#0, #0, #32)
    Move {none $7} <- &{none fp-24}
    {ptr fp-56} = ArchCall {none $7}(<main_Zomain_Zdissue06_returning_val>, #24)
    ArchAfterCall (#0, #0, #32)
    ArchFrameExit (#64)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

// issue10_get_fields
TEST_F(X64PosixLowerTest, GetValFields) {
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
    {ptr $0} = X64Lea {none fp-24}
    Move {ptr fp-32} <- {ptr $0}
    ArchBeforeCall (#16, #0, #32)
    Move {dword $6} <- #2
    Move {dword $2} <- #3
    Move {ptr $7} <- {ptr fp-32}
    ArchCall {ptr $7}, {dword $6}, {dword $2}(<main_Zomain_ZdVertx2_ZdVertx2_Z4constructor>, #0)
    ArchAfterCall (#16, #0, #32)
    {dword $0} = ArchStackLoad {none fp-24}, #16
    Move {dword fp-36} <- {dword $0}
    {dword $0} = ArchStackLoad {none fp-24}, #20
    Move {dword fp-40} <- {dword $0}
    Move {dword $0} <- {dword fp-36}
    {dword $0} = X64Add32 {dword fp-40}
    Move {dword fp-44} <- {dword $0}
    Move {dword fp+28} <- {dword fp-44}
    ArchFrameExit {dword fp-44}(#48)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

// issue11_overflow_args
TEST_F(X64PosixLowerTest, OverflowArgsFun) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue11_overflow_args");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue11_overflow_args:
L0:
    {dword $7}, {dword $6}, {dword $2}, {dword $1}, {dword $8}, {dword $9}, {dword $10}, {dword $11}, {dword fp+20} = ArchFrameEnter (#48)
    Move {dword fp-4} <- {dword $7}
    Move {dword fp-8} <- {dword $6}
    Move {dword fp-12} <- {dword $2}
    Move {dword fp-16} <- {dword $1}
    Move {dword fp-20} <- {dword $8}
    Move {dword fp-24} <- {dword $9}
    Move {dword fp-28} <- {dword $10}
    Move {dword fp-32} <- {dword $11}
    Move {dword $0} <- {dword fp-4}
    {dword $0} = X64Add32 {dword fp-8}
    Move {dword fp-36} <- {dword $0}
    Move {dword $0} <- {dword fp-32}
    {dword $0} = X64Add32 {dword fp+20}
    Move {dword fp-40} <- {dword $0}
    Move {dword fp+24} <- {dword fp-40}
    Move {dword fp+28} <- {dword fp-36}
    ArchFrameExit {dword fp-36}, {dword fp-40}(#48)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

// issue12_call_overflow_args_fun
TEST_F(X64PosixLowerTest, CallOverflowArgsFun) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue12_call_overflow_args_fun");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    static constexpr char z[] = R"(main_Zomain_Zdissue12_call_overflow_args_fun:
L0:
    ArchFrameEnter (#32)
    ArchBeforeCall (#16, #4, #0)
    Move {dword $7} <- #1
    Move {dword $6} <- #2
    Move {dword $2} <- #3
    Move {dword $1} <- #4
    Move {dword $8} <- #5
    Move {dword $9} <- #6
    Move {dword $10} <- #7
    Move {dword $11} <- #8
    Move {dword fp-12} <- #9
    {dword fp-4}, {dword fp-8} = ArchCall {dword $7}, {dword $6}, {dword $2}, {dword $1}, {dword $8}, {dword $9}, {dword $10}, {dword $11}, {dword fp-12}(<main_Zomain_Zdissue11_overflow_args>, #12)
    ArchAfterCall (#16, #4, #0)
    Move {dword $0} <- {dword fp-4}
    {dword $0} = X64Add32 {dword fp-8}
    Move {dword fp-20} <- {dword $0}
    Move {dword fp+28} <- {dword fp-20}
    ArchFrameExit {dword fp-20}(#32)
)";
    auto expected = PrintTo(lo_fun);
    EXPECT_EQ(z, expected) << expected;
}

// issue13_simple_load_barrier
TEST_F(X64PosixLowerTest, SimpleLoadBarrier) {
    auto ir_fun = FindModuleOrNull("main:main")->FindFunOrNull("issue13_simple_load_barrier");
    ASSERT_TRUE(ir_fun != nullptr);

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

    CodeSlotAllocating(lo_fun);
    puts(PrintTo(lo_fun).c_str());
}

} // namespace yalx::backend