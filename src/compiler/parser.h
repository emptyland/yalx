#pragma once
#ifndef YALX_COMPILER_PARSER_H_
#define YALX_COMPILER_PARSER_H_

#include "compiler/node.h"
#include "compiler/token.h"
#include "base/status.h"
#include "base/arena-utils.h"
#include <string>
#include <memory>
#include <tuple>

namespace yalx {
namespace base {
class Arena;
class ArenaString;
class SequentialFile;
} // namespace base
namespace cpl {

class SyntaxFeedback;
class Lexer;
class Operator;
class GenericParameter;

using String = base::ArenaString;

class Parser final {
public:
    static constexpr int kMaxRollbackDepth = 128;
    
    Parser(base::Arena *arena, SyntaxFeedback *error_feedback);
    ~Parser();
    base::Status SwitchInputFile(const std::string &name, base::SequentialFile *file);
    
    FileUnit *Parse(bool *ok);
    const String *ParsePackageName(bool *ok);
    const String *ParseImportStatement(bool *ok);
    AnnotationDeclaration *ParseAnnotationDeclaration(bool *ok);
    Annotation *ParseAnnotation(bool skip_at, bool *ok);
    VariableDeclaration *ParseVariableDeclaration(bool *ok);
    
    Statement *ParseOutsideStatement(bool *ok);
    FunctionDeclaration *ParseFunctionDeclaration(bool *ok);
    FunctionPrototype *ParseFunctionPrototype(bool *ok);
    InterfaceDefinition *ParseInterfaceDefinition(bool *ok);
    AnnotationDefinition *ParseAnnotationDefinition(bool *ok);
    ObjectDeclaration *ParseObjectDeclaration(bool *ok);
    StructDefinition *ParseStructDefinition(bool *ok);
    ClassDefinition *ParseClassDefinition(bool *ok);
    EnumDefinition *ParseEnumDefinition(bool *ok);
    IncompletableDefinition *ParseIncompletableDefinition(IncompletableDefinition *receiver,
                                                          base::ArenaVector<Type *> *concepts, bool *ok);
    Statement *ParseStatement(bool *ok);
    Block *ParseBlock(bool *ok);
    WhileLoop *ParseWhileLoop(bool *ok);
    UnlessLoop *ParseUnlessLoop(bool *ok);
    ConditionLoop *ParseConditionLoop(ConditionLoop *loop, bool *ok);
    ConditionLoop *ParseDoConditionLoop(bool *ok);
    RunCoroutine *ParseRunStatement(bool *ok);
    ForeachLoop *ParseForeachLoop(bool *ok);
    Expression *ParseExpression(bool *ok) { return ParseExpression(0, nullptr, ok); }
    Expression *ParseExpression(int limit, Operator *receiver, bool *ok);
    Expression *ParseSimple(bool *ok);
    Expression *ParseSuffixed(bool *ok);
    Expression *ParsePrimary(bool *ok);
    Expression *ParseArrayInitializer(ArrayType *qualified, int dimension_limit, bool *ok);
    Expression *ParseParenOrLambdaLiteral(bool *ok);
    IfExpression *ParseIfExpression(bool *ok);
    WhenExpression *ParseWhenExpression(bool *ok);
    TryCatchExpression *ParseTryCatchExpression(bool *ok);
    StringTemplate *ParseStringTemplate(bool *ok);
    Type *ParseType(bool *ok);
    ArrayType *ParseArrayTypeMaybeWithLimits(bool *ok);
    ArrayType *ParseArrayTypeMaybeWithLimits(const Identifier *ns, const String *id, bool *ok);
    Type *ParseAtomType(bool *ok);
private:
    bool ProbeInstantiation() {
        auto saved = lookahead_;
        bool dummy = true;
        auto rv = ProbeInstantiation(&dummy);
        lookahead_ = saved;
        return rv;
    }
    bool ProbeInstantiation(bool *ok);
    bool ProbeType(bool *ok);
    bool ProbeAtomType(bool *ok);
    void *ParseGenericParameters(base::ArenaVector<GenericParameter *> *params, bool *ok);
    Expression *ParseCommaSplittedExpressions(base::ArenaVector<Expression *> *list, Expression *receiver[2], bool *ok);
    Expression *ParseCommaSplittedExpressions(base::ArenaVector<Expression *> *receiver, bool *ok);
    Expression *ParseRemainLambdaLiteral(FunctionPrototype *prototype, const SourcePosition &location, bool *ok);
    Statement *ParseInitializerIfExistsWithCondition(Expression **condition, bool *ok);
    Identifier *ParseIdentifier(bool *ok);
    Symbol *ParseSymbol(bool *ok);
    Expression *ParseStaticLiteral(bool *ok);
    ArrayInitializer *ParseStaticArrayLiteral(bool *ok);
    const String *ParseAliasOrNull(bool *ok);
    
    Expression *NewUnaryExpression(const Operator &op, Expression *operand, const SourcePosition &location) {
        return NewExpressionWithOperands(op, operand, nullptr, location);
    }
    Expression *NewBinaryExpression(const Operator &op, Expression *lhs, Expression *rhs, const SourcePosition &location) {
        return NewExpressionWithOperands(op, lhs, rhs, location);
    }
    Expression *NewExpressionWithOperands(const Operator &op, Expression *lhs, Expression *rhs,
                                          const SourcePosition &location);
    
    int ParseDeclarationAccess();
    Symbol *EnsureToSymbol(Expression *expr, bool *ok);
    
    const String *MatchText(Token::Kind kind, bool *ok);
    void Match(Token::Kind kind, bool *ok);
    bool Test(Token::Kind kind);
    const Token &Peek() const { return lookahead_; }
    
    SourcePosition ConcatNow(const SourcePosition &begin) { return begin.Concat(Peek().source_position()); }
    
    const String *MakeFullName(const String *pkg, const String *name);
    
    void MoveNext();
    void Probe(Token::Kind kind, bool *ok);
    bool Probe(Token::Kind kind);
    void ProbeNext();

    base::Arena *arena_; // Memory arena allocator
    SyntaxFeedback *error_feedback_; // Error feedback interface
    std::unique_ptr<Lexer> lexer_; // Lexer of parser
    Token lookahead_ = Token(Token::kError, {0, 0}); // Look a head token
    FileUnit *file_unit_ = nullptr;
    Token *rollback_;
    int rollback_depth_ = 0;
    int rollback_pos_ = 0;
}; // class Parser

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_PARSER_H_
