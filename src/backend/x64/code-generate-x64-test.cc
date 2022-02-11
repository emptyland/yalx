#include "backend/x64/code-generate-x64.h"
#include "backend/constants-pool.h"
#include "backend/linkage-symbols.h"
#include "backend/instruction.h"
#include "compiler/compiler.h"
#include "ir/base-test.h"
#include "base/io.h"
#include <gtest/gtest.h>

namespace yalx {
namespace backend {

class X64CodeGeneratorTest : public ir::BaseTest {
public:
    X64CodeGeneratorTest()
    : const_pool_(arena())
    , symbols_(arena()) {}

    void CodeGen(const char *project_dir, std::string_view name, base::PrintingWriter *printer, bool *ok) {
        base::ArenaMap<std::string_view, ir::Module *> modules(arena());
        IRGen(project_dir, &modules, ok);
        if (!*ok) {
            return;
        }
        ASSERT_TRUE(modules.find(name) != modules.end());
        base::ArenaMap<std::string_view, backend::InstructionFunction *> funs(arena());
        cpl::Compiler::SelectX64InstructionCode(arena(), modules[name], &const_pool_, &symbols_, 1, &funs);
        cpl::Compiler::GenerateX64InstructionCode(funs, modules[name], &const_pool_, &symbols_, printer);
    }
protected:
    ConstantsPool const_pool_;
    LinkageSymbols symbols_;
}; // class X64InstructionGeneratorTest

TEST_F(X64CodeGeneratorTest, Sanity) {
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    bool ok = true;
    CodeGen("tests/40-code-gen-sanity", "main:main", &printer, &ok);
    ASSERT_TRUE(ok);
    printf("%s\n", buf.c_str());
}

} // namespace backend
} // namespace yalx
