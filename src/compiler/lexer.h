#pragma once
#ifndef YALX_COMPILER_LEXER_H_
#define YALX_COMPILER_LEXER_H_

#include "compiler/token.h"
#include "base/checking.h"
#include "base/status.h"
#include "base/base.h"
#include <stack>

namespace yalx {
namespace base {
class Arena;
class SequentialFile;
} // namespace base
namespace cpl {

class SyntaxFeedback;

class Lexer final {
public:
    static constexpr size_t kBufferSize = 4096;
    
    Lexer(base::Arena *arena, SyntaxFeedback *error_feedback)
        : arena_(DCHECK_NOTNULL(arena))
        , error_feedback_(error_feedback) {}

    ~Lexer() {
        if (input_file_ownership_) { delete input_file_; }
    }
    
    base::Status SwitchInputFile(const std::string &name, base::SequentialFile *file);

    Token Next();
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Lexer);
private:
    enum TemplateKind {
        kNotTemplate,
        kSimpleTemplate,
        kExpressionTemplate,
    };

    Token MatchString(int quote, bool escape, bool block);
    Token MatchSimpleTemplateString();
    Token MatchIdentifier();
    Token MatchOne(Token::Kind kind) {
        MoveNext();
        return Token(kind, SourcePosition{line_, column_});
    }
    
    Token MatchNumber(int sign, int line, int row);
    
    bool MatchEscapeCharacter(std::string *buf);
    bool MatchUtf8Character(std::string *buf);
    
    Token OneCharacterError(int line, int row, const char *fmt, ...);
    
    static bool IsTerm(int ch) { return ::isalpha(ch) || ::isdigit(ch) || IsTermChar(ch); }
    static bool IsTermChar(int ch);

    static bool IsHexChar(int ch) {
        return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
    }
    
    inline static int HexCharToInt(int ch) {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }
        if (ch >= 'a' && ch <= 'f') {
            return 10 + (ch - 'a');
        }
        if (ch >= 'A' && ch <= 'F') {
            return 10 + (ch - 'A');
        }
        UNREACHABLE();
        return -1;
    }

    inline static int IsUtf8Prefix(int ch) {
        if ((ch & 0x80) == 0) {
            return 1;
        } else if ((ch & 0xe0) == 0xc0) { // 110x xxxx : 2 bytes
            return 2;
        } else if ((ch & 0xf0) == 0xe0) { // 1110 xxxx : 3 bytes
            return 3;
        } else if ((ch & 0xf8) == 0xf0) { // 1111 0xxx : 4 bytes
            return 4;
        } else if ((ch & 0xfc) == 0xf8) { // 1111 10xx : 5 bytes
            return 5;
        } else if ((ch & 0xfe) == 0xfc) { // 1111 110x : 6 bytes
            return 6;
        }
        return 0;
    }
    
    int Peek();
    
    int MoveNext() {
        buffer_position_++;
        column_++;
        return Peek();
    }
    
    base::Arena *arena_;
    SyntaxFeedback *error_feedback_ = nullptr;
    base::SequentialFile *input_file_ = nullptr;
    bool input_file_ownership_ = false;
    std::stack<TemplateKind> in_string_template_;
    int line_ = 0;
    int column_ = 0;
    size_t available_ = 0; // How many bytes can read
    size_t buffer_position_ = 0; // Position for 'buffered_'
    std::string_view buffered_; // Buffered data
    std::string scratch_; // Scratch for 'buffered_'
}; // class Lexer

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_LEXER_H_


