#include "compiler/ast.h"
#include "compiler/constants.h"

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
    , annotations_(arena)
    , arena_(arena) {
}

void FileUnit::Add(Statement *stmt) {
    if (Definition::Is(stmt)) {
        static_cast<Definition *>(stmt)->set_owns(this);
    } else if (Declaration::Is(stmt)) {
        static_cast<Declaration *>(stmt)->set_owns(this);
    }
    
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

std::tuple<AstNode *, Statement *> Statement::Owns(bool force) {
    if (Declaration::Is(this)) {
        return std::make_tuple(static_cast<const Declaration *>(this)->owns(), this);
    } else if (Definition::Is(this)) {
        return std::make_tuple(static_cast<const Definition *>(this)->owns(), this);
    } else if (force) {
        auto item = down_cast<VariableDeclaration::Item>(this);
        if (!item->owns()) {
            return std::make_tuple(nullptr, nullptr);
        }
        return std::make_tuple(item->owns()->owns(), item->owns());
    } else {
        return std::make_tuple(nullptr, nullptr);
    }
}

Package *Statement::Pack(bool force) {
    if (Declaration::Is(this)) {
        return static_cast<const Declaration *>(this)->package();
    } else if (Definition::Is(this)) {
        return static_cast<const Definition *>(this)->package();
    } else if (force) {
        auto item = down_cast<VariableDeclaration::Item>(this);
        if (!item->owns()) {
            return nullptr;
        }
        return item->owns()->package();
    } else {
        return nullptr;
    }
}

bool Statement::IsNotTemplate() const {
    switch (kind()) {
        case Node::kClassDefinition:
            return AsClassDefinition()->generic_params().empty();
        case Node::kStructDefinition:
            return AsStructDefinition()->generic_params().empty();
        case Node::kFunctionDeclaration:
            return AsFunctionDeclaration()->generic_params().empty();
        default:
            return true;
    }
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

std::string Declaration::FullName() const {
    switch (owns()->kind()) {
        case Node::kFileUnit:
            return owns()->AsFileUnit()->package_name()->ToString() + "." + Identifier()->ToString();
        case Node::kClassDefinition:
        case Node::kInterfaceDefinition:
        case Node::kStructDefinition: {
            auto def = down_cast<Definition>(owns());
            std::string buf(def->PackageName()->ToString());
            return buf.append(".")
                .append(def->name()->ToString())
                .append(".")
                .append(Identifier()->ToString());
        } break;
        case Node::kObjectDeclaration: {
            auto decl = down_cast<Declaration>(owns());
            std::string buf(decl->PackageName()->ToString());
            return buf.append(".")
                .append(decl->Identifier()->ToString())
                .append("$class.")
                .append(Identifier()->ToString());
        } break;
            
        default:
            break;
    }
    UNREACHABLE();
    return nullptr;
}

const String *Declaration::PackageName() const {
    if (owns()->IsFileUnit()) {
        return owns()->AsFileUnit()->package_name();
    }
    if (package()) {
        return package()->name();
    }
    if (Declaration::Is(owns())) {
        static_cast<Declaration *>(owns())->PackageName();
    }
    if (Definition::Is(owns())) {
        static_cast<Definition *>(owns())->PackageName();
    }
    UNREACHABLE();
    return nullptr;
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
    variables_.push_back(new (arena) Item(arena, this, identifier, type, source_position));
}

VariableDeclaration::Item::Item(base::Arena *arena, VariableDeclaration *owns, const String *identifier,
                                class Type *type, const SourcePosition &source_position)
    : Declaration(arena, Node::kMaxKinds, source_position)
    , owns_(owns)
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

std::string Definition::FullName() const {
    switch (owns()->kind()) {
        case Node::kFileUnit:
            return owns()->AsFileUnit()->package_name()->ToString() + "." + name()->ToString();
        case Node::kClassDefinition:
        case Node::kInterfaceDefinition:
        case Node::kStructDefinition: {
            auto def = down_cast<Definition>(owns());
            std::string buf(def->PackageName()->ToString());
            return buf.append(".")
            .append(def->name()->ToString())
            .append(".")
            .append(name()->ToString());
        } break;
        case Node::kObjectDeclaration: {
            auto decl = down_cast<Declaration>(owns());
            std::string buf(decl->PackageName()->ToString());
            return buf.append(".")
            .append(decl->Identifier()->ToString())
            .append("$class.")
            .append(name()->ToString());
        } break;
            
        default:
            break;
    }
    UNREACHABLE();
    return nullptr;
}

const String *Definition::PackageName() const {
    if (owns()->IsFileUnit()) {
        return owns()->AsFileUnit()->package_name();
    }
    if (package()) {
        return package()->name();
    }
    if (Declaration::Is(owns())) {
        static_cast<Declaration *>(owns())->PackageName();
    }
    if (Definition::Is(owns())) {
        static_cast<Definition *>(owns())->PackageName();
    }
    UNREACHABLE();
    return nullptr;
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

Statement *IncompletableDefinition::FindLocalSymbolOrNull(std::string_view name) const {
    for (auto field : fields()) {
        if (name.compare(field.declaration->Identifier()->ToSlice()) == 0) {
            return field.declaration;
        }
    }
    for (auto method : methods()) {
        if (name.compare(method->name()->ToSlice()) == 0) {
            return method;
        }
    }
    return nullptr;
}

StructDefinition::StructDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position)
    : IncompletableDefinition(Node::kStructDefinition, arena, name, source_position) {
}

ClassDefinition::ClassDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position)
    : IncompletableDefinition(Node::kClassDefinition, arena, name, source_position)
    , concepts_(arena) {
}

bool ClassDefinition::IsConceptOf(const InterfaceDefinition *interface) const {
    for (auto owns = this; owns != nullptr; owns = owns->base_of()) {
        for (auto concept : concepts()) {
            if (DCHECK_NOTNULL(concept->AsInterfaceType())->definition() == interface) {
                return true;
            }
        }
    }
    return false;
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

OptionLiteral::OptionLiteral(base::Arena *arena, Expression *value, const SourcePosition &source_position)
    : Literal(kOptionLiteral,
              !value ? new (arena) Type(arena, Type::kType_none, source_position) : nullptr,
              source_position)
    , value_(value) {
}

UnitLiteral::UnitLiteral(base::Arena *arena, const SourcePosition &source_position)
    : Literal(Node::kUnitLiteral, new (arena) Type(arena, Type::kType_unit, source_position), source_position) {
}

EmptyLiteral::EmptyLiteral(const SourcePosition &source_position)
    : Literal(Node::kEmptyLiteral, nullptr, source_position) {
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

ChannelInitializer::ChannelInitializer(Type *type, Expression *capacity, const SourcePosition &source_position)
    : Literal(kChannelInitializer, type, source_position)
    , capacity_(capacity) {
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
    : Expression(Node::kCasting, false /*is_lval*/, true /*ls_rval*/, source_position)
    , source_(source)
    , destination_(destination) {
}

Testing::Testing(Expression *source, Type *destination, const SourcePosition &source_position)
    : Expression(Node::kTesting, false /*is_lval*/, true /*ls_rval*/, source_position)
    , source_(source)
    , destination_(destination) {
}

Calling::Calling(base::Arena *arena, Expression *callee, const SourcePosition &source_position)
    : Expression(Node::kCalling, false /*is_lval*/, true /*ls_rval*/, source_position)
    , callee_(DCHECK_NOTNULL(callee))
    , args_(arena) {
}

//Constructing::Constructing(base::Arena *arena, IncompletableDefinition *mold, const SourcePosition &source_position)
//    : Expression(Node::kConstructing, false /*is_lval*/, true /*ls_rval*/, source_position)
//    , mold_(DCHECK_NOTNULL(mold))
//    , args_(arena) {
//}

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
    , case_clauses_(arena)
    , reduced_types_(arena) {
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
    , catch_clauses_(arena)
    , reduced_types_(arena) {
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

bool Type::IsComparable() const {
    if (IsNumber()) {
        return true;
    }
    Statement *maybe_compare_to_fun = nullptr;
    switch (primary_type()) {
        case kType_char:
        case kType_string:
            return true;
        case kType_class: {
            auto def = AsClassType()->definition();
            maybe_compare_to_fun = def->FindSymbolOrNull(kCompareToFunName);
        } break;
        case kType_struct: {
            auto def = AsStructType()->definition();
            maybe_compare_to_fun = def->FindSymbolOrNull(kCompareToFunName);
        } break;
        case kType_interface: {
            auto def = AsInterfaceType()->definition();
            maybe_compare_to_fun = def->FindSymbolOrNull(kCompareToFunName);
        } break;
        default:
            return false;
    }
    
    if (auto fun = maybe_compare_to_fun->AsFunctionDeclaration()) {
        if ((fun->access() != kPublic && fun->access() != kDefault) ||
            fun->prototype()->return_types_size() != 1 ||
            fun->prototype()->params_size() != 1) {
            return false;
        }
        auto param0 = static_cast<VariableDeclaration::Item *>(fun->prototype()->param(0));
        bool unlinked = false;
        if (!param0->type()->Acceptable(this, &unlinked)) {
            return false;
        }
        assert(!unlinked);
        return true;
    }
    return false;
}

bool Type::IsUnsignedIntegral() const {
    switch (primary_type()) {
        case kType_u8:
        case kType_u16:
        case kType_u32:
        case kType_u64:
            return true;
            
        default:
            return false;
    }
}

bool Type::IsFloating() const {
    switch (primary_type()) {
        case kType_f32:
        case kType_f64:
            return true;
            
        default:
            return false;
    }
}

bool Type::IsSignedIntegral() const {
    switch (primary_type()) {
        case kType_i8:
        case kType_i16:
        case kType_i32:
        case kType_i64:
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
        case kType_string:
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
        case kType_string:
            return "string";
        case kType_unit:
            return "unit";
        case kType_any:
            return "any";
        default:
            break;
    }
    UNREACHABLE();
}

ArrayType::ArrayType(base::Arena *arena, Type *element_type, int dimension_count, const SourcePosition &source_position)
    : Type(arena, Type::kArray, kType_array, nullptr, source_position)
    , dimension_capacitys_(dimension_count, nullptr, arena) {
    assert(dimension_count >= 0);
    mutable_generic_args()->push_back(DCHECK_NOTNULL(element_type));
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

OptionType::OptionType(base::Arena *arena, Type *element_type, const SourcePosition &source_position)
: Type(arena, Type::kOption, kType_option, nullptr, source_position) {
    mutable_generic_args()->push_back(DCHECK_NOTNULL(element_type));
}

bool OptionType::Acceptable(const Type *rhs, bool *unlinked) const {
    if (rhs->IsOptionType()) {
        return element_type()->Acceptable(rhs->AsOptionType()->element_type(), unlinked);
    }
    if (rhs->primary_type() == kType_none) {
        return true;
    }
    return element_type()->Acceptable(rhs, unlinked);
}

Type *OptionType::Link(Linker &&linker) {
    auto linked = element_type()->Link(std::move(linker));
    if (!linked) {
        return nullptr;
    }
    (*mutable_generic_args())[0] = linked;
    return this;
}

std::string OptionType::ToString() const { return element_type()->ToString().append("?"); }

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
    return type->definition()->IsBaseOf(definition());
}

std::string ClassType::ToString() const { return definition()->FullName(); }

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
    return type->definition()->IsBaseOf(definition());
}

std::string StructType::ToString() const { return definition()->FullName(); }

bool InterfaceType::Acceptable(const Type *rhs, bool *unlinked) const {
    if (rhs->primary_type() == kType_symbol) {
        *unlinked = true;
        return false;
    }
    return rhs->category() == kInterface && definition() == rhs->AsInterfaceType()->definition();
}

std::string InterfaceType::ToString() const { return definition()->FullName(); }


std::string FunctionPrototype::MakeSignature() const {
    std::string buf("(");
    for (auto param : params()) {
        if (buf.length() > 1) {
            buf.append(",");
        }
        if (auto ty = param->AsType()) {
            buf.append(ty->ToString());
        } else if (auto var = static_cast<VariableDeclaration::Item *>(param)) {
            buf.append(var->type()->ToString());
        } else {
            UNREACHABLE();
        }
    }
    buf.append("):");
    assert(!return_types().empty());
    if (return_types_size() > 1) {
        buf.append("(");
        for (auto i = 0; i < return_types_size(); i++) {
            if (i > 0) {
                buf.append(",");
            }
            buf.append(return_type(i)->ToString());
        }
        buf.append(")");
    } else {
        buf.append(return_type(0)->ToString());
    }
    return buf;
}

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
        if (param(i)->IsType()) {
            auto old = DCHECK_NOTNULL(param(i)->AsType());
            auto linked = old->Link(std::move(linker));
            if (!linked) {
                return nullptr;
            }
            if (linked != old) {
                params_[i] = linked;
            }
        } else {
            auto item = static_cast<VariableDeclaration::Item *>(param(i));
            auto linked = item->type()->Link(std::move(linker));
            if (!linked) {
                return nullptr;
            }
            item->set_type(linked);
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
