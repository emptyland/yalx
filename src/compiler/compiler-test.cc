#include "compiler/compiler.h"
#include "compiler/syntax-feedback.h"
#include "compiler/source-position.h"
#include "compiler/ast.h"
#include "base/io.h"
#include "base/env.h"
#include "base/arena.h"
#include <gtest/gtest.h>

namespace yalx::cpl {

class CompilerTest : public ::testing::Test {
public:
    class MockErrorFeedback : public SyntaxFeedback {
    public:
        void DidFeedback(const SourcePosition &location, const char *z, size_t n) override {
            ::printf("[%s:(%d,%d)-(%d,%d)] %s\n", file_name().data(), location.begin_line(),
                     location.begin_column(),
                     location.end_line(), location.end_column(), z);
        }

        void DidFeedback(const char *z) override {
            ::puts(z);
        }
    }; // class MockErrorFeedback

    CompilerTest() = default;

    void SetUp() override {}

    void TearDown() override {}

    static std::vector<std::string> MakeSearchPaths(const char *project_name) {
        std::vector<std::string> search_paths;
        std::string prefix("tests");
        prefix.append("/").append(project_name);
        std::string path(prefix);
        path.append("/src");
        search_paths.push_back(path);
        path.assign(prefix);
        path.append("/pkg");
        search_paths.push_back(path);
        search_paths.emplace_back("libs");
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

TEST_F(CompilerTest, FindAndParseAllDependenciesSourceFiles) {
    Package *pkg = nullptr;
    auto rs = Compiler::FindAndParseMainSourceFiles("tests/01-import-1-pkg", &arena_, &feedback_, &pkg);
    ASSERT_TRUE(rs.ok()) << rs.ToString();

    base::ArenaMap<std::string_view, Package *> all(&arena_);
    rs = Compiler::FindAndParseAllDependenciesSourceFiles(MakeSearchPaths("01-import-1-pkg"), &arena_, &feedback_,
                                                          pkg,
                                                          &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_EQ(2, all.size());
    ASSERT_TRUE(all.find("tests/01-import-1-pkg/src/foo") != all.end());
    auto foo = all["tests/01-import-1-pkg/src/foo"];

    ASSERT_TRUE(all.find("libs/yalx/lang") != all.end());
    auto base = all["libs/yalx/lang"];

    ASSERT_EQ(2, pkg->dependences_size());
    ASSERT_EQ(foo, pkg->dependence(0));
    ASSERT_EQ(1, foo->references_size());
    ASSERT_EQ(pkg, foo->reference(0));
    ASSERT_EQ(1, foo->dependences_size());
    ASSERT_EQ(base, foo->dependence(0));
}

TEST_F(CompilerTest, ImportsDependences2Pkgs) {
    Package *pkg = nullptr;
    auto rs = Compiler::FindAndParseMainSourceFiles("tests/02-import-2-pkg", &arena_, &feedback_, &pkg);
    ASSERT_TRUE(rs.ok()) << rs.ToString();

    base::ArenaMap<std::string_view, Package *> all(&arena_);
    rs = Compiler::FindAndParseAllDependenciesSourceFiles(MakeSearchPaths("02-import-2-pkg"), &arena_, &feedback_,
                                                          pkg,
                                                          &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_EQ(4, all.size());

    ASSERT_TRUE(all.find("tests/02-import-2-pkg/src/foo") != all.end());
    auto foo = all["tests/02-import-2-pkg/src/foo"];
    ASSERT_EQ(pkg, foo->reference(0));

    ASSERT_TRUE(all.find("tests/02-import-2-pkg/src/bar") != all.end());
    auto bar = all["tests/02-import-2-pkg/src/bar"];
    ASSERT_EQ(pkg, bar->reference(0));

    ASSERT_TRUE(all.find("tests/02-import-2-pkg/pkg/github.com/emptyland/demo") != all.end());
    auto demo = all["tests/02-import-2-pkg/pkg/github.com/emptyland/demo"];
    ASSERT_EQ(bar, demo->reference(0));
    ASSERT_EQ(demo, bar->dependence(0));
}

TEST_F(CompilerTest, MakeSyntaxError) {
    Package *pkg = nullptr;
    auto rs = Compiler::FindAndParseMainSourceFiles("tests/03-simple-syntax-error", &arena_, &feedback_, &pkg);
    ASSERT_FALSE(rs.ok());
}

TEST_F(CompilerTest, ImportRingPkgs) {
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/04-import-ring-pkg", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_FALSE(rs.ok());
    //ASSERT_NE(std::string_view::npos, rs.message().find("Import dependence-ring package")) << rs.ToString();
}

TEST_F(CompilerTest, Imports2SamePkgs) {
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/05-import-2-same-pkgs", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_EQ(1, entries.size());
    auto pkg = entries[0];
    EXPECT_STREQ("lang", pkg->name()->data());
    EXPECT_STREQ("yalx/lang", pkg->path()->data());

    ASSERT_EQ(3, pkg->references_size());
    EXPECT_STREQ("foo", pkg->reference(0)->name()->data());
    EXPECT_STREQ("external1/foo", pkg->reference(0)->path()->data());
    EXPECT_STREQ("foo", pkg->reference(1)->name()->data());
    EXPECT_STREQ("external2/foo", pkg->reference(1)->path()->data());
    EXPECT_STREQ("main", pkg->reference(2)->name()->data());
    EXPECT_STREQ("main", pkg->reference(2)->path()->data());
}

} // namespace yalx
