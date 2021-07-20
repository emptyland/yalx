#include "compiler/ast.h"

namespace yalx {

namespace cpl {

std::string Symbol::ToString() const {
    if (prefix_name()) {
        return prefix_name()->ToString() + "." + name()->ToString();
    } else {
        return name()->ToString();
    }
}

Package::Package(base::Arena *arena, const String *id, const String *path, const String *full_path, const String *name)
    : AstNode(Node::kPackage, {0,0})
    , id_(DCHECK_NOTNULL(id))
    , name_(DCHECK_NOTNULL(name))
    , path_(DCHECK_NOTNULL(path))
    , full_path_(DCHECK_NOTNULL(full_path))
    , source_files_(arena)
    , references_(arena)
    , dependences_(arena)
    , imports_(arena) {
}

void Package::Prepare() {
    for (auto file : source_files()) {
        for (auto item : file->imports()) {
            imports_[item->package_path()->ToSlice()] = Import{
                .pkg = nullptr,
                .file_unit = file,
                .entry = item,
            };
        }
    }
}

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
    , statements_(arena)
    , funs_(arena)
    , vars_(arena)
    , class_defs_(arena)
    , struct_defs_(arena)
    , interfaces_(arena)
    , objects_(arena)
    , annotations_(arena) {
}

void FileUnit::Add(Statement *stmt) {
    switch (stmt->kind()) {
        case Node::kFunctionDeclaration:
            funs_.push_back(stmt->AsFunctionDeclaration());
            break;
        case Node::kVariableDeclaration:
            vars_.push_back(stmt->AsVariableDeclaration());
            break;
        case Node::kClassDefinition:
            class_defs_.push_back(stmt->AsClassDefinition());
            break;
        case Node::kStructDefinition:
            struct_defs_.push_back(stmt->AsStructDefinition());
            break;
        case Node::kInterfaceDefinition:
            interfaces_.push_back(stmt->AsInterfaceDefinition());
            break;
        case Node::kObjectDeclaration:
            objects_.push_back(stmt->AsObjectDeclaration());
            break;
        case Node::kAnnotationDefinition:
            annotations_.push_back(stmt->AsAnnotationDefinition());
            break;
        default:
            UNREACHABLE();
            break;
    }
    statements_.push_back(stmt);
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

Throw::Throw(Expression *throwing_val, const SourcePosition &source_position)
    : Statement(Node::kThrow, source_position)
    , throwing_val_(throwing_val) {
}

Break::Break(const SourcePosition &source_position)
    : Statement(Node::kBreak, source_position) {
}

Continue::Continue(const SourcePosition &source_position)
    : Statement(Node::kContinue, source_position) {
}

RunCoroutine::RunCoroutine(Calling *entry, const SourcePosition &source_position)
    : Statement(Node::kRunCoroutine, source_position)
    , entry_(entry) {
}

GenericParameter::GenericParameter(const String *name, Type *constraint, const SourcePosition &source_position)
    : Node(Node::kMaxKinds, source_position)
    , name_(DCHECK_NOTNULL(name))
    , constraint_(constraint) {
}

Circulation::Circulation(Node::Kind kind, Block *body, const SourcePosition &source_position)
    : Statement(kind, source_position)
    , body_(body) {
}

bool Circulation::Is(const AstNode *node) {
    switch (node->kind()) {
        case Node::kWhileLoop:
        case Node::kUnlessLoop:
        case Node::kForeachLoop:
            return true;
        default:
            return false;
    }
}

ConditionLoop::ConditionLoop(Node::Kind kind, Block *body, bool execute_first, Statement *initializer,
                             Expression *condition, const SourcePosition &source_position)
    : Circulation(kind, body, source_position)
    , initializer_(initializer)
    , condition_(condition)
    , execute_first_(execute_first) {
}

WhileLoop::WhileLoop(Statement *initializer, bool execute_first, Expression *condition, Block *body,
                     const SourcePosition &source_position)
    : ConditionLoop(Node::kWhileLoop, body, execute_first, initializer, condition, source_position) {}

UnlessLoop::UnlessLoop(Statement *initializer, bool execute_first, Expression *condition, Block *body,
                       const SourcePosition &source_position)
    : ConditionLoop(Node::kUnlessLoop, body, execute_first, initializer, condition, source_position) {}

ForeachLoop::ForeachLoop(Identifier *iterative_destination, Expression *iterable, Block *body,
                         const SourcePosition &source_position)
    : Circulation(Node::kForeachLoop, body, source_position)
    , iteration_(kIterator)
    , iterative_destination_(iterative_destination_)
    , iterable_(iterable) {    
}

ForeachLoop::ForeachLoop(Identifier *iterative_destination, IntRange range, Block *body,
                         const SourcePosition &source_position)
    : Circulation(Node::kForeachLoop, body, source_position)
    , iteration_(range.close ? kCloseBound : kOpenBound)
    , iterative_destination_(iterative_destination)
    , range_(range) {
}

Declaration::Declaration(base::Arena *arena, Kind kind, const SourcePosition &source_position)
    : Statement(kind, source_position) {        
}

bool Declaration::Is(AstNode *node) {
    switch (node->kind()) {
        case Node::kVariableDeclaration:
        case Node::kFunctionDeclaration:
        case Node::kObjectDeclaration:
            return true;
            
        default:
            return false;
    }
}

VariableDeclaration::VariableDeclaration(base::Arena *arena, bool is_volatile, Constraint constraint,
                    const SourcePosition &source_position)
    : Declaration(arena, Node::kVariableDeclaration, source_position)
    , constraint_(constraint)
    , is_volatile_(is_volatile)
    , variables_(arena)
    , initilaizers_(arena) {        
}

VariableDeclaration::VariableDeclaration(base::Arena *arena, bool is_volatile, Constraint constraint,
                                         const String *identifier, class Type *type,
                                         const SourcePosition &source_position)
    : VariableDeclaration(arena, is_volatile, constraint, source_position) {
    variables_.push_back(new (arena) Item(arena, identifier, type, source_position));
}

VariableDeclaration::Item::Item(base::Arena *arena, const String *identifier, class Type *type,
                                const SourcePosition &source_position)
    : Declaration(arena, Node::kMaxKinds, source_position)
    , identifier_(identifier)
    , type_(type) {        
}

FunctionDeclaration::FunctionDeclaration(base::Arena *arena, Decoration decoration, const String *name,
                                         FunctionPrototype *prototype, bool is_reduce,
                                         const SourcePosition &source_position)
    : Declaration(arena, Node::kFunctionDeclaration, source_position)
    , decoration_(decoration)
    , name_(name)
    , prototype_(prototype)
    , generic_params_(arena)
    , is_reduce_(is_reduce) {
}

const String *FunctionDeclaration::Identifier() const { return name(); }
class Type *FunctionDeclaration::Type() const { return prototype(); }

Declaration *FunctionDeclaration::AtItem(size_t i) const { return const_cast<FunctionDeclaration *>(this); }

size_t FunctionDeclaration::ItemSize() const { return 1; }

ObjectDeclaration::ObjectDeclaration(base::Arena *arena, const String *name, const SourcePosition &source_position)
    : Declaration(arena, Node::kObjectDeclaration, source_position)
    , name_(DCHECK_NOTNULL(name))
    , fields_(arena)
    , methods_(arena) {
    auto symbol = new (arena) Symbol(name, source_position);
    dummy_ = new (arena) class Type(arena, symbol, source_position);
}

const String *ObjectDeclaration::Identifier() const { return name(); }
class Type *ObjectDeclaration::Type() const { return dummy_; }
Declaration *ObjectDeclaration::AtItem(size_t i) const { return const_cast<ObjectDeclaration *>(this); }
size_t ObjectDeclaration::ItemSize() const { return 1; }

bool Definition::Is(AstNode *node) {
    switch (node->kind()) {
        case Node::kStructDefinition:
        case Node::kClassDefinition:
        case Node::kInterfaceDefinition:
        case Node::kAnnotationDefinition:
            return true;
            
        default:
            return false;
    }
}

Definition::Definition(base::Arena *arena, Kind kind, const String *name, const SourcePosition &source_position)
    : Statement(kind, source_position)
    , name_(name)
    , generic_params_(arena) {
}

InterfaceDefinition::InterfaceDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position)
    : Definition(arena, Node::kInterfaceDefinition, name, source_position)
    , methods_(arena) {
}

AnnotationDefinition::AnnotationDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position)
    : Definition(arena, Node::kAnnotationDefinition, name, source_position)
    , members_(arena) {
}

IncompletableDefinition::IncompletableDefinition(Node::Kind kind, base::Arena *arena, const String *name,
                                                 const SourcePosition &source_position)
    : Definition(arena, kind, name, source_position)
    , parameters_(arena)
    , named_parameters_(arena)
    , fields_(arena)
    , methods_(arena) {
}

StructDefinition::StructDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position)
    : IncompletableDefinition(Node::kStructDefinition, arena, name, source_position) {
}

ClassDefinition::ClassDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position)
    : IncompletableDefinition(Node::kClassDefinition, arena, name, source_position)
    , concepts_(arena)
    , implements_(arena) {
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

Instantiation::Instantiation(base::Arena *arena, Expression *primary, const SourcePosition &source_position)
    : Expression(Node::kInstantiation, false/*is_lval*/, true/*is_rval*/, source_position)
    , generic_args_(arena)
    , primary_(DCHECK_NOTNULL(primary)) {
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
    : Expression(Node::kDot, true /*is_lval*/, true /*is_rval*/, source_position)
    , primary_(primary)
    , field_(field) {
}

Casting::Casting(Expression *source, Type *destination, const SourcePosition &source_position)
    : Expression(Node::kCasting, false /*is_lval*/, true /*ls_rval*/, source_position) {
}

Testing::Testing(Expression *source, Type *destination, const SourcePosition &source_position)
    : Expression(Node::kTesting, false /*is_lval*/, true /*ls_rval*/, source_position) {
}

Calling::Calling(base::Arena *arena, Expression *callee, const SourcePosition &source_position)
    : Expression(Node::kCalling, false /*is_lval*/, true /*ls_rval*/, source_position)
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

CaseWhenPattern::CaseWhenPattern(Pattern pattern, Statement *then_clause, const SourcePosition &source_position)
    : Node(Node::kMaxKinds, source_position)
    , pattern_(pattern)
    , then_clause_(then_clause) {
}

WhenExpression::WhenExpression(base::Arena *arena, Statement *initializer, Expression *destination,
                               const SourcePosition &source_position)
    : Expression(Node::kWhenExpression, false /*is_lval*/, true /*ls_rval*/, source_position)
    , initializer_(initializer)
    , destination_(destination)
    , case_clauses_(arena) {
}

WhenExpression::ExpectValuesCase::ExpectValuesCase(base::Arena *arena, Statement *then_clause,
                                                   const SourcePosition &source_position)
    : Case(kExpectValues, then_clause, source_position)
    , match_values_(arena) {
}

WhenExpression::TypeTestingCase::TypeTestingCase(Identifier *name, Type *match_type, Statement *then_clause,
                                                 const SourcePosition &source_position)
    : Case(kTypeTesting, then_clause, source_position)
    , name_(DCHECK_NOTNULL(name))
    , match_type_(DCHECK_NOTNULL(match_type)) {
}

WhenExpression::BetweenToCase::BetweenToCase(Expression *lower, Expression *upper, Statement *then_clause, bool is_close,
                                             const SourcePosition &source_position)
    : Case(kBetweenTo, then_clause, source_position)
    , lower_(DCHECK_NOTNULL(lower))
    , upper_(DCHECK_NOTNULL(upper))
    , is_close_(is_close) {
}

WhenExpression::StructMatchingCase::StructMatchingCase(base::Arena *arena, const Symbol *symbol, Statement *then_clause,
                                                       const SourcePosition &source_position)
    : Case(kStructMatching, then_clause, source_position)
    , symbol_(DCHECK_NOTNULL(symbol))
    , expecteds_(arena) {
}

TryCatchExpression::TryCatchExpression(base::Arena *arena, Block *try_block, Block *finally_block,
                                       const SourcePosition &source_position)
    : Expression(Node::kTryCatchExpression, false /*is_lval*/, true /*ls_rval*/, source_position)
    , try_block_(DCHECK_NOTNULL(try_block))
    , finally_block_(finally_block)
    , catch_clauses_(arena) {
}

StringTemplate::StringTemplate(base::Arena *arena, const SourcePosition &source_position)
    : Expression(Node::kStringTemplate, false /*is_lval*/, true /*ls_rval*/, source_position)
    , parts_(arena) {
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

bool Type::Acceptable(const Type *rhs, bool *unlinked) const {
    if (primary_type() == kType_symbol || rhs->primary_type() == kType_symbol) {
        *unlinked = true;
        return false;
    }
    
    switch (primary_type()) {
        case kType_i8:
        case kType_u8:
        case kType_i16:
        case kType_u16:
        case kType_i32:
        case kType_u32:
        case kType_i64:
        case kType_u64:
        case kType_f32:
        case kType_f64:
        case kType_char:
        case kType_bool:
        case kType_unit:
            return primary_type() == rhs->primary_type();
        case kType_any:
            return rhs->primary_type() != kType_unit;
        default:
            UNREACHABLE();
            break;
    }
    return false;
}

Type *Type::Link(Linker &&linker) {
    for (size_t i = 0; i < generic_args_size(); i++) {
        auto old = generic_arg(i);
        auto linked = old->Link(std::move(linker));
        if (!linked) {
            return nullptr;
        }
        if (old != linked) {
            generic_args_[i] = linked;
        }
    }
    if (primary_type() == kType_symbol) {
        auto type = linker(identifier(), this);
        if (!type) {
            return type;
        }
        return type->Link(std::move(linker));
    }
    return this;
}

std::string Type::ToString() const {
    switch (primary_type()) {
        case kType_i8:
            return "i8";
        case kType_u8:
            return "u8";
        case kType_i16:
            return "i16";
        case kType_u16:
            return "u16";
        case kType_i32:
            return "i32";
        case kType_u32:
            return "u32";
        case kType_i64:
            return "i64";
        case kType_u64:
            return "u64";
        case kType_f32:
            return "f32";
        case kType_f64:
            return "f64";
        case kType_char:
            return "char";
        case kType_bool:
            return "bool";
        case kType_unit:
            return "unit";
        case kType_any:
            return "any";
        default:
            break;
    }
    UNREACHABLE();
}

bool ArrayType::Acceptable(const Type *rhs, bool *unlinked) const {
    if (!rhs->IsArrayType() || dimension_count() != rhs->AsArrayType()->dimension_count()) {
        return false;
    }
    auto type = rhs->AsArrayType();
    if (type->element_type()->primary_type() == kType_symbol) {
        *unlinked = true;
        return false;
    }
    return element_type()->Acceptable(type->element_type(), unlinked);
}

std::string ArrayType::ToString() const {
    std::string dim;
    assert(dimension_count() > 0);
    for (int i = 0; i <dimension_count(); i++) {
        dim.append("[");
    }
    for (int i = 0; i <dimension_count(); i++) {
        dim.append("]");
    }
    return element_type()->ToString() + dim;
}

bool ChannelType::Acceptable(const Type *rhs, bool *unlinked) const {
    if (rhs->primary_type() == kType_symbol) {
        *unlinked = true;
        return false;
    }
    return rhs->category() == kChannel && element_type()->Acceptable(rhs->AsChannelType()->element_type(), unlinked);
}

std::string ChannelType::ToString() const {
    return "chan<" + element_type()->ToString() + ">";
}

bool ClassType::Acceptable(const Type *rhs, bool *unlinked) const {
    if (rhs->primary_type() == kType_symbol) {
        *unlinked = true;
        return false;
    }
    if (!rhs->IsClassType()) {
        return false;
    }
    auto type = rhs->AsClassType();
    if (definition() == type->definition()) {
        return true;
    }
    for (auto base_of = definition()->base_of(); base_of != nullptr; base_of = base_of->base_of()) {
        if (base_of == type->definition()) {
            return true;
        }
    }
    return false;
}

std::string ClassType::ToString() const { return definition()->full_name()->ToString(); }

bool StructType::Acceptable(const Type *rhs, bool *unlinked) const {
    if (rhs->primary_type() == kType_symbol) {
        *unlinked = true;
        return false;
    }
    if (!rhs->IsStructType()) {
        return false;
    }
    auto type = rhs->AsStructType();
    if (definition() == type->definition()) {
        return true;
    }
    for (auto base_of = definition()->base_of(); base_of != nullptr; base_of = base_of->base_of()) {
        if (base_of == type->definition()) {
            return true;
        }
    }
    return false;
}

std::string StructType::ToString() const { return definition()->full_name()->ToString(); }

bool InterfaceType::Acceptable(const Type *rhs, bool *unlinked) const {
    if (rhs->primary_type() == kType_symbol) {
        *unlinked = true;
        return false;
    }
    return rhs->category() == kInterface && definition() == rhs->AsInterfaceType()->definition();
}

std::string InterfaceType::ToString() const { return definition()->full_name()->ToString(); }

bool FunctionPrototype::Acceptable(const Type *rhs, bool *unlinked) const {
    if (rhs->primary_type() == kType_symbol) {
        *unlinked = true;
        return false;
    }
    if (!rhs->IsFunctionPrototype()) {
        return false;
    }
    auto prototype = DCHECK_NOTNULL(rhs->AsFunctionPrototype());
    if (vargs() != prototype->vargs() || params_size() != prototype->params_size() ||
        return_types_size() != prototype->return_types_size()) {
        return false;
    }
    UNREACHABLE();
    return false;
}

Type *FunctionPrototype::Link(Linker &&linker) {
    for (size_t i = 0; i < params_size(); i++) {
        auto old = DCHECK_NOTNULL(param(i)->AsType());
        auto linked = old->Link(std::move(linker));
        if (!linked) {
            return nullptr;
        }
        if (linked != old) {
            params_[i] = linked;
        }
    }
    for (size_t i = 0; i < return_types_size(); i++) {
        auto old = DCHECK_NOTNULL(return_type(i));
        auto linked = old->Link(std::move(linker));
        if (!linked) {
            return nullptr;
        }
        if (linked != old) {
            return_types_[i] = linked;
        }
    }
    
    return this;
}

std::string FunctionPrototype::ToString() const {
    std::string buf("(");
    for (size_t i = 0; i < params_size(); i++) {
        if (i > 0) {
            buf.append(",");
        }
        assert(Type::Is(param(i)));
        buf.append(static_cast<Type *>(param(i))->ToString());
    }
    if (vargs()) {
        if (buf.size() > 1) {
            buf.append(",");
        }
        buf.append("...");
    }
    buf.append("):[");
    for (size_t i = 0; i < return_types_size(); i++) {
        if (i > 0) {
            buf.append(",");
        }
        buf.append(return_type(i)->ToString());
    }
    return buf.append("]");
}

} // namespace cpl

} // namespace yalx
