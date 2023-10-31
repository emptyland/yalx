#include "compiler/token.h"
#include <gtest/gtest.h>


namespace yalx::cpl {

    TEST(TokenTest, Sanity) {
        Token token(Token::kEOF, {1, 2, 3, 4});
        ASSERT_EQ(Token::kEOF, token.kind());
        ASSERT_EQ(1, token.source_position().begin_line());
        ASSERT_EQ(2, token.source_position().begin_column());
        ASSERT_EQ(3, token.source_position().end_line());
        ASSERT_EQ(4, token.source_position().end_column());
    }

    TEST(TokenTest, Keywords) {
        ASSERT_EQ(Token::kTrue, Token::IsKeyword("true"));
        ASSERT_EQ(Token::kFalse, Token::IsKeyword("false"));
    }


} // namespace yalx
