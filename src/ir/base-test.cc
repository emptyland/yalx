#include "ir/base-test.h"
#include "ir/codegen.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/operators-factory.h"
#include "compiler/compiler.h"
#include "compiler/syntax-feedback.h"
#include "compiler/source-position.h"
#include "base/io.h"

namespace yalx::ir {

class BaseTest::MockErrorFeedback : public cpl::SyntaxFeedback {
public:
    void DidFeedback(const cpl::SourcePosition &location, const char *z, size_t n) override {
        ::printf("[%s:(%d,%d)-(%d,%d)] %s\n", file_name().data(), location.begin_line(), location.begin_column(),
                 location.end_line(), location.end_column(), z);
    }
    
    void DidFeedback(const char *z) override {
        ::puts(z);
    }
}; // class MockErrorFeedback

BaseTest::BaseTest()
: ops_(new OperatorsFactory(&arena_))
, feedback_(new MockErrorFeedback()) {
    
}

BaseTest::~BaseTest() {
    delete feedback_;
    delete ops_;
}

void BaseTest::IRGen(const char *project_dir, base::ArenaMap<std::string_view, Module *> *modules, bool *ok) {
    base::ArenaMap<std::string_view, cpl::Package *> all(&ast_arena_);
    base::ArenaVector<cpl::Package *> entries(&ast_arena_);
    cpl::Package *main_pkg = nullptr;
    auto rs = cpl::Compiler::FindAndParseProjectSourceFiles(project_dir, "libs", &ast_arena_, feedback_,
                                                            &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, cpl::GlobalSymbol> symbols;
    rs = cpl::Compiler::ReducePackageDependenciesType(main_pkg, &ast_arena_, feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    rs = cpl::Compiler::GenerateIntermediateRepresentationCode(symbols, &arena_, ops_, main_pkg, feedback_,
                                                               modules);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    *ok = true;
}

} // namespace yalx::ir
