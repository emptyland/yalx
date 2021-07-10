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
    EXPECT_STREQ("tests/00-find-main-pkg/src/main", pkg->path()->data());
    EXPECT_STREQ("tests/00-find-main-pkg/src/main:main", pkg->id()->data());
    
    auto file = pkg->source_file(0);
    EXPECT_STREQ("main", file->package_name()->data());
    EXPECT_STREQ("tests/00-find-main-pkg/src/main/foo.yalx", file->file_name()->data());
    EXPECT_STREQ("tests/00-find-main-pkg/src/main/foo.yalx", file->file_full_path()->data());
}

} // namespace cpl

} // namespace yalx
