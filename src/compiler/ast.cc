#include "compiler/ast.h"

namespace yalx {

namespace cpl {

FileUnit::ImportEntry::ImportEntry(const String *original_package_name, const String *package_path, const String *alias,
                                   const SourcePosition &source_position)
    : AstNode(Node::kMaxKinds, source_position)
    , original_package_name_(original_package_name)
    , package_path_(DCHECK_NOTNULL(package_path))
    , alias_(alias) {
}

FileUnit::FileUnit(base::Arena *arena, String *file_name, String *file_full_path, const SourcePosition &source_position)
    : AstNode(Node::kFileUnit, source_position)
    , file_name_(DCHECK_NOTNULL(file_name))
    , file_full_path_(DCHECK_NOTNULL(file_full_path))
    , imports_(arena)
    , statements_(arena) {        
}

Statement::Statement(Kind kind, const SourcePosition &source_position)
    : AstNode(kind, source_position) {
}

Block::Block(base::Arena *arena, const SourcePosition &source_position)
    : Statement(Node::kBlock, source_position)
    , statements_(arena) {        
}

List::List(base::Arena *arena, const SourcePosition &source_position)
    : Statement(Node::kList, source_position)
    , expressions_(arena) {        
}

Assignment::Assignment(base::Arena *arena, const SourcePosition &source_position)
    : Statement(Node::kAssignment, source_position)
    , lvals_(arena)
    , rvals_(arena) {
}

Return::Return(base::Arena *arena, const SourcePosition &source_position)
    : Statement(Node::kReturn, source_position)
    , returnning_vals_(arena) {
}

Declaration::Declaration(base::Arena *arena, Kind kind, const SourcePosition &source_position)
    : Statement(kind, source_position) {        
}

VariableDeclaration::VariableDeclaration(base::Arena *arena, bool is_volatile, Constraint constraint,
                    const SourcePosition &source_position)
    : Declaration(arena, Node::kVariableDeclaration, source_position)
    , constraint_(constraint)
    , is_volatile_(is_volatile)
    , variables_(arena)
    , initilaizers_(arena) {        
}

AnnotationDeclaration::AnnotationDeclaration(base::Arena *arena, const SourcePosition &source_position)
    : AstNode(Node::kAnnotationDeclaration, source_position)
    , annotations_(arena) {        
}

Annotation::Annotation(base::Arena *arena, Symbol *name, const SourcePosition &source_position)
    : AstNode(Node::kAnnotation, source_position)
    , name_(DCHECK_NOTNULL(name))
    , fields_(arena) {
}

Expression::Expression(Kind kind, bool is_lval, bool is_rval, const SourcePosition &source_position)
    : Statement(kind, source_position)
    , is_lval_(is_lval)
    , is_rval_(is_rval) {        
}

Identifier::Identifier(const String *name, const SourcePosition &source_position)
    : Expression(Node::kIdentifier, true, true, source_position)
    , name_(name) {        
}

Literal::Literal(Kind kind, Type *type, const SourcePosition &source_position)
    : Expression(kind, false/*is_lval*/, true/*is_rval*/, source_position)
    , type_(type) {
    assert(is_only_rval());
}

LambdaLiteral::LambdaLiteral(FunctionPrototype *prototype, Statement *body, const SourcePosition &source_position)
    : Literal(Node::kLambdaLiteral, prototype, source_position)
    , prototype_(prototype)
    , body_(body) {
}

ArrayInitializer::ArrayInitializer(base::Arena *arena, Type *type, int dimension_count,
                                   const SourcePosition &source_position)
    : Literal(Node::kArrayInitializer, type, source_position)
    , dimension_count_(dimension_count)
    , dimensions_(arena) {        
}

BinaryExpression::BinaryExpression(Kind kind, Expression *lhs, Expression *rhs, const SourcePosition &source_position)
    : ExpressionWithOperands<2>(kind, source_position) {
    set_operand(0, lhs);
    set_operand(1, rhs);
}

UnaryExpression::UnaryExpression(Kind kind, Expression *operand, const SourcePosition &source_position)
    : ExpressionWithOperands<1>(kind, source_position) {
    set_operand(0, operand);
}

#define DEFINE_CTOR(name, base) \
    name :: name (base##_VARGS, const SourcePosition &source_position) \
        : base##Expression(Node::k##name, base##_PARAMS, source_position) {}
DECLARE_EXPRESSION_WITH_OPERANDS(DEFINE_CTOR)
#undef DEFINE_CTOR

Dot::Dot(Expression *primary, const String *field, const SourcePosition &source_position)
    : Expression(Node::kDot, true /*is_lval*/, true /*is_rval*/, source_position) {
}

Casting::Casting(Expression *source, Type *destination, const SourcePosition &source_position)
    : Expression(Node::kCasting, false /*is_lval*/, true /*ls_rval*/, source_position) {
}

Testing::Testing(Expression *source, Type *destination, const SourcePosition &source_position)
    : Expression(Node::kTesting, false /*is_lval*/, true /*ls_rval*/, source_position) {
}

Calling::Calling(base::Arena *arena, Expression *callee, const SourcePosition &source_position)
    : Expression(Node::kTesting, false /*is_lval*/, true /*ls_rval*/, source_position)
    , callee_(DCHECK_NOTNULL(callee))
    , args_(arena) {
}

IfExpression::IfExpression(Statement *initializer, Expression *condition, Statement *then_clause, Statement *else_clause,
                           const SourcePosition &source_position)
    : Expression(Node::kIfExpression, false /*is_lval*/, true /*ls_rval*/, source_position)
    , initializer_(initializer)
    , condition_(DCHECK_NOTNULL(condition))
    , then_clause_(then_clause)
    , else_clause_(else_clause) {
}

WhenExpression::WhenExpression(base::Arena *arena, Statement *initializer, Expression *destination,
                               const SourcePosition &source_position)
    : Expression(Node::kWhenExpression, false /*is_lval*/, true /*ls_rval*/, source_position)
    , initializer_(initializer)
    , destination_(destination)
    , case_clauses_(arena) {
}

WhenExpression::Case::Case(Expression *pattern, Statement *then_clause, const SourcePosition &source_position)
    : Node(Node::kMaxKinds, source_position)
    , pattern_(pattern)
    , then_clause_(DCHECK_NOTNULL(then_clause)) {
}

bool Type::Is(Node *node) {
    switch (node->kind()) {
    #define DEFINE_CASE(_, clazz) case Node::k##clazz:
        DECLARE_TYPE_CATEGORIES(DEFINE_CASE)
    #undef DEFINE_CASE
            return true;
        default:
            return false;
    }
}

} // namespace cpl

} // namespace yalx
