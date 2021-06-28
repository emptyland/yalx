#include "compiler/ast.h"

namespace yalx {

namespace cpl {

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
    : Expression(Node::kDot, true, true, source_position) {
}

} // namespace cpl

} // namespace yalx
