#include "compiler/parser.h"
#include "compiler/ast.h"
#include "compiler/syntax-feedback.h"
#include "compiler/lexer.h"

namespace yalx {

namespace cpl {

#define CHECK_OK ok); if (!*ok) { return 0; } ((void)0

#define MoveNext() lookahead_ = lexer_->Next()

Parser::Parser(base::Arena *arena, SyntaxFeedback *error_feedback)
    : arena_(arena)
    , error_feedback_(error_feedback)
    , lexer_(new Lexer(arena, error_feedback)) {
}

base::Status Parser::SwitchInputFile(const std::string &name, base::SequentialFile *file) {
    if (auto rs = lexer_->SwitchInputFile(name, file); rs.fail()) {
        return rs;
    }
    lookahead_ = lexer_->Next();
    auto file_name = String::New(arena_, name);
    file_unit_ = new (arena_) FileUnit(arena_, file_name, file_name, {0, 0});
    return base::Status::OK();
}

FileUnit *Parser::Parse(bool *ok) {
    while (lookahead_.kind() != Token::kEOF) {
        Token token = Peek();
        switch (token.kind()) {

            case Token::kPackage:
                ParsePackageName(CHECK_OK);
                break;

            case Token::kImport:
                ParseImportStatement(CHECK_OK);
                break;
                
            case Token::kStruct: {
                // TODO:
            } break;

            case Token::kClass: {
                // TODO:
            } break;

            case Token::kObject: {
                // TODO:
            } break;

            case Token::kInterface: {
                // TODO:
            } break;
                
            case Token::kAtOutlined: {
                // TODO:
            } break;

            case Token::kNative: {
                // TODO:
            } break;

            case Token::kFun: {
                // TODO:
            } break;

            case Token::kVal:
            case Token::kVar: {
                // TODO:
            } break;

            default: {
                error_feedback_->Printf(lookahead_.source_position(), "Unexpected token %s",
                                        lookahead_.ToString().c_str());
                *ok = false;
                return nullptr;
            }
        }
    }
    return file_unit_;
}

// package_declaration ::= identifier
const String *Parser::ParsePackageName(bool *ok) {
    if (file_unit_->package_name() != nullptr) {
        error_feedback_->Printf(lookahead_.source_position(), "Duplicated package name declaration.");
        *ok = false;
        return nullptr;
    }

    Match(Token::kPackage, CHECK_OK);
    auto package_name = MatchText(Token::kIdentifier, CHECK_OK);
    file_unit_->set_package_name(package_name);
    return package_name;
}

// import_statement := `import' string_literal ( `as' alias )?
// import_statement := `import' `{' ( string_literal ( `as' alias )? )+ `}'
// alias := `*' | identifier
const String *Parser::ParseImportStatement(bool *ok) {
    auto begin = Peek().source_position();
    Match(Token::kImport, CHECK_OK);
    
    if (Peek().Is(Token::kStringLine)) {
        auto path = Peek().text_val();
        MoveNext();
        const String *alias = ParseAliasOrNull(CHECK_OK);
        auto import = new (arena_) FileUnit::ImportEntry(nullptr, path, alias, ConcatNow(begin));
        file_unit_->mutable_imports()->push_back(import);
        return path;
    }
    
    Match(Token::kLBrace, CHECK_OK);
    while (!Test(Token::kRBrace)) {
        auto begin = Peek().source_position();
        auto path = MatchText(Token::kStringLine, CHECK_OK);
        const String *alias = ParseAliasOrNull(CHECK_OK);
        auto import = new (arena_) FileUnit::ImportEntry(nullptr, path, alias, ConcatNow(begin));
        file_unit_->mutable_imports()->push_back(import);
    }
    return nullptr;
}

const String *Parser::ParseAliasOrNull(bool *ok) {
    const String *alias = nullptr;
    if (!Test(Token::kAs)) {
        return nullptr;
    }
    switch (Peek().kind()) {
        case Token::kIdentifier:
            return MatchText(Token::kIdentifier, CHECK_OK);
        case Token::kStar: {
            MoveNext();
            return String::New(arena_, "*", 1);
        } break;
        default:
            break;
    }
    return nullptr;
}

const String *Parser::MatchText(Token::Kind kind, bool *ok) {
    if (Peek().IsNot(kind)) {
        error_feedback_->Printf(Peek().source_position(), "Unexpected: `%s', expected: `%s'",
                                Token::ToString(kind).c_str(), Peek().ToString().c_str());
        *ok = false;
        return nullptr;
    }
    auto value = Peek().text_val();
    MoveNext();
    return value;
}

void Parser::Match(Token::Kind kind, bool *ok) {
    if (lookahead_.IsNot(kind)) {
        error_feedback_->Printf(lookahead_.source_position(), "Unexpected: %s, expected: %s",
                                Token::ToString(kind).c_str(), lookahead_.ToString().c_str());
        *ok = false;
        return;
    }
    lookahead_ = lexer_->Next();
}

bool Parser::Test(Token::Kind kind) {
    if (lookahead_.Is(kind)) {
        lookahead_ = lexer_->Next();
        return true;
    }
    return false;
}

} // namespace cpl

} // namespace yalx
