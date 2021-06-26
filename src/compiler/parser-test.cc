#include "compiler/parser.h"
#include "compiler/syntax-feedback.h"
#include "compiler/lexer.h"
#include "compiler/ast.h"
#include "base/env.h"
#include "base/io.h"
#include "base/arena.h"
#include <gtest/gtest.h>


namespace yalx {

namespace cpl {

class ParserTest : public ::testing::Test {
public:
    class MockErrorFeedback : public SyntaxFeedback {
    public:
        void DidFeedback(const SourcePosition &location, const char *z, size_t n) override {
            ::printf("[%s:%d] %s\n", file_name().data(), location.begin_line(), z);
        }
        
        void DidFeedback(const char *z) override {
            ::puts(z);
        }
    }; // class MockErrorFeedback
    
    ParserTest(): parser_(&arena_, &feedback_) {}
    
    void SetUp() override {}
    void TearDown() override {}
    
    void SwitchInput(const char *z) {
        auto file = base::NewMemorySequentialFile(z, strlen(z));
        auto rs = parser_.SwitchInputFile(":memory:", file);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
    }
protected:
    base::Arena arena_;
    MockErrorFeedback feedback_;
    Parser parser_;
    
}; // class ParserTest



TEST_F(ParserTest, Sanity) {
    SwitchInput("package main\n"
                "\n"
                "import \"foo/bar\"");
    bool ok = true;
    auto file_unit = parser_.Parse(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, file_unit);
    
    ASSERT_NE(nullptr, file_unit->package_name());
    EXPECT_STREQ("main", file_unit->package_name()->data());
    
    ASSERT_EQ(1, file_unit->imports_size());
    EXPECT_STREQ("foo/bar", file_unit->import(0)->package_path()->data());
}

TEST_F(ParserTest, ImportStatements) {
    SwitchInput("package main\n"
                "\n"
                "import \"foo/bar\" as fb\n"
                "import \"bar/baz\" as *\n");
    bool ok = true;
    auto file_unit = parser_.Parse(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, file_unit);
    
    ASSERT_EQ(2, file_unit->imports_size());
    EXPECT_STREQ("foo/bar", file_unit->import(0)->package_path()->data());
    EXPECT_STREQ("fb", file_unit->import(0)->alias()->data());
    EXPECT_STREQ("bar/baz", file_unit->import(1)->package_path()->data());
    EXPECT_STREQ("*", file_unit->import(1)->alias()->data());
}

} // namespace cpl

} // namespace yalx
