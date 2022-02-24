#include "backend/arm64/instruction-generating-arm64.h"
#include "backend/constants-pool.h"
#include "backend/linkage-symbols.h"
#include "backend/instruction.h"
#include "compiler/compiler.h"
#include "ir/base-test.h"
#include <gtest/gtest.h>

namespace yalx {
namespace backend {

class Arm64InstructionGeneratorTest : public ir::BaseTest {
public:
    Arm64InstructionGeneratorTest()
    : const_pool_(arena())
    , symbols_(arena()) {}

    void CodeGen(const char *project_dir,
                 std::string_view name,
                 base::ArenaMap<std::string_view, backend::InstructionFunction *> *funs,
                 bool *ok) {
        base::ArenaMap<std::string_view, ir::Module *> modules(arena());
        IRGen(project_dir, &modules, ok);
        if (!*ok) {
            return;
        }
        ASSERT_TRUE(modules.find(name) != modules.end());
        cpl::Compiler::SelectArm64InstructionCode(arena(), modules[name], &const_pool_, &symbols_, 1, funs);
    }
protected:
    ConstantsPool const_pool_;
    LinkageSymbols symbols_;
}; // class X64InstructionGeneratorTest

TEST_F(Arm64InstructionGeneratorTest, Sanity) {
    base::ArenaMap<std::string_view, backend::InstructionFunction *> funs(arena());
    // 40-code-gen-sanity
    bool ok = true;
    CodeGen("tests/40-code-gen-sanity", "main:main", &funs, &ok);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(funs.find("main:main.foo") != funs.end());
    auto fun = funs["main:main.foo"];
    ASSERT_STREQ("_main_Zomain_Zdfoo", fun->symbol()->data());
}

} // namespace backend
} // namespace yalx
