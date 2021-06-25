#include "compiler/lexer.h"
#include "compiler/syntax-feedback.h"
#include "base/io.h"
#include "base/env.h"
#include "base/arena.h"
#include <gtest/gtest.h>

namespace yalx {
    
namespace cpl {


class LexerTest : public ::testing::Test {
public:
    class MockErrorFeedback : public SyntaxFeedback {
    public:
        void DidFeedback(const SourcePosition &location, const char *z, size_t n) override {
            
        }
        
        void DidFeedback(const char *z) override {
            
        }
    }; // class MockErrorFeedback
    
    LexerTest() : lexer_(&arena_, &feedback_) {}
    
    void SetUp() override {
        
    }
    
    void TearDown() override {
        
    }
    
    void SwitchInput(const char *z) {
        auto file = base::NewMemorySequentialFile(z, strlen(z));
        auto rs = lexer_.SwitchInputFile(":memory:", file);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
    }

protected:
    base::Arena arena_;
    MockErrorFeedback feedback_;
    Lexer lexer_;
};

TEST_F(LexerTest, Sanity) {
    SwitchInput("1,2,3");
    auto token = lexer_.Next();
    ASSERT_EQ(Token::kIntVal, token.kind());
    ASSERT_EQ(1, token.i64_val());
    ASSERT_EQ(1, token.source_position().begin_line());
    ASSERT_EQ(1, token.source_position().begin_column());
    
    token = lexer_.Next();
    ASSERT_EQ(Token::kComma, token.kind());
    ASSERT_EQ(1, token.source_position().begin_line());
    ASSERT_EQ(2, token.source_position().begin_column());
    
    token = lexer_.Next();
    ASSERT_EQ(Token::kIntVal, token.kind());
    ASSERT_EQ(2, token.i64_val());
    ASSERT_EQ(1, token.source_position().begin_line());
    ASSERT_EQ(3, token.source_position().begin_column());
    
    token = lexer_.Next();
    ASSERT_EQ(Token::kComma, token.kind());
    ASSERT_EQ(1, token.source_position().begin_line());
    ASSERT_EQ(4, token.source_position().begin_column());
    
    token = lexer_.Next();
    ASSERT_EQ(Token::kIntVal, token.kind());
    ASSERT_EQ(3, token.i64_val());
    ASSERT_EQ(1, token.source_position().begin_line());
    ASSERT_EQ(5, token.source_position().begin_column());
    
    token = lexer_.Next();
    ASSERT_EQ(Token::kEOF, token.kind());
}

TEST_F(LexerTest, Annotation) {
    SwitchInput("@Lang");
    auto token = lexer_.Next();
    ASSERT_EQ(Token::kAtOutlined, token.kind());
    ASSERT_EQ(1, token.source_position().begin_line());
    ASSERT_EQ(1, token.source_position().begin_column());
    
    token = lexer_.Next();
    ASSERT_EQ(Token::kIdentifier, token.kind());
    ASSERT_EQ(1, token.source_position().begin_line());
    ASSERT_EQ(2, token.source_position().begin_column());
    ASSERT_EQ(1, token.source_position().end_line());
    ASSERT_EQ(5, token.source_position().end_column());
}


} // namespace cpl

} // namespace yalx
