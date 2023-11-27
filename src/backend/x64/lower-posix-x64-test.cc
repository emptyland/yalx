#include "backend/x64/lower-posix-x64.h"
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

class X64PosixLowerTest : public ir::BaseTest {
public:
    X64PosixLowerTest(): const_pool_(&arena_), linkage_(&arena_), all_(&arena_) {}

    void SetUp() override {
        bool ok = true;
        IRGen("tests/32-code-lower", &all_, &ok);
        ASSERT_TRUE(ok);
    }

    InstructionFunction *IRLowing(ir::Function *fun) {
        X64PosixLower lower(&arena_, RegistersConfiguration::of_x64(), &linkage_, &const_pool_);
        return lower.VisitFunction(fun);
    }

    void CodeSlotAllocating(InstructionFunction *fun) {
        ZeroSlotAllocator allocator{arena(), RegistersConfiguration::of_x64(), fun};
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
    {dword $7}, {dword $6}, {ptr $2} = ArchFrameEnter (#32)
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
    {ptr $7} = ArchFrameEnter (#32)
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

} // namespace yalx::backend