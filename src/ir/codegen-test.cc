#include "ir/codegen.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "compiler/compiler.h"
#include "compiler/syntax-feedback.h"
#include "compiler/source-position.h"
#include "base/io.h"
#include <gtest/gtest.h>

namespace yalx {

namespace ir {

class IntermediateRepresentationGeneratorTest : public ::testing::Test {
public:
    class MockErrorFeedback : public cpl::SyntaxFeedback {
    public:
        void DidFeedback(const cpl::SourcePosition &location, const char *z, size_t n) override {
            ::printf("[%s:(%d,%d)-(%d,%d)] %s\n", file_name().data(), location.begin_line(), location.begin_column(),
                     location.end_line(), location.end_column(), z);
        }
        
        void DidFeedback(const char *z) override {
            ::puts(z);
        }
    }; // class MockErrorFeedback
    
    IntermediateRepresentationGeneratorTest() {}
    
    void SetUp() override {}
    void TearDown() override {}
    
    void IRGen(const char *project_dir, base::ArenaMap<std::string_view, Module *> *modules, bool *ok) {
        base::ArenaMap<std::string_view, cpl::Package *> all(&ast_arean_);
        base::ArenaVector<cpl::Package *> entries(&ast_arean_);
        cpl::Package *main_pkg = nullptr;
        auto rs = cpl::Compiler::FindAndParseProjectSourceFiles(project_dir, "libs", &ast_arean_, &feedback_,
                                                                &main_pkg, &entries, &all);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
        std::unordered_map<std::string_view, cpl::GlobalSymbol> symbols;
        rs = cpl::Compiler::ReducePackageDependencesType(main_pkg, &ast_arean_, &feedback_, &symbols);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
        
        rs = cpl::Compiler::GenerateIntermediateRepresentationCode(symbols, &arean_, main_pkg, &feedback_, modules);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
        *ok = true;
    }

protected:
    base::Arena ast_arean_;
    base::Arena arean_;
    MockErrorFeedback feedback_;
}; // class IntermediateRepresentationGenerator


TEST_F(IntermediateRepresentationGeneratorTest, Sanity) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arean_);
    IRGen("tests/18-ir-gen-sanity", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("foo:foo") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
//    modules["yalx/lang:lang"]->PrintTo(&printer);
//    modules["foo:foo"]->PrintTo(&printer);
    modules["main:main"]->PrintTo(&printer);
    
    printf("%s\n", buf.data());
}

} // namespace ir

} // namespace yalx
