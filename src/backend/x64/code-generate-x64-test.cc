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

extern "C" void call_returning_vals(void *returnning_vals, size_t size_in_bytes, void *yalx_fun);
extern "C" void main_Zomain_Zdissue1();
extern "C" void main_Zomain_Zdfoo();

TEST_F(X64CodeGeneratorTest, Sanity) {
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    bool ok = true;
    CodeGen("tests/40-code-gen-sanity", "main:main", &printer, &ok);
    ASSERT_TRUE(ok);
    printf("%s\n", buf.c_str());
}

TEST_F(X64CodeGeneratorTest, ReturningVals) {
    int buf[4] = {0};
    printf("%p\n", main_Zomain_Zdfoo);
    call_returning_vals(buf, arraysize(buf) * sizeof(buf[0]), reinterpret_cast<void *>(&main_Zomain_Zdfoo));
    memset(buf, 0, sizeof(buf));
    call_returning_vals(buf, arraysize(buf) * sizeof(buf[0]), reinterpret_cast<void *>(&main_Zomain_Zdissue1));
    printf("ok\n");
}

} // namespace backend
} // namespace yalx
