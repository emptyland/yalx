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
        //auto rs = GenIRAll("tests/32-code-lower");
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

    puts(PrintTo(ir_fun).c_str());

    auto lo_fun = IRLowing(ir_fun);
    ASSERT_TRUE(lo_fun != nullptr);

#if defined(YALX_OS_LINUX)
    ASSERT_STREQ("main_Zomain_Zdissue01_returning_one", lo_fun->symbol()->data());
#endif

#if defined(YALX_OS_DARWIN)
    ASSERT_STREQ("_main_Zomain_Zdissue01_returning_one", lo_fun->symbol()->data());
#endif

    CodeSlotAllocating(lo_fun);
    puts(PrintTo(lo_fun).c_str());
}

} // namespace yalx::backend