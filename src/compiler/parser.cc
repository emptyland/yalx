#include "compiler/parser.h"
#include "compiler/ast.h"
#include "compiler/syntax-feedback.h"
#include "compiler/lexer.h"
#include "compiler/constants.h"

namespace yalx {

namespace cpl {

#define CHECK_OK ok); if (!*ok) { return 0; } ((void)0

DECLARE_STATIC_STRING(kDashName, "_");
DECLARE_STATIC_STRING(kStarName, "*");

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
//        case Token::k2Exclamation:
//            return Operators::kAssertedGet;
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

Parser::~Parser() {}

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
    auto pkg = ParsePackageName(CHECK_OK);
    USE(pkg);
    while (lookahead_.kind() != Token::kEOF) {
        Token token = Peek();
        switch (token.kind()) {
                
            case Token::kImport:
                ParseImportStatement(CHECK_OK);
                break;
                
            case Token::kStruct: {
                auto stmt = ParseStructDefinition(CHECK_OK);
                file_unit_->Add(stmt);
            } break;
                
            case Token::kClass: {
                auto stmt = ParseClassDefinition(CHECK_OK);
                file_unit_->Add(stmt);
            } break;
                
            case Token::kEnum: {
                auto stmt = ParseEnumDefinition(CHECK_OK);
                file_unit_->Add(stmt);
            } break;
                
            case Token::kObject: {
                auto stmt = ParseObjectDeclaration(CHECK_OK);
                file_unit_->Add(stmt);
            } break;
                
            case Token::kInterface: {
                auto stmt = ParseInterfaceDefinition(CHECK_OK);
                file_unit_->Add(stmt);
            } break;
                
            case Token::kAtOutlined: {
                auto anno = ParseAnnotationDeclaration(CHECK_OK);
                auto access = static_cast<Access>(ParseDeclarationAccess());
                auto stmt = ParseOutsideStatement(CHECK_OK);
                if (Declaration::Is(stmt)) {
                    auto decl = static_cast<Declaration *>(stmt);
                    decl->set_annotations(anno);
                    decl->set_access(access);
                    for (size_t i = 0; i < decl->ItemSize(); i++) {
                        decl->AtItem(i)->set_owns(file_unit_);
                    }
                    // Declaration should has annotations.
                } else if (Definition::Is(stmt)) {
                    auto def = static_cast<Definition *>(stmt);
                    def->set_annotations(anno);
                    def->set_access(access);
                } else {
                    error_feedback_->Printf(anno->source_position(), "Incorrect annotation declaration, wrong position");
                    *ok = false;
                    return nullptr;
                }
                file_unit_->Add(stmt);
            } break;
                
            case Token::kExport:
            case Token::kPublic:
            case Token::kProtected:
            case Token::kPrivate: {
                auto access = static_cast<Access>(ParseDeclarationAccess());
                auto stmt = ParseOutsideStatement(CHECK_OK);
                if (Declaration::Is(stmt)) {
                    auto decl = static_cast<Declaration *>(stmt);
                    decl->set_access(access);
                    for (size_t i = 0; i < decl->ItemSize(); i++) {
                        decl->AtItem(i)->set_owns(file_unit_);
                    }
                    // Declaration should has annotations.
                } else if (Definition::Is(stmt)) {
                    static_cast<Definition *>(stmt)->set_access(access);
                } else {
                    error_feedback_->Printf(stmt->source_position(), "Incorrect access declaration, wrong position");
                    *ok = false;
                    return nullptr;
                }
                file_unit_->Add(stmt);
            } break;
                
            case Token::kNative:
            case Token::kFun: {
                auto fun = ParseFunctionDeclaration(CHECK_OK);
                file_unit_->Add(fun);
            } break;
                
            case Token::kVolatile:
            case Token::kVal:
            case Token::kVar: {
                auto decl = ParseVariableDeclaration(CHECK_OK);
                for (size_t i = 0; i < decl->ItemSize(); i++) {
                    decl->AtItem(i)->set_owns(file_unit_);
                }
                file_unit_->Add(decl);
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
    //auto location = Peek().source_position();
    switch (Peek().kind()) {
        case Token::kInterface:
            return ParseInterfaceDefinition(ok);
            
        case Token::kObject:
            return ParseObjectDeclaration(ok);
            
        case Token::kStruct:
            return ParseStructDefinition(ok);
            
        case Token::kClass:
            return ParseClassDefinition(ok);
            
        case Token::kEnum:
            return ParseEnumDefinition(ok);
            
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
            
            auto param = new (arena_) VariableDeclaration::Item(arena_, nullptr, name, type,
                                                                param_location.Concat(type->source_position()));
            param->set_annotations(anno);
            
            prototype->mutable_params()->push_back(param);
        } while (Test(Token::kComma));
        
        Match(Token::kRParen, CHECK_OK);
    }
    
    if (Test(Token::kColon)) {
        do {
            auto type = ParseType(CHECK_OK);
            prototype->mutable_return_types()->push_back(type);
        } while (Test(Token::kComma));
    }
    return prototype;
}


// interface_definition ::= `interface' identifer generic_declaration? udi_block
// udi_block ::= `{' udi_item+ `}'
// udi_item ::= identifier function_prototype
InterfaceDefinition *Parser::ParseInterfaceDefinition(bool *ok) {
    auto location = Peek().source_position();
    
    Match(Token::kInterface, CHECK_OK);
    auto name = MatchText(Token::kIdentifier, CHECK_OK);
    auto def = new (arena_) InterfaceDefinition(arena_, name, location);
    
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
    auto def = new (arena_) AnnotationDefinition(arena_, name, location);
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
        
        auto field = new (arena_) VariableDeclaration::Item(arena_, nullptr, id, type, member_location);
        def->mutable_members()->push_back({field, default_value});
        
        location = location.Concat(Peek().source_position());
    }
    
    *def->mutable_source_position() = location;
    return def;
}

// class_definition ::= `class' identifer generic_declaration? constructor? super_declaration? udt_block
// struct_definition ::= `struct' identifer generic_declaration? constructor? super_declaration? udt_block
// object_definition ::= `object' identifer udt_block
// constructor ::= `(' constructor_member_definition* `)'
// constructor_member_definition ::= annotation_declaration? access_description? (`val' | `var') identifer `:' type_ref
//                                | argument
// super_declaration ::= `:' symbol ( `(' expression_list? `)' )?
// udt_block ::= `{' udt_item* `}'
// udt_item ::= member_definition | method_definition
// member_definition ::= annotation_declaration? access_description? `override' variable_declaration
// method_definition ::= annotation_declaration? `override'?  access_description? ( function_definition | function_declaration)
// access_description ::= `public' | `private' | `protected'
ObjectDeclaration *Parser::ParseObjectDeclaration(bool *ok) {
    auto location = Peek().source_position();
    Match(Token::kObject, CHECK_OK);
    auto name = MatchText(Token::kIdentifier, CHECK_OK);
    auto decl = new (arena_) ObjectDeclaration(arena_, name, location);
    Match(Token::kLBrace, CHECK_OK);
    
    while (!Test(Token::kRBrace)) {
        AnnotationDeclaration *anno = nullptr;
        if (Peek().Is(Token::kAtOutlined)) {
            anno = ParseAnnotationDeclaration(CHECK_OK);
        }
        auto access = static_cast<Access>(ParseDeclarationAccess());
        
        switch (Peek().kind()) {
                // val a, b, c = 1, 2, 3
                // private val name, alias = "setup", "dom"
            case Token::kVolatile:
            case Token::kVal:
            case Token::kVar: {
                auto field = ParseVariableDeclaration(CHECK_OK);
                decl->mutable_fields()->push_back(field);
            } break;
                
            default: {
                auto fun = ParseFunctionDeclaration(CHECK_OK);
                fun->set_access(access);
                fun->set_annotations(anno);
                decl->mutable_methods()->push_back(fun);
            } break;
        }
    }
    return decl;
}

StructDefinition *Parser::ParseStructDefinition(bool *ok) {
    auto location = Peek().source_position();
    Match(Token::kStruct, CHECK_OK);
    auto name = MatchText(Token::kIdentifier, CHECK_OK);
    auto def = new (arena_) StructDefinition(arena_, name, location);
    ParseIncompletableDefinition(def, nullptr/* concepts */, CHECK_OK);
    return def;
}

ClassDefinition *Parser::ParseClassDefinition(bool *ok) {
    auto location = Peek().source_position();
    Match(Token::kClass, CHECK_OK);
    auto name = MatchText(Token::kIdentifier, CHECK_OK);
    auto def = new (arena_) ClassDefinition(arena_, name, location);
    ParseIncompletableDefinition(def, def->mutable_concepts(), CHECK_OK);
    return def;
}

/*
 enum Name<T> {
    Value1,
    Value2,
    Value3,
    Value4(i32),
    Value5(string)
 
    fun unwarp() {...}
 }
 */
EnumDefinition *Parser::ParseEnumDefinition(bool *ok) {
    auto location = Peek().source_position();
    Match(Token::kEnum, CHECK_OK);
    auto name = MatchText(Token::kIdentifier, CHECK_OK);
    auto def = new (arena_) EnumDefinition(arena_, name, location);
    if (Peek().Is(Token::kLess)) {
        ParseGenericParameters(def->mutable_generic_params(), CHECK_OK);
    }
    
    Match(Token::kLBrace, CHECK_OK); // {
    
    do {
        auto val_location = Peek().source_position();
        auto val_name = MatchText(Token::kIdentifier, CHECK_OK);
        auto val = new (arena_) VariableDeclaration(arena_, false, VariableDeclaration::kVal, location);
        val->set_name(val_name);
        if (Test(Token::kLParen)) { // (
            
            do {
                auto ty = ParseType(CHECK_OK);
                auto item = new (arena_) VariableDeclaration::Item(arena_, val, kDashName, ty, ty->source_position());
                val->mutable_variables()->push_back(item);
            } while (Test(Token::kComma));
            val_location = val_location.Concat(Peek().source_position());
            Match(Token::kRParen, CHECK_OK); // )
        } else {
            auto unit = new (arena_) Type(arena_, Type::kType_unit, val_location);
            auto item = new (arena_) VariableDeclaration::Item(arena_, val, kDashName, unit, unit->source_position());
            val->mutable_variables()->push_back(item);
        }
        
        *val->mutable_source_position() = val_location;
        IncompletableDefinition::Field field;
        field.in_constructor = false;
        field.declaration = val;
        val->set_owns(def);
        def->mutable_fields()->push_back(field);
    } while (Test(Token::kComma));
    
    while (Peek().IsNot(Token::kRBrace)) {
        AnnotationDeclaration *anno = nullptr;
        if (Peek().Is(Token::kAtOutlined)) {
            anno = ParseAnnotationDeclaration(CHECK_OK);
        }
        auto access = static_cast<Access>(ParseDeclarationAccess());
        
        auto fun = ParseFunctionDeclaration(CHECK_OK);
        fun->set_access(access);
        fun->set_annotations(anno);
        fun->set_owns(def);
        def->mutable_methods()->push_back(fun);
    }
    
    Match(Token::kRBrace, CHECK_OK); // }
    return def;
}

IncompletableDefinition *Parser::ParseIncompletableDefinition(IncompletableDefinition *def,
                                                              base::ArenaVector<Type *> *concepts, bool *ok) {
    auto location = def->source_position();
    
    if (Peek().Is(Token::kLess)) {
        ParseGenericParameters(def->mutable_generic_params(), CHECK_OK);
    }
    
    if (Test(Token::kLParen)) {
        do {
            auto arg_location = Peek().source_position();
            AnnotationDeclaration *anno = nullptr;
            if (Peek().Is(Token::kAtOutlined)) {
                anno = ParseAnnotationDeclaration(CHECK_OK);
            }
            
            IncompletableDefinition::Parameter param;
            auto access = static_cast<Access>(ParseDeclarationAccess());
            USE(access); // FIXME: use access
            if (Peek().Is(Token::kVal) || Peek().Is(Token::kVar) || Peek().Is(Token::kVolatile)) {
                bool is_volatile = false;
                VariableDeclaration::Constraint constraint;
                if (Test(Token::kVolatile)) {
                    Match(Token::kVar, CHECK_OK);
                    constraint = VariableDeclaration::kVar;
                    is_volatile = true;
                } else {
                    if (Test(Token::kVal)) {
                        constraint = VariableDeclaration::kVal;
                    } else {
                        Match(Token::kVar, CHECK_OK);
                        constraint = VariableDeclaration::kVar;
                    }
                }
                auto id = MatchText(Token::kIdentifier, CHECK_OK);
                Match(Token::kColon, CHECK_OK);
                auto type = ParseType(CHECK_OK);
                auto var = new (arena_) VariableDeclaration(arena_, is_volatile, constraint, id, type, location);
                
                IncompletableDefinition::Field field;
                field.as_constructor = static_cast<int>(def->parameters_size());
                field.in_constructor = true;
                field.declaration = var;
                
                param.field_declaration = true;
                param.as_field = static_cast<int>(def->fields_size());
                
                def->mutable_fields()->push_back(field);
            } else {
                auto id = MatchText(Token::kIdentifier, CHECK_OK);
                Match(Token::kColon, CHECK_OK);
                auto type = ParseType(CHECK_OK);
                
                param.field_declaration = false;
                param.as_parameter = new (arena_) VariableDeclaration::Item(arena_, nullptr, id, type,
                                                                            arg_location.Concat(type->source_position()));
            }
            
            // TODO:
            def->mutable_parameters()->push_back(param);
        } while (Test(Token::kComma));
        Match(Token::kRParen, CHECK_OK);
    }
    
    if (Test(Token::kColon)) {
        auto symbol = ParseSymbol(CHECK_OK);
        Expression *callee = nullptr;
        if (symbol->prefix_name()) {
            auto id = new (arena_) Identifier(symbol->prefix_name(), symbol->source_position());
            callee = new (arena_) Dot(id, symbol->name(), symbol->source_position());
        } else {
            callee = new (arena_) Identifier(symbol->name(), symbol->source_position());
        }
        
        auto inst_location = Peek().source_position();
        if (Test(Token::kLess)) {
            auto inst = new (arena_) Instantiation(arena_, callee, inst_location);
            do {
                auto type = ParseType(CHECK_OK);
                inst->mutable_generic_args()->push_back(type);
            } while (Test(Token::kComma));
            callee = inst;
            Match(Token::kGreater, CHECK_OK);
        }
        
        if (Test(Token::kLParen)) {
            auto calling = new (arena_) Calling(arena_, callee, symbol->source_position().Concat(Peek().source_position()));
            if (!Test(Token::kRParen)) {
                do {
                    auto arg = ParseExpression(CHECK_OK);
                    calling->mutable_args()->push_back(arg);
                } while (Test(Token::kComma));
                
                *calling->mutable_source_position() = symbol->source_position().Concat(Peek().source_position());
                Match(Token::kRParen, CHECK_OK);
            }
            def->set_super_calling(calling);
        } else {
            assert(concepts);
            auto inst = callee->AsInstantiation();
            auto maybe_symbol = callee;
            if (inst) {
                maybe_symbol = inst->primary();
            }
            
            Symbol *symbol = nullptr;
            if (auto id = maybe_symbol->AsIdentifier()) {
                symbol = new (arena_) Symbol(id->name(), id->source_position());
            } else if (auto dot = maybe_symbol->AsDot()) {
                symbol = new (arena_) Symbol(dot->primary()->AsIdentifier()->name(), dot->field(),
                                             dot->source_position());
            } else {
                UNREACHABLE();
            }
            auto concept = new (arena_) Type(arena_, symbol, symbol->source_position());
            if (inst) {
                for (auto garg : inst->generic_args()) {
                    concept->mutable_generic_args()->push_back(garg);
                }
            }
            concepts->push_back(concept);
        }
        
        if (concepts) {
            while (Test(Token::kComma)) {
                auto symbol = ParseSymbol(CHECK_OK);
                auto type = new (arena_) Type(arena_, symbol, symbol->source_position());
                if (Test(Token::kLess)) {
                    do {
                        auto arg = ParseType(CHECK_OK);
                        type->mutable_generic_args()->push_back(arg);
                    } while (Test(Token::kComma));
                    Match(Token::kGreater, CHECK_OK);
                }
                concepts->push_back(type);
            }
        }
    }
    
    if (Test(Token::kLBrace)) {
        while (!Test(Token::kRBrace)) {
            AnnotationDeclaration *anno = nullptr;
            if (Peek().Is(Token::kAtOutlined)) {
                anno = ParseAnnotationDeclaration(CHECK_OK);
            }
            auto access = static_cast<Access>(ParseDeclarationAccess());
            
            switch (Peek().kind()) {
                    // val a, b, c = 1, 2, 3
                    // private val name, alias = "setup", "dom"
                case Token::kVolatile:
                case Token::kVal:
                case Token::kVar: {
                    auto var = ParseVariableDeclaration(CHECK_OK);
                    var->set_access(access);
                    var->set_annotations(anno);
                    IncompletableDefinition::Field field;
                    field.in_constructor = false;
                    field.declaration = var;
                    def->mutable_fields()->push_back(field);
                } break;
                    
                default: {
                    auto fun = ParseFunctionDeclaration(CHECK_OK);
                    fun->set_access(access);
                    fun->set_annotations(anno);
                    def->mutable_methods()->push_back(fun);
                } break;
            }
        }
    }
    return def;
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
        auto item = new (arena_) VariableDeclaration::Item(arena_, decl, name, type,
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

// block ::= `{' statement* `}'
// statement ::= if_statement
//             | when_statement
//             | for_statement
//             | while_statement
//             | `run` call
//             | `return' expression_list?
//             | `throw' expression
//             | `break'
//             | `continue'
//             | expression `<-' expression
Statement *Parser::ParseStatement(bool *ok) {
    auto location = Peek().source_position();
    switch (Peek().kind()) {
        case Token::kLBrace:
            return ParseBlock(ok);
            
        case Token::kVolatile:
        case Token::kVal:
        case Token::kVar:
            return ParseVariableDeclaration(ok);
            
        case Token::kReturn: {
            MoveNext();
            auto stmt = new (arena_) Return(arena_, location);
            // return Unit
            if (Peek().Is(Token::kRBrace) || Peek().Is(Token::kSemi)) {
                if (Peek().Is(Token::kSemi)) {
                    MoveNext();
                }
                return stmt;
            }
            ParseCommaSplittedExpressions(stmt->mutable_returnning_vals(), CHECK_OK);
            *stmt->mutable_source_position() = location.Concat(stmt->returnning_vals().back()->source_position());
            return stmt;
        } break;
            
        case Token::kThrow: {
            MoveNext();
            auto throwing_val = ParseExpression(CHECK_OK);
            return new (arena_) Throw(throwing_val, location.Concat(throwing_val->source_position()));
        } break;
            
        case Token::kRun:
            return ParseRunStatement(ok);
            
        case Token::kWhile:
            return ParseWhileLoop(ok);
            
        case Token::kUnless:
            return ParseUnlessLoop(ok);
            
        case Token::kFor:
            return ParseForeachLoop(ok);
            
        case Token::kDo:
            return ParseDoConditionLoop(ok);
            
        case Token::kBreak:
            MoveNext();
            return new (arena_) Break(location);
            
        case Token::kContinue:
            MoveNext();
            return new (arena_) Continue(location);
            
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

Block *Parser::ParseBlock(bool *ok) {
    auto location = Peek().source_position();
    Match(Token::kLBrace, CHECK_OK);
    auto block = new (arena_) Block(arena_, location);
    while (!Test(Token::kRBrace)) {
        auto stmt = ParseStatement(CHECK_OK);
        block->mutable_statements()->push_back(stmt);
        
        *block->mutable_source_position() = location.Concat(Peek().source_position());
    }
    return block;
}

WhileLoop *Parser::ParseWhileLoop(bool *ok) {
    auto location = Peek().source_position();
    Match(Token::kWhile, CHECK_OK);
    auto loop = new (arena_) WhileLoop(nullptr/*init*/, false/*execute_first*/, nullptr/*condition*/, nullptr/*body*/,
                                       location);
    ParseConditionLoop(loop, CHECK_OK);
    *loop->mutable_source_position() = location.Concat(loop->body()->source_position());
    return loop;
}

UnlessLoop *Parser::ParseUnlessLoop(bool *ok) {
    auto location = Peek().source_position();
    Match(Token::kUnless, CHECK_OK);
    auto loop = new (arena_) UnlessLoop(nullptr/*init*/, false/*execute_first*/, nullptr/*condition*/, nullptr/*body*/,
                                        location);
    ParseConditionLoop(loop, CHECK_OK);
    *loop->mutable_source_position() = location.Concat(loop->body()->source_position());
    return loop;
}

ConditionLoop *Parser::ParseConditionLoop(ConditionLoop *loop, bool *ok) {
    Match(Token::kLParen, CHECK_OK);
    Expression *condition = nullptr;
    Statement *initializer = ParseInitializerIfExistsWithCondition(&condition, CHECK_OK);
    Match(Token::kRParen, CHECK_OK);
    
    loop->set_condition(condition);
    loop->set_initializer(initializer);
    
    auto block = ParseBlock(CHECK_OK);
    loop->set_body(block);
    return loop;
}

ConditionLoop *Parser::ParseDoConditionLoop(bool *ok) {
    auto location = Peek().source_position();
    Match(Token::kDo, CHECK_OK);
    
    auto block = ParseBlock(CHECK_OK);
    
    bool while_or_unless = false;
    if (Test(Token::kWhile)) {
        while_or_unless = true;
    } else {
        Match(Token::kUnless, CHECK_OK);
        while_or_unless = false;
    }
    
    Match(Token::kLParen, CHECK_OK);
    auto condition = ParseExpression(CHECK_OK);
    location = location.Concat(Peek().source_position());
    Match(Token::kRParen, CHECK_OK);
    
    ConditionLoop *loop = nullptr;
    if (while_or_unless) {
        loop = new (arena_) WhileLoop(nullptr/*init*/, true/*execute_first*/, condition, block, location);
    } else {
        loop = new (arena_) UnlessLoop(nullptr/*init*/, true/*execute_first*/, condition, block, location);
    }
    return loop;
}

RunCoroutine *Parser::ParseRunStatement(bool *ok) {
    auto location = Peek().source_position();
    Match(Token::kRun, CHECK_OK);
    auto maybe_calling = ParseExpression(CHECK_OK);
    if (!maybe_calling->IsCalling()) {
        *ok = false;
        error_feedback_->Printf(maybe_calling->source_position(), "Unexpected 'calling' for coroutine entry");
        return nullptr;
    }
    return new (arena_) RunCoroutine(maybe_calling->AsCalling(), location.Concat(maybe_calling->source_position()));
}

// for_statement ::= `for' `(' identifer `in' expression `)' block
//                 | `for' `(' identifer `in' expression `until' expression `)' block
//                 | `for' `(' identifer `in' expression `..' expression `)' block
ForeachLoop *Parser::ParseForeachLoop(bool *ok) {
    auto location = Peek().source_position();
    
    Match(Token::kFor, CHECK_OK);
    Match(Token::kLParen, CHECK_OK);
    
    auto id_location = Peek().source_position();
    auto name = MatchText(Token::kIdentifier, CHECK_OK);
    auto id = new (arena_) Identifier(name, id_location);
    
    Match(Token::kIn, CHECK_OK);
    
    auto iterable_or_lower = ParseExpression(CHECK_OK);
    ForeachLoop::IntRange range;
    if (Peek().Is(Token::kUntil) || Peek().Is(Token::kMore)) {
        if (Test(Token::kUntil)) {
            range.close = false;
        } else {
            MoveNext();
            range.close = true;
        }
        range.lower = iterable_or_lower;
        range.upper = ParseExpression(CHECK_OK);
    }
    
    Match(Token::kRParen, CHECK_OK);
    
    auto block = ParseBlock(CHECK_OK);
    
    location.Concat(block->source_position());
    if (range.lower != nullptr && range.upper != nullptr) {
        return new (arena_) ForeachLoop(id, range, block, location);
    } else {
        return new (arena_) ForeachLoop(id, iterable_or_lower, block, location);
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
            auto literal = new (arena_) BoolLiteral(arena_, true, location);
            MoveNext();
            return literal;
        } break;
            
        case Token::kFalse: {
            auto literal = new (arena_) BoolLiteral(arena_, false, location);
            MoveNext();
            return literal;
        } break;
            
        case Token::kUnitVal:
            MoveNext();
            return new (arena_) UnitLiteral(arena_, location);

//        case Token::kNone:
//            MoveNext();
//            return new (arena_) OptionLiteral(arena_, nullptr/*value*/, location);
//
//        case Token::kSome: {
//            MoveNext();
//            Match(Token::kLParen, CHECK_OK);
//            auto expr = ParseExpression(CHECK_OK);
//            location = location.Concat(Peek().source_position());
//            Match(Token::kRParen, CHECK_OK);
//            return new (arena_) OptionLiteral(arena_, expr, location);
//        } break;
            
        case Token::kIf:
            return ParseIfExpression(ok);
            
        case Token::kTry:
            return ParseTryCatchExpression(ok);
            
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
                
            case Token::k2Colon: { // ::
                MoveNext();
                auto dot_location = location.Concat(Peek().source_position());
                auto field = MatchText(Token::kIdentifier, CHECK_OK);
                expr = new (arena_) Resolving(expr, field, dot_location);
            } break;
                
            case Token::kLBrack: { // [
                MoveNext();
                auto idx_location = location.Concat(Peek().source_position());
                auto get = new (arena_) IndexedGet(arena_, expr, idx_location);
                do {
                    Expression *index = ParseExpression(CHECK_OK);
                    get->mutable_indexs()->push_back(index);
                } while (Test(Token::kComma));
                Match(Token::kRBrack, CHECK_OK);
                expr = get;
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
    if (Peek().Is(Token::kIdentifier)) {
        auto expr = new (arena_) Identifier(Peek().text_val(), location);
        MoveNext();
        return expr;
    }
    
    if (Peek().Is(Token::kChan)) {
        auto type = ParseType(CHECK_OK);
        Match(Token::kLParen, CHECK_OK);
        auto capacity = ParseExpression(CHECK_OK);
        location = location.Concat(Peek().source_position());
        Match(Token::kRParen, CHECK_OK);
        return new (arena_) ChannelInitializer(type, capacity, location);
    }
    
    Expression *expr = nullptr;
    switch (Peek().kind()) {
        case Token::kLParen:
            return ParseParenOrLambdaLiteral(ok);
        case Token::kLBrace:
            return ParseArrayInitializer(nullptr, kMaxArrayInitializerDims, CHECK_OK);
        case Token::kIdentifier:
            expr = new (arena_) Identifier(Peek().text_val(), location);
            MoveNext();
            break;
        case Token::kI8Val:
            expr = new (arena_) I8Literal(arena_, Peek().i8_val(), location);
            MoveNext();
            break;
        case Token::kU8Val:
            expr = new (arena_) U8Literal(arena_, Peek().u8_val(), location);
            MoveNext();
            break;
        case Token::kI16Val:
            expr = new (arena_) I16Literal(arena_, Peek().i16_val(), location);
            MoveNext();
            break;
        case Token::kU16Val:
            expr = new (arena_) U16Literal(arena_, Peek().u16_val(), location);
            MoveNext();
            break;
        case Token::kI32Val:
            expr = new (arena_) I32Literal(arena_, Peek().i32_val(), location);
            MoveNext();
            break;
        case Token::kU32Val:
            expr = new (arena_) U32Literal(arena_, Peek().u32_val(), location);
            MoveNext();
            break;
        case Token::kIntVal:
            expr = new (arena_) IntLiteral(arena_, Peek().i32_val(), location);
            MoveNext();
            break;
        case Token::kUIntVal:
            expr = new (arena_) UIntLiteral(arena_, Peek().u32_val(), location);
            MoveNext();
            break;
        case Token::kI64Val:
            expr = new (arena_) I64Literal(arena_, Peek().i64_val(), location);
            MoveNext();
            break;
        case Token::kU64Val:
            expr = new (arena_) U64Literal(arena_, Peek().u64_val(), location);
            MoveNext();
            break;
        case Token::kF32Val:
            expr = new (arena_) F32Literal(arena_, Peek().f32_val(), location);
            MoveNext();
            break;
        case Token::kF64Val:
            expr = new (arena_) F64Literal(arena_, Peek().f64_val(), location);
            MoveNext();
            break;
        case Token::kCharVal:
            expr = new (arena_) CharLiteral(arena_, Peek().char_val(), location);
            MoveNext();
            break;
        case Token::kAtOutlined: {
            MoveNext();
            auto type = ParseArrayTypeMaybeWithLimits(CHECK_OK);
            expr = ParseArrayInitializer(type, type->GetActualDimensionCount(), CHECK_OK);
        } break;
            
        case Token::kStringLine:
        case Token::kStringBlock:
            expr = new (arena_) StringLiteral(arena_, Peek().text_val(), location);
            MoveNext();
            break;
        case Token::kStringTempletePrefix:
            return ParseStringTemplate(ok);
        default:
            error_feedback_->Printf(Peek().source_position(), "Unexpected primary expression, expected: %s",
                                    Peek().ToString().c_str());
            *ok = false;
            return nullptr;
    }
    return expr;
}

Expression *Parser::ParseArrayInitializer(ArrayType *qualified, int dimension_limit, bool *ok) {
    if (dimension_limit == 0) {
        error_feedback_->Printf(Peek().source_position(), "Max array initializer dimensions");
        *ok = false;
        return nullptr;
    }
    
    auto location = Peek().source_position();
    if (qualified) {
        location = qualified->source_position();
        if (qualified->HasCapacities()) {
            Match(Token::kLParen, CHECK_OK);
            auto filling = ParseExpression(CHECK_OK);
            location.Concat(Peek().source_position());
            Match(Token::kRParen, CHECK_OK);
            auto init = new (arena_) ArrayInitializer(arena_, qualified, qualified->dimension_count(), location);
            init->set_filling_value(filling);
            return init;
        }
    }
    Match(Token::kLBrace, CHECK_OK);
    
    auto dimension_count = !qualified ? 1 : qualified->GetActualDimensionCount();
    auto init = new (arena_) ArrayInitializer(arena_, qualified, dimension_count, location);
    if (Peek().Is(Token::kLBrace)) {
        do {
            auto dim = ParseArrayInitializer(nullptr, dimension_limit - 1, CHECK_OK);
            init->mutable_dimensions()->push_back(dim);
        } while(Test(Token::kComma));
    } else if (Peek().IsNot(Token::kRBrace)) {
        do {
            auto expr = ParseExpression(CHECK_OK);
            init->mutable_dimensions()->push_back(expr);
        } while(Test(Token::kComma));
    }
    
    *init->mutable_source_position() = location.Concat(Peek().source_position());
    Match(Token::kRBrace, CHECK_OK);
    return init;
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
        return new (arena_) LambdaLiteral(arena_, prototype, stmt, location.Concat(stmt->source_position()));
    }
    
    // (...) -> expression
    if (Test(Token::kVargs)) { // `...'
        location = location.Concat(Peek().source_position());
        Match(Token::kRParen, CHECK_OK);
        Match(Token::kRArrow, CHECK_OK);
        auto stmt = ParseStatement(CHECK_OK);
        auto prototype = new (arena_) FunctionPrototype(arena_, true, location);
        return new (arena_) LambdaLiteral(arena_, prototype, stmt, location.Concat(stmt->source_position()));
    }
    
    Expression *expr = nullptr;
    
    auto maybe_expr_location = Peek().source_position();
    // (id:type)->expression
    if (Peek().Is(Token::kIdentifier)) {
        auto id = MatchText(Token::kIdentifier, CHECK_OK);
        if (Test(Token::kColon)) {
            auto type = ParseType(CHECK_OK);
            
            auto prototype = new (arena_) FunctionPrototype(arena_, false, location);
            auto param = new (arena_) VariableDeclaration::Item(arena_, nullptr, id, type, maybe_expr_location);
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
        
        auto param = new (arena_) VariableDeclaration::Item(arena_, nullptr, expr->AsIdentifier()->name(), nullptr,
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
    Match(Token::kLParen, CHECK_OK);
    Expression *condition = nullptr;
    Statement *initializer = ParseInitializerIfExistsWithCondition(&condition, CHECK_OK);
    Match(Token::kRParen, CHECK_OK);
    
    auto then_clause = ParseStatement(CHECK_OK);
    location = location.Concat(then_clause->source_position());
    
    Statement *else_clause = nullptr;
    if (Test(Token::kElse)) {
        else_clause = ParseStatement(CHECK_OK);
        location = location.Concat(else_clause->source_position());
    }
    
    return new (arena_) IfExpression(arena_, initializer, condition, then_clause, else_clause, location);
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
    Statement *initializer = nullptr;
    Expression *destination = nullptr;
    if (Test(Token::kLParen)) {
        initializer = ParseInitializerIfExistsWithCondition(&destination, CHECK_OK);
        Match(Token::kRParen, CHECK_OK);
    }
    
    Match(Token::kLBrace, CHECK_OK);
    auto when = new (arena_) WhenExpression(arena_, initializer, destination, location);
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
                                                                           nullptr, case_location);
            } else if (Test(Token::kLBrace)) {
                auto symbol = EnsureToSymbol(match_value_or_id, CHECK_OK);
                auto type = new (arena_) Type(arena_, symbol, case_location);
                auto clause = new (arena_) WhenExpression::StructMatchingCase(arena_, type, nullptr, case_location);
                do {
                    auto field_name = ParseIdentifier(CHECK_OK);
                    clause->mutable_expecteds()->push_back(field_name);
                } while (Test(Token::kComma));
                Match(Token::kRBrace, CHECK_OK);
                case_clause = clause;
            } else {
                auto clause = new (arena_) WhenExpression::ExpectValuesCase(arena_, nullptr, case_location);
                clause->mutable_match_values()->push_back(match_value_or_id);
                while (Test(Token::kComma)) {
                    auto match_value = ParseExpression(CHECK_OK);
                    clause->mutable_match_values()->push_back(match_value);
                }
                *clause->mutable_source_position() = case_location.Concat(clause->match_values().back()->source_position());
                case_clause = clause;
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

// try_catch_statement ::= `try' block catch_clause+ finally_clause?
// catch_clause ::= `catch' `(' identifier `:' type_ref `)' block
// finally_clause ::= `finally' block
TryCatchExpression *Parser::ParseTryCatchExpression(bool *ok) {
    auto location = Peek().source_position();
    
    Match(Token::kTry, CHECK_OK);
    auto try_block = ParseBlock(CHECK_OK);
    auto try_catch = new (arena_) TryCatchExpression(arena_, try_block, nullptr, location);
    auto catch_location = Peek().source_position();
    while (Test(Token::kCatch)) { // catch clause
        Match(Token::kLParen, CHECK_OK);
        auto id = ParseIdentifier(CHECK_OK);
        Match(Token::kColon, CHECK_OK);
        auto type = ParseType(CHECK_OK);
        Match(Token::kRParen, CHECK_OK);
        auto block = ParseBlock(CHECK_OK);
        auto catch_clause = new (arena_) TryCatchExpression::CatchClause(id, type, block,
                                                                         catch_location.Concat(block->source_position()));
        try_catch->mutable_catch_clauses()->push_back(catch_clause);
        
        *try_catch->mutable_source_position() = location.Concat(catch_location);
        catch_location = Peek().source_position();
    }
    
    if (Test(Token::kFinally)) {
        auto block = ParseBlock(CHECK_OK);
        try_catch->set_finally_block(block);
        
        *try_catch->mutable_source_position() = location.Concat(block->source_position());
    }
    
    if (try_catch->catch_clauses().empty() && !try_catch->finally_block()) {
        *ok = false;
        error_feedback_->Printf(try_catch->source_position(), "Unexpected any 'catch-clause' or 'finally-clause'");
        return nullptr;
    }
    return try_catch;
}

StringTemplate *Parser::ParseStringTemplate(bool *ok) {
    auto location = Peek().source_position();
    
    auto literal = MatchText(Token::kStringTempletePrefix, CHECK_OK);
    auto tmpl = new (arena_) StringTemplate(arena_, location);
    tmpl->mutable_parts()->push_back(new (arena_) StringLiteral(arena_, literal, location));
    for (;;) {
        auto part_location = Peek().source_position();
        Expression *part = nullptr;
        switch (Peek().kind()) {
            case Token::kStringTempletePart: {
                literal = Peek().text_val();
                MoveNext();
                part = new (arena_) StringLiteral(arena_, literal, part_location);
            } break;
                
            case Token::kIdentifier:
                part = ParseExpression(CHECK_OK);
                break;
                
            case Token::kStringTempleteExpressBegin: {
                MoveNext();
                part = ParseExpression(CHECK_OK);
                Match(Token::kStringTempleteExpressEnd, CHECK_OK);
            } break;
                
            case Token::kStringTempleteSuffix: {
                literal = Peek().text_val();
                MoveNext();
                if (literal->size() > 0) {
                    part = new (arena_) StringLiteral(arena_, literal, part_location);
                    tmpl->mutable_parts()->push_back(part);
                }
            } goto done;
                
            default:
                UNREACHABLE();
                break;
        }
        tmpl->mutable_parts()->push_back(part);
    }
done:
    *tmpl->mutable_source_position() = location.Concat(tmpl->parts().back()->source_position());
    return tmpl;
}

bool Parser::ProbeInstantiation(bool *ok) {
    ProbeNext();
    do {
        ProbeType(CHECK_OK);
    } while (Test(Token::kComma));
    Probe(Token::kGreater, CHECK_OK);
    if (!Probe(Token::k2Colon)) {
        Probe(Token::kLParen, CHECK_OK);
    }
    return true;
}

bool Parser::ProbeType(bool *ok) {
    ProbeAtomType(CHECK_OK);
    if (Peek().Is(Token::kLBrack)) {
        while (Probe(Token::kLBrack)) {
            Probe(Token::kRBrack, CHECK_OK);
        }
    }
    return false;
}

bool Parser::ProbeAtomType(bool *ok) {
    switch (Peek().kind()) {
        case Token::kUnit:
        case Token::kBool:
        case Token::kChar:
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
        case Token::k2Colon:
        case Token::kIdentifier: {
            bool prefix_id = false;
            if (!Probe(Token::k2Colon)) {
                prefix_id = Probe(Token::kIdentifier);
            }
            if (prefix_id) {
                if (Probe(Token::kDot) || Probe(Token::k2Colon)) {
                    Probe(Token::kIdentifier, CHECK_OK);
                }
            } else {
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
        auto param = new (arena_) VariableDeclaration::Item(arena_, nullptr, name, type, param_location);
        prototype->mutable_params()->push_back(param);
    }
    
    Match(Token::kRArrow, CHECK_OK);
    auto stmt = ParseStatement(CHECK_OK);
    return new (arena_) LambdaLiteral(arena_, prototype, stmt, location.Concat(stmt->source_position()));
}

Statement *Parser::ParseInitializerIfExistsWithCondition(Expression **condition, bool *ok) {
    auto initializer_or_condition = ParseStatement(CHECK_OK);
    if (initializer_or_condition->IsAssignment() ||
        initializer_or_condition->IsVariableDeclaration()) {
        // Must be initializer
        Match(Token::kSemi, CHECK_OK);
        *condition = ParseExpression(CHECK_OK);
    } else {
        if (!initializer_or_condition->IsExplicitExpression()) {
            *ok = false;
            error_feedback_->Printf(initializer_or_condition->source_position(), "Destination of when expression"
                                    " must be a expression");
            return nullptr;
        }
        *condition = static_cast<Expression *>(initializer_or_condition);
        initializer_or_condition = nullptr;
    }
    return initializer_or_condition;
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
    auto location = Peek().source_position();
    auto type = ParseAtomType(CHECK_OK);
    
    if (Peek().Is(Token::kLBrack)) {
        //auto ar = new (arena_) ArrayType(arena_, type, 0, location);
        std::vector<base::ArenaVector<Expression *>> stack;
        base::ArenaVector<Expression *> capacities(arena_);
        while (Test(Token::kLBrack)) {
            if (Peek().Is(Token::kRBrack)) {
                MoveNext();
                capacities.push_back(nullptr);
                stack.push_back(std::move(capacities));
                continue;
            }

            while (Test(Token::kComma)) {
                capacities.push_back(nullptr);
            }
            if (Test(Token::kRBrack)) {
                capacities.push_back(nullptr);
                stack.push_back(std::move(capacities));
            }
        }
    
        // type = ar;
        if (stack.empty()) {
            error_feedback_->Printf(location, "Invalid array type.");
            *ok = false;
            return nullptr;
        }
        
        ArrayType *ar = reinterpret_cast<ArrayType *>(type);
        for (ssize_t i = stack.size() - 1; i >= 1; i--) {
            ar = new (arena_) ArrayType(arena_, ar, std::move(stack[i]), location);
        }
        type = new (arena_) ArrayType(arena_, ar, std::move(stack[0]), location);
    }
    return type;
}

ArrayType *Parser::ParseArrayTypeMaybeWithLimits(bool *ok) {
    return ParseArrayTypeMaybeWithLimits(nullptr, nullptr, ok);
}

ArrayType *Parser::ParseArrayTypeMaybeWithLimits(const Identifier *ns, const String *id, bool *ok) {
    auto location = Peek().source_position();
    
    Type *type = nullptr;
    if (ns && id) {
        auto symbol = new (arena_) Symbol(ns->name(), id, ns->source_position().Concat(location));
        type = new (arena_) Type(arena_, symbol, location);
        if (Test(Token::kLess)) {
            do {
                auto arg = ParseType(CHECK_OK);
                type->mutable_generic_args()->push_back(arg);
            } while (Test(Token::kComma));
            Match(Token::kGreater, CHECK_OK);
        }
    } else {
        type = ParseAtomType(CHECK_OK);
    }
    
    //int[,][][,,]?
    //val ar = some(int[,][][,,])
    
    //auto ar = new (arena_) ArrayType(arena_, type, 0, location);
    //ArrayType *ar = reinterpret_cast<ArrayType *>(type);
    // [][]...
    // [expr,expr,expr,...]
    std::vector<base::ArenaVector<Expression *>> stack;
    base::ArenaVector<Expression *> capacities(arena_);
    while (Test(Token::kLBrack)) {
        if (Peek().Is(Token::kRBrack)) {
            MoveNext();
            capacities.push_back(nullptr);
            //ar = new (arena_) ArrayType(arena_, ar, std::move(capacities), location);
            stack.push_back(std::move(capacities));
            continue;
        }
        
        if (Peek().Is(Token::kComma)) {
            while (Test(Token::kComma)) {
                capacities.push_back(nullptr);
            }
            if (Test(Token::kRBrack)) {
                capacities.push_back(nullptr);
                //ar = new (arena_) ArrayType(arena_, ar, std::move(capacities), location);
                stack.push_back(std::move(capacities));
            }
            continue;
        }

        auto expr = ParseExpression(CHECK_OK);
        capacities.push_back(expr);
        if (Test(Token::kRBrack)) {
            stack.push_back(std::move(capacities));
            continue;
        }
        while (Test(Token::kComma)) {
            expr = ParseExpression(CHECK_OK);
            capacities.push_back(expr);
        }
        stack.push_back(std::move(capacities));
        Match(Token::kRBrack, CHECK_OK);
    }
    if (stack.empty()) {
        error_feedback_->Printf(location, "Invalid array initializer.");
        *ok = false;
        return nullptr;
    }
    
    ArrayType *ar = reinterpret_cast<ArrayType *>(type);
    for (ssize_t i = stack.size() - 1; i >= 1; i--) {
        ar = new (arena_) ArrayType(arena_, ar, std::move(stack[i]), location);
    }
    ar = new (arena_) ArrayType(arena_, ar, std::move(stack[0]), location);
    
    if (!ar->HasCapacities() != ar->HasNotCapacities() || !ar->HasNotCapacities() != ar->HasCapacities()) {
        error_feedback_->Printf(ar->source_position(), "Bad array type: `%s'", ar->ToString().c_str());
        *ok = false;
        return nullptr;
    }
    return ar;
}

Type *Parser::ParseAtomType(bool *ok) {
    auto location = Peek().source_position();
    Type *type = nullptr;
    switch (Peek().kind()) {
        case Token::kUnit:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_unit, location);
            break;
        case Token::kBool:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_bool, location);
            break;
        case Token::kChar:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_char, location);
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
        case Token::kAny:
            MoveNext();
            type = new (arena_) Type(arena_, Type::kType_any, location);
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
            do {
                auto ret = ParseType(CHECK_OK);
                fun->mutable_return_types()->push_back(ret);
            } while (Test(Token::kComma));
            type = fun;
        } break;
        default:
            *ok = false;
            error_feedback_->Printf(location, "Unexpected 'type_ref', expected: %s", Peek().ToString().c_str());
            return nullptr;
    }
    return type;
}

Expression *Parser::ParseStaticLiteral(bool *ok) {
    Expression *literal = nullptr;
    auto location = Peek().source_position();
    switch (Peek().kind()) {
        case Token::kI8Val:
            literal = new (arena_) I8Literal(arena_, Peek().i8_val(), location);
            MoveNext();
            break;
        case Token::kU8Val:
            literal = new (arena_) U8Literal(arena_, Peek().u8_val(), location);
            MoveNext();
            break;
        case Token::kI16Val:
            literal = new (arena_) I16Literal(arena_, Peek().i16_val(), location);
            MoveNext();
            break;
        case Token::kU16Val:
            literal = new (arena_) U16Literal(arena_, Peek().u16_val(), location);
            MoveNext();
            break;
        case Token::kI32Val:
            literal = new (arena_) I32Literal(arena_, Peek().i32_val(), location);
            MoveNext();
            break;
        case Token::kU32Val:
            literal = new (arena_) U32Literal(arena_, Peek().u32_val(), location);
            MoveNext();
            break;
        case Token::kIntVal:
            literal = new (arena_) IntLiteral(arena_, Peek().i32_val(), location);
            MoveNext();
            break;
        case Token::kUIntVal:
            literal = new (arena_) UIntLiteral(arena_, Peek().u32_val(), location);
            MoveNext();
            break;
        case Token::kI64Val:
            literal = new (arena_) I64Literal(arena_, Peek().i64_val(), location);
            MoveNext();
            break;
        case Token::kU64Val:
            literal = new (arena_) U64Literal(arena_, Peek().u64_val(), location);
            MoveNext();
            break;
        case Token::kF32Val:
            literal = new (arena_) F32Literal(arena_, Peek().f32_val(), location);
            MoveNext();
            break;
        case Token::kF64Val:
            literal = new (arena_) F64Literal(arena_, Peek().f64_val(), location);
            MoveNext();
            break;
        case Token::kStringLine:
        case Token::kStringBlock:
            literal = new (arena_) StringLiteral(arena_, Peek().text_val(), location);
            MoveNext();
            break;
        case Token::kTrue:
            literal = new (arena_) BoolLiteral(arena_, true, location);
            MoveNext();
            break;
        case Token::kFalse:
            literal = new (arena_) BoolLiteral(arena_, false, location);
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

Identifier *Parser::ParseIdentifier(bool *ok) {
    auto location = Peek().source_position();
    auto name = MatchText(Token::kIdentifier, CHECK_OK);
    return new (arena_) Identifier(name, location);
}

// symbol ::= identifier | identifier `.' identifier | identifier `::' identifier
Symbol *Parser::ParseSymbol(bool *ok) {
    auto location = Peek().source_position();
    if (Peek().Is(Token::k2Colon)) {
        MoveNext();
    }
    auto prefix_or_name = MatchText(Token::kIdentifier, CHECK_OK);
    if (Test(Token::kDot) || Test(Token::k2Colon)) {
        auto name = MatchText(Token::kIdentifier, CHECK_OK);
        return new (arena_) Symbol(prefix_or_name, name, ConcatNow(location));
    }
    return new (arena_) Symbol(prefix_or_name, ConcatNow(location));
}

const String *Parser::ParseAliasOrNull(bool *ok) {
    if (!Test(Token::kAs)) {
        return nullptr;
    }
    switch (Peek().kind()) {
        case Token::kIdentifier:
            return MatchText(Token::kIdentifier, CHECK_OK);
        case Token::kStar: {
            MoveNext();
            //return String::New(arena_, "*", 1);
            return kStarName;
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

const String *Parser::MakeFullName(const String *pkg, const String *name) {
    return String::New(arena_, base::Sprintf("%s.%s", pkg->data(), name->data()));
}

Symbol *Parser::EnsureToSymbol(Expression *expr, bool *ok) {
    if (expr->IsIdentifier()) {
        return new (arena_) Symbol(expr->AsIdentifier()->name(), expr->source_position());
    }
    
    if (expr->IsDot()) {
        auto dot = expr->AsDot();
        if (dot->primary()->IsIdentifier()) {
            auto prefix_name = dot->primary()->AsIdentifier()->name();
            return new (arena_) Symbol(prefix_name, dot->field(), expr->source_position());
        }
    }
    
    *ok = false;
    error_feedback_->Printf(expr->source_position(), "Unexpected 'symbol'");
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
