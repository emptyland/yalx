#include "compiler/parser.h"
#include "compiler/ast.h"
#include "compiler/syntax-feedback.h"
#include "compiler/lexer.h"

namespace yalx {

namespace cpl {

#define CHECK_OK ok); if (!*ok) { return 0; } ((void)0

//#define MoveNext() lookahead_ = lexer_->Next()

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
    , lexer_(new Lexer(arena, error_feedback))
    , rollback_(arena->NewArray<Token>(kMaxRollbackDepth)){
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
                auto stmt = ParseInterfaceDefinition(CHECK_OK);
                file_unit_->mutable_statements()->push_back(stmt);
            } break;
                
            case Token::kAtOutlined: {
                auto anno = ParseAnnotationDeclaration(CHECK_OK);
                auto access = static_cast<Access>(ParseDeclarationAccess());
                auto stmt = ParseOutsideStatement(CHECK_OK);
                if (Declaration::Is(stmt)) {
                    static_cast<Declaration *>(stmt)->set_annotations(anno);
                    static_cast<Declaration *>(stmt)->set_access(access);
                    // Declaration should has annotations.
                } else if (Definition::Is(stmt)) {
                    static_cast<Definition *>(stmt)->set_annotations(anno);
                    static_cast<Definition *>(stmt)->set_access(access);
                } else {
                    error_feedback_->Printf(anno->source_position(), "Incorrect annotation declaration, wrong position");
                    *ok = false;
                    return nullptr;
                }
            } break;
                
            case Token::kExport:
            case Token::kPublic:
            case Token::kProtected:
            case Token::kPrivate: {
                auto access = static_cast<Access>(ParseDeclarationAccess());
                auto stmt = ParseOutsideStatement(CHECK_OK);
                if (Declaration::Is(stmt)) {
                    static_cast<Declaration *>(stmt)->set_access(access);
                    // Declaration should has annotations.
                } else if (Definition::Is(stmt)) {
                    static_cast<Definition *>(stmt)->set_access(access);
                } else {
                    error_feedback_->Printf(stmt->source_position(), "Incorrect access declaration, wrong position");
                    *ok = false;
                    return nullptr;
                }
                file_unit_->mutable_statements()->push_back(stmt);
            } break;

            case Token::kNative:
            case Token::kFun: {
                auto fun = ParseFunctionDeclaration(ok);
                file_unit_->mutable_statements()->push_back(fun);
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

Statement *Parser::ParseOutsideStatement(bool *ok) {
    auto location = Peek().source_position();
    switch (Peek().kind()) {
        case Token::kInterface:
            return ParseInterfaceDefinition(ok);
            
        case Token::kAnnotation:
            return ParseAnnotationDefinition(ok);
            
        case Token::kNative:
        case Token::kFun:
            return ParseFunctionDeclaration(ok);
            
        case Token::kVolatile:
        case Token::kVal:
        case Token::kVar:
            return ParseVariableDeclaration(ok);
            
        default:
            error_feedback_->Printf(Peek().source_position(), "Unexpected token %s", Peek().ToString().c_str());
            *ok = false;
            return nullptr;
    }
}

// function_definition ::= `fun' generic_declaration? identifier function_prototype block
//                       | `fun' generic_declaration? identifier `(' argument_list? `)' `->' expression
// function_declaration ::= `native' `fun' identifier function_prototype
// function_prototype ::= `(' argument_list? `)' ( `:'  return_types )?
// return_types ::= type_ref | `(' type_list `)'
// argument_list ::= argument+ ( `,' vargs )?
//                  | vargs
// argument ::= identifier `:' type_ref
// vargs ::= `...'
//
// generic_declaration ::= `<' generic_symbol_list `>'
// generic_symbol_list ::= identifer ( `,' identifer )*
FunctionDeclaration *Parser::ParseFunctionDeclaration(bool *ok) {
    auto location = Peek().source_position();
    
    FunctionDeclaration::Decoration decoration = FunctionDeclaration::kDefault;
    switch (Peek().kind()) {
        case Token::kNative:
            decoration = FunctionDeclaration::kNative;
            MoveNext();
            break;
        case Token::kOverride:
            decoration = FunctionDeclaration::kOverride;
            MoveNext();
            break;
        case Token::kAbstract:
            decoration = FunctionDeclaration::kAbstract;
            MoveNext();
            break;
        default:
            break;
    }
    
    // `fun' `<' .. `>'
    Match(Token::kFun, CHECK_OK);
    
    base::ArenaVector<GenericParameter *> generic_params(arena_);
    if (Peek().Is(Token::kLess)) {
        ParseGenericParameters(&generic_params, CHECK_OK);
    }
    
    auto name = MatchText(Token::kIdentifier, CHECK_OK);
    
    auto prototype = ParseFunctionPrototype(CHECK_OK);
    auto is_reduce = Test(Token::kRArrow);
    auto decl = new (arena_) FunctionDeclaration(arena_, decoration, name, prototype, is_reduce, location);
    *decl->mutable_generic_params() = std::move(generic_params);
    
    if (decoration == FunctionDeclaration::kNative || decoration == FunctionDeclaration::kAbstract) {
        return decl;
    }
    
    if (!is_reduce) {
        if (Peek().IsNot(Token::kLBrace)) {
            return decl;
        }
        
        auto body = ParseStatement(CHECK_OK);
        if (!body->IsBlock()) {
            error_feedback_->Printf(body->source_position(), "Incorrect function body");
            *ok = false;
            return nullptr;
        }
        *decl->mutable_source_position() = location.Concat(body->source_position());
        decl->set_body(body);
        return decl;
    }
    
    auto body = ParseStatement(CHECK_OK);
    *decl->mutable_source_position() = location.Concat(body->source_position());
    decl->set_body(body);
    return decl;
}

FunctionPrototype *Parser::ParseFunctionPrototype(bool *ok) {
    auto location = Peek().source_position();
    
    Match(Token::kLParen, CHECK_OK);
    
    auto prototype = new (arena_) FunctionPrototype(arena_, false, location);
    if (!Test(Token::kRParen)) {
        do {
            auto param_location = Peek().source_position();

            if (Test(Token::kVargs)) {
                prototype->set_vargs(true);
                Match(Token::kRParen, CHECK_OK);
                break;
            }
            
            AnnotationDeclaration *anno = nullptr;
            if (Peek().Is(Token::kAtOutlined)) {
                anno = ParseAnnotationDeclaration(CHECK_OK);
            }
            
            auto name = MatchText(Token::kIdentifier, CHECK_OK);
            Match(Token::kColon, CHECK_OK);
            auto type = ParseType(CHECK_OK);
            
            auto param = new (arena_) VariableDeclaration::Item(arena_, name, type,
                                                                param_location.Concat(type->source_position()));
            param->set_annotations(anno);
            
            prototype->mutable_params()->push_back(param);
        } while (Test(Token::kComma));
        
        Match(Token::kRParen, CHECK_OK);
    }

    if (Test(Token::kColon)) {
        if (Test(Token::kLParen)) {
            do {
                auto type = ParseType(CHECK_OK);
                prototype->mutable_return_types()->push_back(type);
            } while (Test(Token::kComma));
            Match(Token::kRParen, CHECK_OK);
        } else {
            auto type = ParseType(CHECK_OK);
            prototype->mutable_return_types()->push_back(type);
        }
    }
    return prototype;
}

InterfaceDefinition *Parser::ParseInterfaceDefinition(bool *ok) {
    auto location = Peek().source_position();
    
    Match(Token::kInterface, CHECK_OK);
    auto name = MatchText(Token::kIdentifier, CHECK_OK);
    auto def = new (arena_) InterfaceDefinition(arena_, location);
    
    if (Peek().Is(Token::kLess)) {
        ParseGenericParameters(def->mutable_generic_params(), CHECK_OK);
    }
    
    Match(Token::kLBrace, CHECK_OK);
    do {
        AnnotationDeclaration *anno = nullptr;
        if (Peek().Is(Token::kAtOutlined)) {
            anno = ParseAnnotationDeclaration(CHECK_OK);
        }
        
        auto decl = ParseFunctionDeclaration(CHECK_OK);
        if (decl->access() != kDefault && decl->access() != kPublic) {
            *ok = false;
            error_feedback_->Printf(decl->source_position(), "Incorrect member access, should be `public' or none");
            return nullptr;
        }
        
        if (decl->body() != nullptr) {
            *ok = false;
            error_feedback_->Printf(decl->source_position(), "Method of interface don't need body");
            return nullptr;
        }
        
        decl->set_annotations(anno);
        def->mutable_methods()->push_back(decl);
        
        location = location.Concat(Peek().source_position());
    } while (!Test(Token::kRBrace));
    
    *def->mutable_source_position() = location;
    return def;
}

// annotation Foo {
//     name: i8[] default {1,2,3}
//     id: string default "ok"
// }
AnnotationDefinition *Parser::ParseAnnotationDefinition(bool *ok) {
    auto location = Peek().source_position();
    
    Match(Token::kAnnotation, CHECK_OK);
    auto name = MatchText(Token::kIdentifier, CHECK_OK);
    
    Match(Token::kLBrace, CHECK_OK);
    auto def = new (arena_) AnnotationDefinition(arena_, location);
    while (!Test(Token::kRBrace)) {
        auto member_location = Peek().source_position();
        
        auto id = MatchText(Token::kIdentifier, CHECK_OK);
        Match(Token::kColon, CHECK_OK);
        auto type = ParseType(CHECK_OK);
        member_location = member_location.Concat(type->source_position());
        
        Expression *default_value = nullptr;
        if (Test(Token::kDefault)) {
            default_value = ParseStaticLiteral(CHECK_OK);
            member_location = member_location.Concat(default_value->source_position());
        }
        
        auto field = new (arena_) VariableDeclaration::Item(arena_, id, type, member_location);
        def->mutable_members()->push_back({field, default_value});
        
        location = location.Concat(Peek().source_position());
    }
    
    *def->mutable_source_position() = location;
    return def;
}

StructDefinition *Parser::ParseStructDefinition(bool *ok) {
    auto location = Peek().source_position();
    Match(Token::kStruct, CHECK_OK);
    auto name = MatchText(Token::kIdentifier, CHECK_OK);
    // TODO:
}

// package_declaration ::= identifier
const String *Parser::ParsePackageName(bool *ok) {
    if (file_unit_->package_name() != nullptr) {
        error_feedback_->Printf(Peek().source_position(), "Duplicated package name declaration.");
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

Statement *Parser::ParseStatement(bool *ok) {
    auto location = Peek().source_position();
    switch (Peek().kind()) {
        case Token::kLBrace: {
            MoveNext();
            auto block = new (arena_) Block(arena_, location);
            while (!Test(Token::kRBrace)) {
                auto stmt = ParseStatement(CHECK_OK);
                block->mutable_statements()->push_back(stmt);
                
                *block->mutable_source_position() = location.Concat(Peek().source_position());
            }
        } break;
            
        case Token::kVolatile:
        case Token::kVal:
        case Token::kVar:
            return ParseVariableDeclaration(ok);
            
        case Token::kReturn: {
            auto stmt = new (arena_) Return(arena_, location);
            // return Unit
            if (Peek().Is(Token::kRBrace) || Peek().Is(Token::kSemi)) {
                MoveNext();
                return stmt;
            }
            ParseCommaSplittedExpressions(stmt->mutable_returnning_vals(), CHECK_OK);
            *stmt->mutable_source_position() = location.Concat(stmt->returnning_vals().back()->source_position());
            return stmt;
        } break;
        // TODO:
            
        default: {
            Expression *tmp[2] = {nullptr,nullptr};
            base::ArenaVector<Expression *> lvals(arena_);
            ParseCommaSplittedExpressions(&lvals, tmp, CHECK_OK);
            
            if (Test(Token::kAssign)) { //
                Assignment *stmt = new (arena_) Assignment(arena_, location);
                if (!lvals.empty()) {
                    *stmt->mutable_lvals() = std::move(lvals);
                } else {
                    stmt->mutable_lvals()->push_back(tmp[0]);
                }
                ParseCommaSplittedExpressions(stmt->mutable_rvals(), CHECK_OK);
                return stmt;
            }
            if (!lvals.empty()) {
                auto list = new (arena_) List(arena_, location.Concat(lvals.back()->source_position()));
                *list->mutable_expressions() = std::move(lvals);
                return list;
            }
            return tmp[0];
        } break;
    }
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
            
            expr = NewBinaryExpression(op, expr, rhs, location);
            op = next_op;
        }
    }
    if (receiver) { *receiver = op; }
    return expr;
}

Expression *Parser::ParseSimple(bool *ok) {
    auto location = Peek().source_position();
    switch (Peek().kind()) {
        case Token::kTrue: {
            auto literal = new (arena_) BoolLiteral(true, location);
            MoveNext();
            return literal;
        } break;

        case Token::kFalse: {
            auto literal = new (arena_) BoolLiteral(false, location);
            MoveNext();
            return literal;
        } break;

        case Token::kIf:
            return ParseIfExpression(ok);
            
        case Token::kWhen:
            return ParseWhenExpression(ok);

        default:
            return ParseSuffixed(ok);
    }
}

Expression *Parser::ParseSuffixed(bool *ok) {
    auto location = Peek().source_position();
    Expression *expr = ParsePrimary(CHECK_OK);
    for (;;) {
        switch (Peek().kind()) {
            case Token::kLess: {
                if (!expr->IsIdentifier() && !expr->IsDot()) {
                    return expr;
                }
                if (!ProbeInstantiation()) {
                    return expr;
                }
                
                auto inst = new (arena_) Instantiation(arena_, expr, location);
                MoveNext();
                do {
                    auto type = ParseType(CHECK_OK);
                    inst->mutable_generic_args()->push_back(type);
                } while (Test(Token::kComma));
                Match(Token::kGreater, CHECK_OK);
                expr = inst;
            } break;
                
            case Token::kDot: { // .
                MoveNext();
                auto dot_location = location.Concat(Peek().source_position());
                auto field = MatchText(Token::kIdentifier, CHECK_OK);
                expr = new (arena_) Dot(expr, field, dot_location);
            } break;
                
            case Token::kLBrack: { // [
                MoveNext();
                Expression *index = ParseExpression(CHECK_OK);
                auto idx_location = location.Concat(Peek().source_position());
                Match(Token::kRBrack, CHECK_OK);
                expr = new (arena_) IndexedGet(expr, index, idx_location);
            } break;
                
            case Token::kLParen: { // (
                MoveNext();
                auto call_location = Peek().source_position();
                if (Test(Token::kRParen)) { // call()
                    expr = new (arena_) Calling(arena_, expr, location.Concat(call_location));
                } else {
                    auto call = new (arena_) Calling(arena_, expr, location.Concat(call_location));
                    do {
                        auto arg = ParseExpression(CHECK_OK);
                        call->mutable_args()->push_back(arg);
                    } while (Test(Token::kComma));
                    
                    call_location = Peek().source_position();
                    Match(Token::kRParen, CHECK_OK);
                    *call->mutable_source_position() = location.Concat(call_location);
                    expr = call;
                }
            } break;

            case Token::kIs: { // is
                MoveNext();
                auto type = ParseType(CHECK_OK);
                expr = new (arena_) Testing(expr, type, location.Concat(type->source_position()));
            } break;

            case Token::kAs: { // as
                MoveNext();
                auto type = ParseType(CHECK_OK);
                expr = new (arena_) Casting(expr, type, location.Concat(type->source_position()));
            } break;

            default:
                return expr;
        }
    }
}

Expression *Parser::ParsePrimary(bool *ok) {
    auto location = Peek().source_position();
    Expression *expr = nullptr;
    switch (Peek().kind()) {
        case Token::kLParen:
            return ParseParenOrLambdaLiteral(ok);
        case Token::kIdentifier:
            expr = new (arena_) Identifier(Peek().text_val(), location);
            MoveNext();
            break;
        case Token::kIntVal:
            expr = new (arena_) IntLiteral(static_cast<int>(Peek().i64_val()), location);
            MoveNext();
            break;
        case Token::kUIntVal:
            expr = new (arena_) UIntLiteral(static_cast<unsigned>(Peek().u64_val()), location);
            MoveNext();
            break;
        case Token::kF32Val:
            expr = new (arena_) F32Literal(Peek().f32_val(), location);
            MoveNext();
            break;
        case Token::kF64Val:
            expr = new (arena_) F64Literal(Peek().f64_val(), location);
            MoveNext();
            break;
        case Token::kStringLine:
        case Token::kStringBlock:
            expr = new (arena_) StringLiteral(Peek().text_val(), location);
            MoveNext();
            break;
        case Token::kStringTempletePrefix: // TODO:
            UNREACHABLE();
            break;
        default:
            error_feedback_->Printf(Peek().source_position(), "Unexpected primary expression, expected: %s",
                                    Peek().ToString().c_str());
            *ok = false;
            return nullptr;
    }
    return expr;
}

Expression *Parser::ParseParenOrLambdaLiteral(bool *ok) {
    auto location = Peek().source_position();
    Match(Token::kLParen, CHECK_OK);
    // lambda_literal ::= `(' argument_list `)' `->' expression
    // () -> expression
    if (Test(Token::kRParen)) {
        Match(Token::kRArrow, CHECK_OK);
        auto stmt = ParseStatement(CHECK_OK);
        auto prototype = new (arena_) FunctionPrototype(arena_, false, location);
        return new (arena_) LambdaLiteral(prototype, stmt, location.Concat(stmt->source_position()));
    }
    
    // (...) -> expression
    if (Test(Token::kVargs)) { // `...'
        location = location.Concat(Peek().source_position());
        Match(Token::kRParen, CHECK_OK);
        Match(Token::kRArrow, CHECK_OK);
        auto stmt = ParseStatement(CHECK_OK);
        auto prototype = new (arena_) FunctionPrototype(arena_, true, location);
        return new (arena_) LambdaLiteral(prototype, stmt, location.Concat(stmt->source_position()));
    }
    
    Expression *expr = nullptr;
    
    auto maybe_expr_location = Peek().source_position();
    // (id:type)->expression
    if (Peek().Is(Token::kIdentifier)) {
        auto id = MatchText(Token::kIdentifier, CHECK_OK);
        if (Test(Token::kColon)) {
            auto type = ParseType(CHECK_OK);
            
            auto prototype = new (arena_) FunctionPrototype(arena_, false, location);
            auto param = new (arena_) VariableDeclaration::Item(arena_, id, type, maybe_expr_location);
            prototype->mutable_params()->push_back(param);
            
            return ParseRemainLambdaLiteral(prototype, location, CHECK_OK);
        }
        expr = new (arena_) Identifier(id, maybe_expr_location);
    } else {
        expr = ParseExpression(CHECK_OK);
    }
    
    // (a,b,c...) -> expression
    if (Peek().Is(Token::kComma)) { // `,'
        auto prototype = new (arena_) FunctionPrototype(arena_, false, location);
        if (!expr->IsIdentifier()) {
            *ok = false;
            error_feedback_->Printf(expr->source_position(), "Unexpected 'argument' in function prototype");
            return nullptr;
        }
        
        auto param = new (arena_) VariableDeclaration::Item(arena_, expr->AsIdentifier()->name(), nullptr,
                                                            expr->source_position());
        prototype->mutable_params()->push_back(param);
        return ParseRemainLambdaLiteral(prototype, location, CHECK_OK);
    }
    
    
    Match(Token::kRParen, CHECK_OK);
    return expr; // Just only paren expression: `(' expr `)'
}

// if_statement ::= `if' `(' condition_clause `)' statement else_if_clause* eles_clause?
// condition_clause ::= variable_declaration `;' expression
//                    | expression
// else_if_clause ::= `else' `if' `(' condition_clause `)' statement
// eles_clause ::= `else' statement
IfExpression *Parser::ParseIfExpression(bool *ok) {
    auto location = Peek().source_position();

    Match(Token::kIf, CHECK_OK);
    Expression *condition = nullptr;
    Match(Token::kLParen, CHECK_OK);
    Statement *initializer_or_condition = ParseStatement(CHECK_OK);
    if (initializer_or_condition->IsAssignment() ||
        initializer_or_condition->IsVariableDeclaration()) {
        // Must be initializer
        Match(Token::kSemi, CHECK_OK);
        condition = ParseExpression(CHECK_OK);
    } else {
        if (!initializer_or_condition->IsExplicitExpression()) {
            *ok = false;
            error_feedback_->Printf(initializer_or_condition->source_position(), "Condition of if expression"
                                    " must be a expression.");
            return nullptr;
        }
        condition = static_cast<Expression *>(initializer_or_condition);
        initializer_or_condition = nullptr;
    }
    Match(Token::kRParen, CHECK_OK);
    
    auto then_clause = ParseStatement(CHECK_OK);
    location = location.Concat(then_clause->source_position());
    
    Statement *else_clause = nullptr;
    if (Test(Token::kElse)) {
        else_clause = ParseStatement(CHECK_OK);
        location = location.Concat(else_clause->source_position());
    }
    
    return new (arena_) IfExpression(initializer_or_condition, condition, then_clause, else_clause, location);
}

// when_statement ::= `when' `{' when_condition_clause+ when_else_clause? `}'
//                  | `when' `(' expression `)' `{' when_casting_clause+ when_else_clause? `}'
// when_condition_clause ::= expression `->' expression
// when_casting_clause ::= identifer `:' type_ref `->' expression
// when_between_to_clause ::= `in' expression `..' expression `->' expression
// when_else_clause ::= `else' `->' expression
WhenExpression *Parser::ParseWhenExpression(bool *ok) {
    auto location = Peek().source_position();
    
    Match(Token::kWhen, CHECK_OK);
    Statement *initializer_or_destination = nullptr;
    Expression *destination = nullptr;
    if (Test(Token::kLParen)) {
        initializer_or_destination = ParseStatement(CHECK_OK);
        if (initializer_or_destination->IsAssignment() ||
            initializer_or_destination->IsVariableDeclaration()) {
            // Must be initializer
            Match(Token::kSemi, CHECK_OK);
            destination = ParseExpression(CHECK_OK);
        } else {
            if (!initializer_or_destination->IsExplicitExpression()) {
                *ok = false;
                error_feedback_->Printf(initializer_or_destination->source_position(), "Destination of when expression"
                                        " must be a expression");
                return nullptr;
            }
            destination = static_cast<Expression *>(initializer_or_destination);
            initializer_or_destination = nullptr;
        }
        Match(Token::kRParen, CHECK_OK);
    }
    
    Match(Token::kLBrace, CHECK_OK);
    auto when = new (arena_) WhenExpression(arena_, initializer_or_destination, destination, location);
    while (!Test(Token::kRBrace)) {
        auto case_location = Peek().source_position();
        
        WhenExpression::Case *case_clause = nullptr;
        if (Test(Token::kIn)) { // `in' expression `..' expression -> statement
            auto lower = ParseExpression(CHECK_OK);
            bool close = false;
            if (Test(Token::kMore)) {
                close = true;
            } else {
                Match(Token::kUntil, CHECK_OK);
                close = false;
            }
            auto upper = ParseExpression(CHECK_OK);
            case_clause = new (arena_) WhenExpression::BetweenToCase(lower, upper, nullptr, close, location);
        } else if (Test(Token::kElse)) {
            Match(Token::kRArrow, CHECK_OK);
            auto else_clause = ParseStatement(CHECK_OK);
            when->set_else_clause(else_clause);
            Match(Token::kRBrace, CHECK_OK);
            break; // else clause mut be finally case.
        } else {
            auto match_value_or_id = ParseExpression(CHECK_OK);
            if (Test(Token::kColon)) { // id `:' type -> statement
                if (!match_value_or_id->IsIdentifier()) {
                    *ok = false;
                    error_feedback_->Printf(match_value_or_id->source_position(),
                                            "Type casting case must be a identifier");
                    return nullptr;
                }
                
                auto type = ParseType(CHECK_OK);
                case_clause = new (arena_) WhenExpression::TypeTestingCase(match_value_or_id->AsIdentifier(), type,
                                                                           nullptr, location);
            } else {
                case_clause = new (arena_) WhenExpression::ExpectValueCase(match_value_or_id, nullptr, location);
            }
        }

        DCHECK_NOTNULL(case_clause);
        Match(Token::kRArrow, CHECK_OK);
        auto then_clause = ParseStatement(CHECK_OK);
        case_clause->set_then_clause(then_clause);
        
        *case_clause->mutable_source_position() = case_location.Concat(then_clause->source_position());
        when->mutable_case_clauses()->push_back(case_clause);
        
        location = location.Concat(Peek().source_position());
    }
    
    *when->mutable_source_position() = location;
    return when;
}

//static bool EnsureType(Token::Kind kind) {
//    switch (kind) {
//        case Token::kUnit:
//        case Token::kBool:
//        case Token::kI8:
//        case Token::kU8:
//        case Token::kI16:
//        case Token::kU16:
//        case Token::kI32:
//        case Token::kU32:
//        case Token::kI64:
//        case Token::kU64:
//        case Token::kF32:
//        case Token::kF64:
//        case Token::kInt:
//        case Token::kUInt:
//        case Token::kString:
//        case Token::kIn:
//        case Token::kOut:
//        case Token::kChan:
//        case Token::kLParen:
//            return true;
//        default:
//            return false;
//    }
//}

bool Parser::ProbeInstantiation(bool *ok) {
    ProbeNext();
    do {
        ProbeType(CHECK_OK);
    } while (Test(Token::kComma));
    Probe(Token::kGreater, CHECK_OK);
    Probe(Token::kLParen, CHECK_OK);
    return true;
}

bool Parser::ProbeType(bool *ok) {
    switch (Peek().kind()) {
        case Token::kUnit:
        case Token::kBool:
        case Token::kI8:
        case Token::kU8:
        case Token::kI16:
        case Token::kU16:
        case Token::kI32:
        case Token::kU32:
        case Token::kI64:
        case Token::kU64:
        case Token::kF32:
        case Token::kF64:
        case Token::kInt:
        case Token::kUInt:
        case Token::kString:
            ProbeNext();
            break;
        case Token::kIn:
        case Token::kOut:
            ProbeNext();
            Probe(Token::kChan, CHECK_OK);
            Probe(Token::kLess, CHECK_OK);
            ProbeType(CHECK_OK);
            Probe(Token::kGreater, CHECK_OK);
            break;
        case Token::kChan:
            ProbeNext();
            Probe(Token::kLess, CHECK_OK);
            ProbeType(CHECK_OK);
            Probe(Token::kGreater, CHECK_OK);
            break;
        case Token::kLParen:
            ProbeNext();
            if (!Probe(Token::kRParen)) {
                do {
                    if (Probe(Token::kVargs)) {
                        break;
                    }
                } while (Probe(Token::kComma));
                Probe(Token::kRParen, CHECK_OK);
            }
            Probe(Token::kRArrow, CHECK_OK);
            if (Probe(Token::kLParen)) {
                do {
                    ProbeType(CHECK_OK);
                } while (Probe(Token::kComma));
                Probe(Token::kRParen, CHECK_OK);
            } else {
                ProbeType(CHECK_OK);
            }
            break;
        case Token::kIdentifier: {
            ProbeNext();
            if (Probe(Token::kDot)) {
                Probe(Token::kIdentifier, CHECK_OK);
            }
            if (Probe(Token::kLess)) {
                do {
                    ProbeType(CHECK_OK);
                } while (Probe(Token::kComma));
                Probe(Token::kGreater, CHECK_OK);
            }
        } break;
        default:
            *ok = false;
            return true;
    }
    
    if (Peek().Is(Token::kLBrack)) {
        while (Probe(Token::kLBrack)) {
            Probe(Token::kRBrack, CHECK_OK);
        }
    }
    return false;
}

void *Parser::ParseGenericParameters(base::ArenaVector<GenericParameter *> *params, bool *ok) {
    Match(Token::kLess, CHECK_OK);
    
    do {
        auto location = Peek().source_position();
        auto name = MatchText(Token::kIdentifier, CHECK_OK);
        Type *type = nullptr;
        if (Test(Token::kColon)) {
            type = ParseType(CHECK_OK);
        }
        
        if (type) {
            location = location.Concat(type->source_position());
        }
        params->push_back(new (arena_) GenericParameter(name, type, location));
    } while (Test(Token::kComma));
    
    Match(Token::kGreater, CHECK_OK);
    return nullptr;
}

Expression *Parser::ParseCommaSplittedExpressions(base::ArenaVector<Expression *> *list, Expression *receiver[2],
                                                  bool *ok) {
    Expression *expr = nullptr;
    int i = 0;
    do {
        expr = ParseExpression(ok);
        if (!*ok) {
            break;
        }
        if (i == 0) {
            receiver[i] = expr;
        }
        if (i == 1) {
            list->push_back(receiver[0]);
        }
        if (i > 0) {
            list->push_back(expr);
        }
        i++;
    } while (Test(Token::kComma));
    return expr;
}

Expression *Parser::ParseCommaSplittedExpressions(base::ArenaVector<Expression *> *receiver, bool *ok) {
    Expression *expr = nullptr;
    do {
        expr = ParseExpression(CHECK_OK);
        receiver->push_back(expr);
    } while (Test(Token::kComma));
    return expr;
}

Expression *Parser::ParseRemainLambdaLiteral(FunctionPrototype *prototype, const SourcePosition &location, bool *ok) {
    while (!Test(Token::kRParen)) {
        Match(Token::kComma, CHECK_OK);
        
        auto param_location = Peek().source_position();
        if (Test(Token::kVargs)) {
            prototype->set_vargs(true);
            Match(Token::kRParen, CHECK_OK);
            break;
        }
        
        auto name = MatchText(Token::kIdentifier, CHECK_OK);
        Type *type = nullptr;
        if (Test(Token::kColon)) {
            type = ParseType(CHECK_OK);
        } else {
            type = nullptr;
        }
        auto param = new (arena_) VariableDeclaration::Item(arena_, name, type, param_location);
        prototype->mutable_params()->push_back(param);
    }
    
    Match(Token::kRArrow, CHECK_OK);
    auto stmt = ParseStatement(CHECK_OK);
    return new (arena_) LambdaLiteral(prototype, stmt, location.Concat(stmt->source_position()));
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

Expression *Parser::NewExpressionWithOperands(const Operator &op, Expression *lhs, Expression *rhs,
                                              const SourcePosition &begin) {
    
    assert(op.operands == 2 || op.operands == 1);
    auto location = op.operands == 2 ? begin.Concat(rhs->source_position()) : begin.Concat(lhs->source_position());
    Expression *operand = op.operands == 1 ? lhs : nullptr;
    switch (op.kind) {
    #define DEFINE_CASE(name, base) \
        case Node::k##name: \
            return new (arena_) name (base##_PARAMS, location);
        
        DECLARE_EXPRESSION_WITH_OPERANDS(DEFINE_CASE)
            
    #undef DEFINE_CASE
            
        default:
            UNREACHABLE();
            break;
    }
    return nullptr;
}

int Parser::ParseDeclarationAccess() {
    Declaration::Access access = Access::kDefault;
    
    switch (Peek().kind()) {
        case Token::kExport:
            access = Access::kExport;
            MoveNext();
            break;
        case Token::kPublic:
            access = Access::kPublic;
            MoveNext();
            break;
        case Token::kPrivate:
            access = Access::kPrivate;
            MoveNext();
            break;
        case Token::kProtected:
            access = Access::kProtected;
            MoveNext();
            break;
        default:
            break;
    }
    
    return static_cast<int>(access);
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
    MoveNext();
}

bool Parser::Test(Token::Kind kind) {
    if (lookahead_.Is(kind)) {
        MoveNext();
        return true;
    }
    return false;
}

void Parser::MoveNext() {
    if (rollback_pos_ < rollback_depth_) {
        assert(rollback_depth_ > 0);
        
        lookahead_ = rollback_[rollback_pos_++];
        if (rollback_pos_ >= rollback_depth_) {
            rollback_pos_ = 0;
            rollback_depth_ = 0;
            // Rollback finish
        }
        return;
    }
    
    lookahead_ = lexer_->Next();
}

void Parser::Probe(Token::Kind kind, bool *ok) {
    if (rollback_depth_ >= kMaxRollbackDepth) {
        *ok = false;
        return;
    }
    if (lookahead_.IsNot(kind)) {
        *ok = false;
        return;
    }
    ProbeNext();
}

bool Parser::Probe(Token::Kind kind) {
    if (Peek().Is(kind)) {
        ProbeNext();
        return true;
    }
    return false;
}

void Parser::ProbeNext() {
    lookahead_ = lexer_->Next();
    rollback_[rollback_depth_++] = lookahead_;
}

} // namespace cpl

} // namespace yalx
