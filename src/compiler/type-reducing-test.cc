#include "compiler/type-reducing.h"
#include "compiler/compiler.h"
#include "compiler/syntax-feedback.h"
#include "compiler/source-position.h"
#include "compiler/ast.h"
#include "base/io.h"
#include "base/env.h"
#include "base/arena.h"
#include <gtest/gtest.h>

namespace yalx {

namespace cpl {

class TypeReducingTest : public ::testing::Test {
public:
    class MockErrorFeedback : public SyntaxFeedback {
    public:
        void DidFeedback(const SourcePosition &location, const char *z, size_t n) override {
            ::printf("[%s:(%d,%d)-(%d,%d)] %s\n", file_name().data(), location.begin_line(), location.begin_column(),
                     location.end_line(), location.end_column(), z);
        }
        
        void DidFeedback(const char *z) override {
            ::puts(z);
        }
    }; // class MockErrorFeedback
    
    TypeReducingTest() {}
    
    void SetUp() override {}
    void TearDown() override {}
    
    std::vector<std::string> MakeSearchPaths(const char *project_name) {
        std::vector<std::string> search_paths;
        std::string prefix("tests");
        prefix.append("/").append(project_name);
        std::string path(prefix);
        path.append("/src");
        search_paths.push_back(path);
        path.assign(prefix);
        path.append("/pkg");
        search_paths.push_back(path);
        search_paths.push_back("libs");
        return search_paths;
    }
protected:
    base::Arena arena_;
    MockErrorFeedback feedback_;
    
}; // class TypeReducingTest


TEST_F(TypeReducingTest, Sanity) {
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/06-type-reducing-sanity", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    {
        std::unordered_map<std::string_view, GlobalSymbol> symbols;
        rs = ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
    }
}


} // namespace yalx

} // namespace yalx
