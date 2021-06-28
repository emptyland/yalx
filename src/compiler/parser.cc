#include "compiler/parser.h"
#include "compiler/ast.h"
#include "compiler/syntax-feedback.h"
#include "compiler/lexer.h"

namespace yalx {

namespace cpl {

#define CHECK_OK ok); if (!*ok) { return 0; } ((void)0

#define MoveNext() lookahead_ = lexer_->Next()

struct Operator {
    Node::Kind kind;
    int operands;
    int left;
    int right;
    bool assignment;
    
    constexpr Operator(Node::Kind k, int o, int l, int r, bool a = false)
        : kind(k), operands(o), left(l), right(r), assignment(a) {}
    Operator() {}
}; // struct Operator

struct Operators {
    static constexpr int kUnaryPrio = 110;
    
    // Unary:
    static constexpr auto kNot = Operator(Node::kNot, 1, kUnaryPrio, kUnaryPrio); // !
    static constexpr auto kBitwiseNegative = Operator(Node::kBitwiseNegative, 1, kUnaryPrio, kUnaryPrio); // ^
    static constexpr auto kNegative = Operator(Node::kNegative, 1, kUnaryPrio, kUnaryPrio); // -
    static constexpr auto kRecv = Operator(Node::kRecv, 1, kUnaryPrio, kUnaryPrio); // <-
//    static constexpr auto kIncrement = Operator(Node::kIncrement, 1, kUnaryPrio, kUnaryPrio, true); // expr++
//    static constexpr auto kIncrementPost = Operator(Node::kIncrementPost, 1, kUnaryPrio, kUnaryPrio, true); // ++expr
//    static constexpr auto kDecrement = Operator(Node::kDecrement, 1, kUnaryPrio, kUnaryPrio, true); // expr--
//    static constexpr auto kDecrementPost = Operator(Node::kDecrementPost, 1, kUnaryPrio, kUnaryPrio, true); // --expr

    // Binary:
    static constexpr auto kAdd = Operator(Node::kAdd, 2, 90, 90); // +
    static constexpr auto kSub = Operator(Node::kSub, 2, 90, 90); // -
    static constexpr auto kMul = Operator(Node::kMul, 2, 100, 100); // *
    static constexpr auto kDiv = Operator(Node::kDiv, 2, 100, 100); // /
    static constexpr auto kMod = Operator(Node::kMod, 2, 100, 100); // %

    static constexpr auto kSend = Operator(Node::kSend, 2, 100, 100); // chan <- data
    
    // Bitwise op
    static constexpr auto kBitwiseShl = Operator(Node::kBitwiseShl, 2, 70, 70); // <<
    static constexpr auto kBitwiseShr = Operator(Node::kBitwiseShr, 2, 70, 70); // >>
    
    // Compare
    static constexpr auto kEqual = Operator(Node::kEqual, 2, 60, 60); // ==
    static constexpr auto kNotEqual = Operator(Node::kNotEqual, 2, 60, 60); // !=
    static constexpr auto kLess = Operator(Node::kLess, 2, 60, 60); // <
    static constexpr auto kLessEqual = Operator(Node::kLessEqual, 2, 60, 60); // <=
    static constexpr auto kGreater = Operator(Node::kGreater, 2, 60, 60); // >
    static constexpr auto kGreaterEqual = Operator(Node::kGreaterEqual, 2, 60, 60); // >=

    // Or/Xor/And
    static constexpr auto kBitwiseOr = Operator(Node::kBitwiseOr, 2, 30, 30); // |
    static constexpr auto kBitwiseXor = Operator(Node::kBitwiseXor, 2, 40, 40); // ^
    static constexpr auto kBitwiseAnd = Operator(Node::kBitwiseAnd, 2, 40, 40); // &
    
    // And/Or
    static constexpr auto kAnd = Operator(Node::kAnd, 2, 20, 20); // &&
    static constexpr auto kOr = Operator(Node::kOr, 2, 10, 10); // ||
    
    //static constexpr auto kContact = Operator(Node::kContcat, 2, 10, 10); // string concat
    
    static constexpr auto NOT_OPERATOR = Operator(Node::kMaxKinds, 0, 0, 0);
}; // struct Operators

static Operator GetPrefixOp(Token::Kind kind) {
    switch (kind) {
        case Token::kNot:
            return Operators::kNot;
        case Token::kWave:
            return Operators::kBitwiseNegative;
        case Token::kMinus:
            return Operators::kNegative;
        case Token::kLArrow:
            return Operators::kRecv;
//        case Token::k2Plus:
//            return Operators::kIncrement;
//        case Token::k2Minus:
//            return Operators::kDecrement;
        default:
            return Operators::NOT_OPERATOR;
    }
}

static Operator GetPostfixOp(Token::Kind kind) {
    switch (kind) {
        case Token::kPlus:
            return Operators::kAdd;
        case Token::kMinus:
            return Operators::kSub;
        case Token::kStar:
            return Operators::kMul;
        case Token::kDiv:
            return Operators::kDiv;
        case Token::kPercent:
            return Operators::kMod;
        case Token::kLArrow:
            return Operators::kSend;
        case Token::kLShift:
            return Operators::kBitwiseShl;
        case Token::kRShift:
            return Operators::kBitwiseShr;
        case Token::kBitwiseAnd:
            return Operators::kBitwiseAnd;
        case Token::kBitwiseOr:
            return Operators::kBitwiseOr;
        case Token::kBitwiseXor:
            return Operators::kBitwiseXor;
        case Token::kEqual:
            return Operators::kEqual;
        case Token::kNotEqual:
            return Operators::kNotEqual;
        case Token::kLess:
            return Operators::kLess;
        case Token::kLessEqual:
            return Operators::kLessEqual;
        case Token::kGreater:
            return Operators::kGreater;
        case Token::kGreaterEqual:
            return Operators::kGreaterEqual;
        case Token::kAnd:
            return Operators::kAnd;
        case Token::kOr:
            return Operators::kOr;
//        case Token::k2Plus:
//            return Operators::kIncrementPost;
//        case Token::k2Minus:
//            return Operators::kDecrementPost;
        default:
            return Operators::NOT_OPERATOR;
    }
}


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

            case Token::kVolatile:
            case Token::kVal:
            case Token::kVar: {
                auto decl = ParseVariableDeclaration(CHECK_OK);
                file_unit_->mutable_statements()->push_back(decl);
            } break;

            default: {
                error_feedback_->Printf(Peek().source_position(), "Unexpected token %s", Peek().ToString().c_str());
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

// annotation_declaration ::= annotation_using+
AnnotationDeclaration *Parser::ParseAnnotationDeclaration(bool *ok) {
    auto location = Peek().source_position();
    auto decl = new (arena_) AnnotationDeclaration(arena_, location);
    while (Peek().Is(Token::kAtOutlined)) {
        auto annotation = ParseAnnotation(false /*skip_at*/, CHECK_OK);
        decl->mutable_annotations()->push_back(annotation);
        *decl->mutable_source_position() = location.Concat(annotation->source_position());
    }
    return decl;
}

// annotation_using ::= `@' symbol ( `(' annotation_value_list `)' )?
// annotation_value_list ::= annotation_value (`,' annotation_value)+
// annotation_value ::= literal | `(' annotation_value_list `)'
Annotation *Parser::ParseAnnotation(bool skip_at, bool *ok) {
    auto location = Peek().source_position();
    if (!skip_at) {
        Match(Token::kAtOutlined, CHECK_OK); // @
    }
    auto symbol = ParseSymbol(CHECK_OK);
    auto annotation = new (arena_) Annotation(arena_, symbol, location);
    if (Test(Token::kLParen)) {
        do {
            auto field_location = Peek().source_position();
            auto name = MatchText(Token::kIdentifier, CHECK_OK);
            Match(Token::kAssign, CHECK_OK);
            Annotation::Field *field = nullptr;
            if (Peek().Is(Token::kIdentifier)) {
                auto nested = ParseAnnotation(true /*skip_at*/, CHECK_OK);
                field = new (arena_) Annotation::Field(name, nested, field_location.Concat(nested->source_position()));
            } else {
                auto value = ParseStaticLiteral(CHECK_OK);
                field = new (arena_) Annotation::Field(name, value, field_location.Concat(value->source_position()));
            }
            annotation->mutable_fields()->push_back(field);
        } while (Test(Token::kComma));
        Match(Token::kRParen, CHECK_OK);
    }
    
    *annotation->mutable_source_position() = ConcatNow(location);
    return annotation;
}

// variable_declaration ::= (`val' | `var') variable_name_type `=' expression_list
// variable_name_type_list ::= variable_name_type ( `,' variable_name_type )*
// variable_name_type ::= identifer (`:' type_ref)?
VariableDeclaration *Parser::ParseVariableDeclaration(bool *ok) {
    auto location = Peek().source_position();
    const bool is_volatile = Test(Token::kVolatile);
    VariableDeclaration::Constraint constraint;
    if (is_volatile) {
        Match(Token::kVar, CHECK_OK);
        constraint = VariableDeclaration::kVar;
    } else {
        if (Test(Token::kVal)) {
            constraint = VariableDeclaration::kVal;
        } else {
            Match(Token::kVar, CHECK_OK);
            constraint = VariableDeclaration::kVar;
        }
    }
    auto decl = new (arena_) VariableDeclaration(arena_, is_volatile, constraint, location);
    do {
        auto item_location = Peek().source_position();
        auto name = MatchText(Token::kIdentifier, CHECK_OK);
        Type *type = nullptr;
        if (Test(Token::kColon)) {
            type = ParseType(CHECK_OK);
        }
        auto item = new (arena_) VariableDeclaration::Item(arena_, name, type,
                                                           !type
                                                           ? item_location
                                                           : item_location.Concat(type->source_position()));
        decl->mutable_variables()->push_back(item);
    } while (Test(Token::kComma));
    
    Match(Token::kAssign, CHECK_OK);
    
    do {
        auto expr = ParseExpression(CHECK_OK);
        *decl->mutable_source_position() = location.Concat(expr->source_position());
        decl->mutable_initilaizers()->push_back(expr);
    } while (Test(Token::kComma));
    return decl;
}

// expression ::= primary | suffix | unary | binary | statement | block
// block ::= `{' expression* `}'
// primary ::= identifer | literal | `(' expression `)'
// suffix ::= expression ( `[' expression `]' )+
//          | expression ( `.' identifer )
//          | expression `as' type_ref
//          | call
// call ::= expression `(' expression_list `)'
// unary ::= `~' expression | `!' expression | `-' expression | `<-' expression
// binary ::= boolean_expression
//          | expression `+' expression
//          | expression `-' expression
//          | expression `*' expression
//          | expression `/' expression
//          | expression `|' expression
//          | expression `&' expression
//          | expression `^' expression
//          | expression `>>' expression
//          | expression `<<' expression
// boolean_expression ::= expression `&&' expression
//                      | expression `||' expression
//                      | condition
// condition ::= expression `==' expression
//             | expression `!=' expression
//             | expression `<' expression
//             | expression `<=' expression
//             | expression `>' expression
//             | expression `<=' expression
// lval ::= identifer | expression ( `.' identifer ) | expression ( `[' expression `]' )+
Expression *Parser::ParseExpression(int limit, Operator *receiver, bool *ok) {
    auto location = Peek().source_position();
    Expression *expr = nullptr;
    Operator op = GetPrefixOp(Peek().kind());
    if (op.kind != Operators::NOT_OPERATOR.kind) {
        MoveNext();
        Expression *operand = ParseExpression(Operators::kUnaryPrio, nullptr, CHECK_OK);
        return NewUnaryExpression(op, operand, location);
    } else {
        expr = ParseSimple(CHECK_OK);
    }
    
    op = GetPostfixOp(Peek().kind());
    while (op.kind != Operators::NOT_OPERATOR.kind && op.left > limit) {

        MoveNext();
        if (op.operands == 1) { // Post unary operator
            expr = NewUnaryExpression(op, expr, location);
            op = GetPostfixOp(Peek().kind());
        } else {
            Operator next_op;
            Expression *rhs = ParseExpression(op.right, &next_op, CHECK_OK);
            
//            expr = new (arena_) BinaryExpression(position, op, expr, rhs);
//            op = next_op;
        }
    }
    if (receiver) { *receiver = op; }
    return expr;
}

Expression *Parser::ParseSimple(bool *ok) {
    // TODO:
    UNREACHABLE();
    return nullptr;
}

// type_ref ::= `bool' | `i8' | `u8' | `i16' | `u16' | `i32' | `u32' | `i64' | `u64' | `f32' | `f64'
//            | `int' | `uint' | `char' | `string'
//            | symbol
//            | generic_type
//            | array_type
//            | channel_type
// generic_type ::= symbol `<' type_list `>'
// channel_type ::= ( `in' | `out' ) `chan' `<' type_ref `>'
// array_type ::= type_ref ( `[' ([0-9]+)? `]' ) +
// function_type ::= `(' type_list? `)' (`->' type_ref | `(' type_list `)' )
// symbol ::= identifier | identifier `.' identifier
Type *Parser::ParseType(bool *ok) {
    Type *type = nullptr;
    auto location = Peek().source_position();
    switch (Peek().kind()) {
        case Token::kUnit:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_unit, location);
            break;
        case Token::kBool:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_bool, location);
            break;
        case Token::kI8:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_i8, location);
            break;
        case Token::kU8:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_u8, location);
            break;
        case Token::kI16:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_i16, location);
            break;
        case Token::kU16:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_u16, location);
            break;
        case Token::kI32:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_i32, location);
            break;
        case Token::kU32:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_u32, location);
            break;
        case Token::kI64:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_i64, location);
            break;
        case Token::kU64:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_u64, location);
            break;
        case Token::kF32:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_f32, location);
            break;
        case Token::kF64:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_f64, location);
            break;
        case Token::kInt:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_i32, location);
            break;
        case Token::kUInt:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_u32, location);
            break;
        case Token::kString:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_string, location);
            break;
        case Token::kIdentifier: {
            auto symbol = ParseSymbol(CHECK_OK);
            type = new (arena_) Type(arena_, symbol, location);
            if (Test(Token::kLess)) {
                do {
                    auto arg = ParseType(CHECK_OK);
                    type->mutable_generic_args()->push_back(arg);
                } while (Test(Token::kComma));
                Match(Token::kGreater, CHECK_OK);
            }
        } break;
            // in chan<int>
            // out chan<string>
            // chan<f32>
        case Token::kIn:
        case Token::kOut: {
            int ability = Peek().Is(Token::kIn) ? ChannelType::kInbility : ChannelType::kOutbility;
            MoveNext();
            Match(Token::kLess, CHECK_OK);
            auto element = ParseType(CHECK_OK);
            location = location.Concat(Peek().source_position());
            Match(Token::kGreater, CHECK_OK);
            type = new (arena_) ChannelType(arena_, ability, element, location);
        } break;
        case Token::kChan: {
            MoveNext();
            Match(Token::kLess, CHECK_OK);
            auto element = ParseType(CHECK_OK);
            location = location.Concat(Peek().source_position());
            Match(Token::kGreater, CHECK_OK);
            type = new (arena_) ChannelType(arena_, ChannelType::kInbility | ChannelType::kOutbility, element, location);
        } break;
        case Token::kLParen: {
            MoveNext();
            auto fun = new (arena_) FunctionPrototype(arena_, false, location);
            if (!Test(Token::kRParen)) {
                do {
                    if (Test(Token::kVargs)) {
                        fun->set_vargs(true);
                        break;
                    }
                    auto param = ParseType(CHECK_OK);
                    fun->mutable_params()->push_back(param);
                } while (Test(Token::kComma));
                Match(Token::kRParen, CHECK_OK);
            }
            Match(Token::kRArrow, CHECK_OK);
            if (Test(Token::kLParen)) {
                do {
                    auto ret = ParseType(CHECK_OK);
                    fun->mutable_return_types()->push_back(ret);
                } while (Test(Token::kComma));
                Match(Token::kRParen, CHECK_OK);
            } else {
                auto ret = ParseType(CHECK_OK);
                fun->mutable_return_types()->push_back(ret);
            }
            type = fun;
        } break;
        default:
            *ok = false;
            error_feedback_->Printf(location, "Unexpected 'type_ref', expected: %s", Peek().ToString().c_str());
            return nullptr;
    }
    
    if (Peek().Is(Token::kLBrack)) {
        int dim = 0;
        while (Test(Token::kLBrack)) {
            dim++;
            Match(Token::kRBrack, CHECK_OK);
        }
        type = new (arena_) ArrayType(arena_, type, dim, location);
        //type = array_type;
    }
    return type;
}

Expression *Parser::ParseStaticLiteral(bool *ok) {
    Expression *literal = nullptr;
    auto location = Peek().source_position();
    switch (Peek().kind()) {
        case Token::kIntVal:
            literal = new (arena_) IntLiteral(static_cast<int>(Peek().i64_val()), location);
            MoveNext();
            break;
        case Token::kUIntVal:
            literal = new (arena_) UIntLiteral(static_cast<unsigned>(Peek().u64_val()), location);
            MoveNext();
            break;
        case Token::kF32Val:
            literal = new (arena_) F32Literal(Peek().f32_val(), location);
            MoveNext();
            break;
        case Token::kF64Val:
            literal = new (arena_) F64Literal(Peek().f64_val(), location);
            MoveNext();
            break;
        case Token::kStringLine:
            literal = new (arena_) StringLiteral(Peek().text_val(), location);
            MoveNext();
            break;
        case Token::kTrue:
            literal = new (arena_) BoolLiteral(true, location);
            MoveNext();
            break;
        case Token::kFalse:
            literal = new (arena_) BoolLiteral(false, location);
            MoveNext();
            break;
        case Token::kLBrace:
            return ParseStaticArrayLiteral(ok);
        default:
            error_feedback_->Printf(location, "Unexpected 'static literal value', expected: `%s'",
                                    Peek().ToString().c_str());
            *ok = false;
            break;
    }
    return literal;
}

// array_initializer ::= array_type? array_initializer_dimension
// array_initializer_dimension ::= `{' `}'
//                               | `{' expression? ( `,' expression )+ `}'
//                               | `{' array_initializer_dimension? ( `,' array_initializer_dimension )+ `}'
ArrayInitializer *Parser::ParseStaticArrayLiteral(bool *ok) {
    auto location = Peek().source_position();
    auto init = new (arena_) ArrayInitializer(arena_, nullptr, 0, location);
    Match(Token::kLBrace, CHECK_OK);
    if (Test(Token::kRBrace)) {
        return init;
    }
    
    do {
        if (Peek().Is(Token::kIdentifier)) {
            auto anno = ParseAnnotation(true/*skip_at*/, CHECK_OK);
            init->mutable_dimensions()->push_back(anno);
        } else {
            auto literal = ParseStaticLiteral(CHECK_OK);
            init->mutable_dimensions()->push_back(literal);
        }
    } while (Test(Token::kComma));
    
    *init->mutable_source_position() = location.Concat(Peek().source_position());
    Match(Token::kRBrace, CHECK_OK);
    return init;
}

// symbol ::= identifier | identifier `.' identifier
Symbol *Parser::ParseSymbol(bool *ok) {
    auto location = Peek().source_position();
    auto prefix_or_name = MatchText(Token::kIdentifier, CHECK_OK);
    if (Test(Token::kDot)) {
        auto name = MatchText(Token::kIdentifier, CHECK_OK);
        return new (arena_) Symbol(prefix_or_name, name, ConcatNow(location));
    }
    return new (arena_) Symbol(prefix_or_name, ConcatNow(location));
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

Expression *Parser::NewUnaryExpression(const Operator &op, Expression *operand, const SourcePosition &begin) {
    auto location = begin.Concat(operand->source_position());
    assert(op.operands == 1);
    switch (op.kind) {
        case Node::kNegative:
            return new (arena_) Negative(operand, location);
        case Node::kNot:
            return new (arena_) Not(operand, location);
        case Node::kBitwiseNegative:
            return new (arena_) BitwiseNegative(operand, location);
        case Node::kRecv:
            return new (arena_) Recv(operand, location);
        default:
            UNREACHABLE();
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
