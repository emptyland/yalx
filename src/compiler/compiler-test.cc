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

class CompilerTest : public ::testing::Test {
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
    
    CompilerTest() {}

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
    
}; // class ParserTest

TEST_F(CompilerTest, FindAndParseMainPackage) {
    Package *pkg = nullptr;
    auto rs = Compiler::FindAndParseMainSourceFiles("tests/00-find-main-pkg", &arena_, &feedback_, &pkg);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    ASSERT_EQ(2, pkg->source_files_size());
    EXPECT_STREQ("main", pkg->name()->data());
    EXPECT_STREQ("main", pkg->path()->data());
    EXPECT_STREQ("tests/00-find-main-pkg/src/main", pkg->full_path()->data());
    EXPECT_STREQ("tests/00-find-main-pkg/src/main:main", pkg->id()->data());
    
    auto file = pkg->source_file(0);
    EXPECT_STREQ("main", file->package_name()->data());
    EXPECT_STREQ("tests/00-find-main-pkg/src/main/foo.yalx", file->file_name()->data());
    EXPECT_STREQ("tests/00-find-main-pkg/src/main/foo.yalx", file->file_full_path()->data());
}

TEST_F(CompilerTest, FindAndParseAllDependencesSourceFiles) {
    Package *pkg = nullptr;
    auto rs = Compiler::FindAndParseMainSourceFiles("tests/01-import-1-pkg", &arena_, &feedback_, &pkg);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    rs = Compiler::FindAndParseAllDependencesSourceFiles(MakeSearchPaths("01-import-1-pkg"), &arena_, &feedback_, pkg,
                                                         &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_EQ(1, all.size());
    ASSERT_TRUE(all.find("tests/01-import-1-pkg/src/foo") != all.end());
}

} // namespace cpl

} // namespace yalx
