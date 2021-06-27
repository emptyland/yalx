#pragma once
#ifndef YALX_COMPILER_PARSER_H_
#define YALX_COMPILER_PARSER_H_

#include "compiler/node.h"
#include "compiler/token.h"
#include "base/status.h"
#include <string>
#include <memory>

namespace yalx {
namespace base {
class Arena;
class ArenaString;
class SequentialFile;
} // namespace base
namespace cpl {

class SyntaxFeedback;
class Lexer;

using String = base::ArenaString;

class Parser final {
public:
    Parser(base::Arena *arena, SyntaxFeedback *error_feedback);
    base::Status SwitchInputFile(const std::string &name, base::SequentialFile *file);
    
    FileUnit *Parse(bool *ok);
    const String *ParsePackageName(bool *ok);
    const String *ParseImportStatement(bool *ok);
    AnnotationDeclaration *ParseAnnotationDeclaration(bool *ok);
    Annotation *ParseAnnotation(bool skip_at, bool *ok);
    
private:
    Symbol *ParseSymbol(bool *ok);
    Expression *ParseStaticLiteral(bool *ok);
    const String *ParseAliasOrNull(bool *ok);
    
    const String *MatchText(Token::Kind kind, bool *ok);
    void Match(Token::Kind kind, bool *ok);
    bool Test(Token::Kind kind);
    const Token &Peek() const { return lookahead_; }
    
    SourcePosition ConcatNow(const SourcePosition &begin) { return begin.Concat(Peek().source_position()); }
    

    base::Arena *arena_; // Memory arena allocator
    SyntaxFeedback *error_feedback_; // Error feedback interface
    std::unique_ptr<Lexer> lexer_; // Lexer of parser
    Token lookahead_ = Token(Token::kError, {0, 0}); // Look a head token
    FileUnit *file_unit_ = nullptr;
}; // class Parser

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_PARSER_H_
