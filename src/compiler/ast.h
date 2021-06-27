#pragma once
#ifndef YALX_COMPILER_AST_H_
#define YALX_COMPILER_AST_H_

#include "compiler/node.h"
#include "base/checking.h"
#include "base/arena-utils.h"

namespace yalx {

namespace cpl {

class AstVisitor;

class AstNode : public Node {
public:
    AstNode(Node::Kind kind, SourcePosition source_position): Node(kind, source_position) {}
    
    virtual void Accept(AstVisitor *visitor) = 0;
}; // class AstNode


class AstVisitor {
public:
#define DEFINE_METHOD(name) virtual void Visit##name(name *node) = 0;
    DECLARE_AST_NODES(DEFINE_METHOD)
#undef DEFINE_METHOD
}; // class AstVisitor


#define DECLARE_AST_NODE(name) \
    void Accept(AstVisitor *visitor) override { return visitor->Visit##name(this); }

class Statement;
class Declaration;
class Expression;

//----------------------------------------------------------------------------------------------------------------------
// FileUnit
//----------------------------------------------------------------------------------------------------------------------
class FileUnit : public AstNode {
public:
    class ImportEntry : public AstNode {
    public:
        ImportEntry(const String *original_package_name, const String *package_path, const String *alias,
                    SourcePosition source_position)
            : AstNode(Node::kMaxKinds, source_position)
            , original_package_name_(original_package_name)
            , package_path_(DCHECK_NOTNULL(package_path))
            , alias_(alias) {}
        
        void Accept(AstVisitor *visitor) override {}
        
        DEF_PTR_PROP_RW(const String, original_package_name);
        DEF_PTR_PROP_RW(const String, package_path);
        DEF_PTR_PROP_RW(const String, alias);
    private:
        const String *original_package_name_;
        const String *package_path_;
        const String *alias_;
    }; // class ImportEntry
    
    
    FileUnit(base::Arena *arena, String *file_name, String *file_full_path, const SourcePosition &source_position)
        : AstNode(Node::kFileUnit, source_position)
        , file_name_(DCHECK_NOTNULL(file_name))
        , file_full_path_(DCHECK_NOTNULL(file_full_path))
        , imports_(arena) {}
    
    DEF_PTR_PROP_RW(const String, file_name);
    DEF_PTR_PROP_RW(const String, file_full_path);
    DEF_PTR_PROP_RW(const String, package_name);
    DEF_ARENA_VECTOR_GETTER(ImportEntry *, import);
    
    DECLARE_AST_NODE(FileUnit);
private:
    const String *file_name_;
    const String *file_full_path_;
    const String *package_name_ = nullptr;
    base::ArenaVector<ImportEntry *> imports_;
}; // class FileUnit


//----------------------------------------------------------------------------------------------------------------------
// Statement
//----------------------------------------------------------------------------------------------------------------------
class Statement : public AstNode {
protected:
    Statement(Kind kind, const SourcePosition &source_position): AstNode(kind, source_position) {}
}; // class Statement


//----------------------------------------------------------------------------------------------------------------------
// Declaration
//----------------------------------------------------------------------------------------------------------------------
class Declaration : public Statement {
public:
    enum Access {
        kExport, kPublic, kProtected, kPrivate, kDefault
    };
    
    DEF_VAL_PROP_RW(Access, access);
    DEF_PTR_PROP_RW(AnnotationDeclaration, annotations);

    virtual Identifier *Identifier() const = 0;
    virtual Type *Type() const = 0;
    virtual Declaration *AtItem(size_t i) const = 0;
    virtual size_t ItemSize() const = 0;
    
protected:
    Declaration(base::Arena *arena, Kind kind, const SourcePosition &source_position)
        : Statement(kind, source_position) {}

private:
    AnnotationDeclaration *annotations_ = nullptr;
    Access access_ = kDefault;
}; // class Declaration


//----------------------------------------------------------------------------------------------------------------------
// VariableDeclaration
//----------------------------------------------------------------------------------------------------------------------
class VariableDeclaration : public Declaration {
public:
    class Item : public Declaration {
    public:
        Item(base::Arena *arena, class Identifier *identifier, class Type *type, const SourcePosition &source_position)
            : Declaration(arena, Node::kMaxKinds, source_position)
            , identifier_(identifier)
            , type_(type) {}
        
        DEF_PTR_PROP_RW(class Identifier, identifier);
        DEF_PTR_PROP_RW(class Type, type);
        
        class Identifier *Identifier() const override { return identifier(); }
        class Type *Type() const override { return type(); }
        Declaration *AtItem(size_t i) const override { return nullptr; }
        size_t ItemSize() const override { return 0; }
        
        void Accept(AstVisitor *v) override {}
    private:
        class Identifier *identifier_;
        class Type *type_;
    }; // class Item
    
    enum Constraint { kVal, kVar };
    
    VariableDeclaration(base::Arena *arena, bool is_volatile, Constraint constraint,
                        const SourcePosition &source_position)
        : Declaration(arena, Node::kVariableDeclaration, source_position)
        , constraint_(constraint)
        , is_volatile_(is_volatile)
        , variables_(arena)
        , initilaizers_(arena) {}

    DEF_VAL_PROP_RW(Constraint, constraint);
    DEF_VAL_PROP_RW(bool, is_volatile);
    DEF_ARENA_VECTOR_GETTER(Item *, variable);
    DEF_ARENA_VECTOR_GETTER(Expression *, initilaizer);
    
    class Identifier *Identifier() const override { return variable(0)->identifier(); }
    class Type *Type() const override { return variable(0)->type(); }
    Declaration *AtItem(size_t i) const override { return variable(i); }
    size_t ItemSize() const override { return variables_size(); }
    
    DECLARE_AST_NODE(VariableDeclaration);
private:
    Constraint constraint_;
    bool is_volatile_ = false;
    base::ArenaVector<Item *> variables_;
    base::ArenaVector<Expression *> initilaizers_;
}; // class VariableDeclaration

//----------------------------------------------------------------------------------------------------------------------
// Annotation
//----------------------------------------------------------------------------------------------------------------------
class AnnotationDeclaration : public AstNode {
public:
    AnnotationDeclaration(base::Arena *arena, const SourcePosition &source_position)
        : AstNode(Node::kAnnotationDeclaration, source_position)
        , annotations_(arena) {}

    DEF_ARENA_VECTOR_GETTER(Annotation *, annotation);
    DECLARE_AST_NODE(AnnotationDeclaration);
private:
    base::ArenaVector<Annotation *> annotations_;
}; // class AnnotationDeclaration

class Annotation : public AstNode {
public:
    class Field : public Node {
    public:
        Field(const String *name, Expression *value, const SourcePosition &source_position)
            : Node(Node::kMaxKinds, source_position)
            , name_(name)
            , value_or_nested_(true)
            , value_(DCHECK_NOTNULL(value)) {}
        
        Field(const String *name, Annotation *nested, const SourcePosition &source_position)
            : Node(Node::kMaxKinds, source_position)
            , name_(name)
            , value_or_nested_(false)
            , nested_(DCHECK_NOTNULL(nested)) {}
        
        bool IsValue() const { return value_or_nested_; }
        bool IsNested() const { return !IsValue(); }
        
        Expression *value() const {
            assert(IsValue());
            return value_;
        }
        
        void set_value(Expression *value) {
            assert(IsValue());
            value_ = DCHECK_NOTNULL(value);
        }
        
        Annotation *nested() const {
            assert(IsNested());
            return nested_;
        }
        
        DEF_PTR_GETTER(const String, name);
    private:
        const String *name_;
        union {
            Expression *value_;
            Annotation *nested_;
        };
        const bool value_or_nested_;
    }; // class Field
    
    Annotation(base::Arena *arena, Symbol *name, const SourcePosition &source_position)
        : AstNode(Node::kAnnotation, source_position)
        , name_(DCHECK_NOTNULL(name))
        , fields_(arena) {}

    DEF_PTR_PROP_RW(Symbol, name);
    DEF_ARENA_VECTOR_GETTER(Field *, field);
    
    DECLARE_AST_NODE(Annotation);
private:
    Symbol *name_;
    base::ArenaVector<Field *> fields_;
}; // class Annotation

//----------------------------------------------------------------------------------------------------------------------
// Expressions
//----------------------------------------------------------------------------------------------------------------------
class Expression : public Statement {
public:
    DEF_VAL_GETTER(bool, is_lval);
    DEF_VAL_GETTER(bool, is_rval);
    
    bool is_only_lval() const { return is_lval() && !is_rval(); }
    bool is_only_rval() const { return !is_lval() && is_rval(); }
    
protected:
    Expression(Kind kind, bool is_lval, bool is_rval, const SourcePosition &source_position)
        : Statement(kind, source_position)
        , is_lval_(is_lval)
        , is_rval_(is_rval) {}
    
    bool is_lval_;
    bool is_rval_;
}; //class Expression


class Literal : public Expression {
public:
    DEF_PTR_PROP_RW(Type, type);
    
protected:
    Literal(Kind kind, Type *type, const SourcePosition &source_position)
        : Expression(kind, false/*is_lval*/, true/*is_rval*/, source_position)
        , type_(type) {
        assert(is_only_rval());
    }
    
    Type *type_;
}; // class Literal


class UnitLiteral : public Literal {
public:
    UnitLiteral(const SourcePosition &source_position): Literal(Node::kUnitLiteral, nullptr, source_position) {}
    DECLARE_AST_NODE(UnitLiteral);
}; // class UnitLiteral

class EmptyLiteral : public Literal {
public:
    EmptyLiteral(const SourcePosition &source_position): Literal(Node::kEmptyLiteral, nullptr, source_position) {}
    DECLARE_AST_NODE(EmptyLiteral);
}; // class EmptyLiteral


template<class T>
struct LiteralTraits {
    static constexpr Node::Kind kKind = Node::kMaxKinds;
    using NodeType = Node;
    
    static void Accept(NodeType *node, AstVisitor *visitor) {}
}; // struct LiteralTraits

template<> struct LiteralTraits<int> {
    static constexpr Node::Kind kKind = Node::kIntLiteral;
    using NodeType = IntLiteral;
    
    static void Accept(NodeType *node, AstVisitor *visitor) { visitor->VisitIntLiteral(node); }
}; // struct LiteralTraits<int>

template<> struct LiteralTraits<unsigned> {
    static constexpr Node::Kind kKind = Node::kUIntLiteral;
    using NodeType = UIntLiteral;
    
    static void Accept(NodeType *node, AstVisitor *visitor) { visitor->VisitUIntLiteral(node); }
}; // struct LiteralTraits<unsigned>

template<> struct LiteralTraits<int64_t> {
    static constexpr Node::Kind kKind = Node::kI64Literal;
    using NodeType = I64Literal;
    
    static void Accept(NodeType *node, AstVisitor *visitor) { visitor->VisitI64Literal(node); }
}; // struct LiteralTraits<int64_t>

template<> struct LiteralTraits<uint64_t> {
    static constexpr Node::Kind kKind = Node::kU64Literal;
    using NodeType = U64Literal;
    
    static void Accept(NodeType *node, AstVisitor *visitor) { visitor->VisitU64Literal(node); }
}; // struct LiteralTraits<uint64_t>

template<> struct LiteralTraits<float> {
    static constexpr Node::Kind kKind = Node::kF32Literal;
    using NodeType = F32Literal;
    
    static void Accept(NodeType *node, AstVisitor *visitor) { visitor->VisitF32Literal(node); }
}; // struct LiteralTraits<float>

template<> struct LiteralTraits<double> {
    static constexpr Node::Kind kKind = Node::kF64Literal;
    using NodeType = F64Literal;
    
    static void Accept(NodeType *node, AstVisitor *visitor) { visitor->VisitF64Literal(node); }
}; // struct LiteralTraits<double>

template<> struct LiteralTraits<bool> {
    static constexpr Node::Kind kKind = Node::kBoolLiteral;
    using NodeType = BoolLiteral;
    
    static void Accept(NodeType *node, AstVisitor *visitor) { visitor->VisitBoolLiteral(node); }
}; // struct LiteralTraits<bool>

template<> struct LiteralTraits<const String *> {
    static constexpr Node::Kind kKind = Node::kStringLiteral;
    using NodeType = StringLiteral;
    
    static void Accept(NodeType *node, AstVisitor *visitor) { visitor->VisitStringLiteral(node); }
}; // struct LiteralTraits<bool>

template<class T>
class ActualLiteral : public Literal {
public:
    DEF_VAL_PROP_RW(T, value);
    
    void Accept(AstVisitor *visitor) override {
        LiteralTraits<T>::Accept(static_cast<typename LiteralTraits<T>::NodeType *>(this), visitor);
    }
protected:
    inline ActualLiteral(T value, const SourcePosition &source_position)
        : Literal(LiteralTraits<T>::kKind, nullptr, source_position)
        , value_(value) {}
    
    T value_;
}; // template<class T> class ActualLiteral


#define DEFINE_ACTUAL_LITERAL(name, type) \
    class name##Literal : public ActualLiteral<type> { \
    public: \
        name##Literal(type value, const SourcePosition &source_position): ActualLiteral<type>(value, source_position) {} \
    }

DEFINE_ACTUAL_LITERAL(Int, int);
DEFINE_ACTUAL_LITERAL(UInt, unsigned);
DEFINE_ACTUAL_LITERAL(I64, int64_t);
DEFINE_ACTUAL_LITERAL(U64, uint64_t);
DEFINE_ACTUAL_LITERAL(F32, float);
DEFINE_ACTUAL_LITERAL(F64, double);
DEFINE_ACTUAL_LITERAL(Bool, bool);
DEFINE_ACTUAL_LITERAL(String, const String *);

//----------------------------------------------------------------------------------------------------------------------
// Types
//----------------------------------------------------------------------------------------------------------------------
#define DECLARE_TYPE_CATEGORIES(V) \
    V(Primary,   Type) \
    V(Channel,   ChannelType) \
    V(Array,     ArrayType) \
    V(Class,     ClassType) \
    V(Struct,    StructType) \
    V(Interface, InterfaceType)

#define DECLARE_PRIMARY_TYPE(V) \
    V(any) \
    V(bool) \
    V(char) \
    V(i8) \
    V(u8) \
    V(i16) \
    V(u16) \
    V(i32) \
    V(u32) \
    V(i64) \
    V(u64) \
    V(f32) \
    V(f64) \
    V(string) \
    V(channel) \
    V(array) \
    V(class) \
    V(struct) \
    V(interface) \
    V(symbol)

class Type : public Node {
public:
    enum Category {
#define DEFINE_ENUM(name, node) k##name,
        DECLARE_TYPE_CATEGORIES(DEFINE_ENUM)
#undef  DEFINE_ENUM
        kMaxCategories,
    };
    
    enum Primary {
#define DEFINE_ENUM(name) kType_##name,
        DECLARE_PRIMARY_TYPE(DEFINE_ENUM)
#undef  DEFINE_ENUM
        kMaxTypes,
    };
    
    Type(base::Arena *arena, Primary primary_type, const SourcePosition &source_position)
        : Type(arena, kPrimary, primary_type, nullptr, source_position) {}
    
    Type(base::Arena *arena, const Symbol *identifier, const SourcePosition &source_position)
        : Type(arena, kPrimary, kType_symbol, identifier, source_position) {}
    
    DEF_VAL_GETTER(Category, category);
    DEF_VAL_PROP_RW(Primary, primary_type);
    DEF_PTR_PROP_RW(const Symbol, identifier);
    DEF_ARENA_VECTOR_GETTER(Type *, generic_arg);
    
#define DEFINE_METHOD(name, node) \
    bool Is##name##Type() const { return category_ == k##name; }
    DECLARE_TYPE_CATEGORIES(DEFINE_METHOD)
#undef  DEFINE_METHOD
    
    //DECLARE_AST_NODE(Type);
protected:
    Type(base::Arena *arena, Category category, Primary primary_type, const Symbol *identifier,
         const SourcePosition &source_position)
        : Node(CategoryToKind(category), source_position)
        , category_(category)
        , primary_type_(primary_type)
        , identifier_(identifier)
        , generic_args_(arena) {}
    
    static Kind CategoryToKind(Category category) {
        switch (category) {
    #define DEFINE_CASE(name, node) \
            case k##name: return k##node;
        DECLARE_TYPE_CATEGORIES(DEFINE_CASE)
    #undef DEFINE_CASE
            default:
                UNREACHABLE();
                break;
            return kMaxKinds;
        }
    }
private:
    Category category_;
    Primary primary_type_;
    const Symbol *identifier_;
    base::ArenaVector<Type *> generic_args_;
}; // class Type


class ArrayType : public Type {
public:
    ArrayType(base::Arena *arena, Type *element_type, int dimension_count, const SourcePosition &source_position)
        : Type(arena, Type::kArray, element_type->primary_type(), nullptr, source_position)
        , dimension_count_(dimension_count) {
        assert(dimension_count > 0);
        mutable_generic_args()->push_back(DCHECK_NOTNULL(element_type));
    }
    
    DEF_VAL_GETTER(int, dimension_count);
    Type *element_type() const { return generic_arg(0); }
private:
    int dimension_count_ = 0;
}; // class ArrayType


class ChannelType : public Type {
public:
    ChannelType(base::Arena *arena, Type *element_type, const SourcePosition &source_position)
        : Type(arena, Type::kChannel, element_type->primary_type(), nullptr, source_position) {
        mutable_generic_args()->push_back(DCHECK_NOTNULL(element_type));
    }

    Type *element_type() const { return generic_arg(0); }
}; // class ChannelType

template<class T>
struct UDTTraits {
    static constexpr Type::Category category = Type::kMaxCategories;
    static constexpr Type::Primary type = Type::kMaxTypes;
}; // struct UDTTraits

template<>
struct UDTTraits<ClassDefinition> {
    static constexpr Type::Category category = Type::kClass;
    static constexpr Type::Primary type = Type::kType_class;
}; // struct UDTTraits<ClassDefinition>

template<>
struct UDTTraits<StructDefinition> {
    static constexpr Type::Category category = Type::kStruct;
    static constexpr Type::Primary type = Type::kType_struct;
}; // struct UDTTraits<StructDefinition>

template<>
struct UDTTraits<InterfaceDefinition> {
    static constexpr Type::Category category = Type::kInterface;
    static constexpr Type::Primary type = Type::kType_interface;
}; // struct UDTTraits<InterfaceDefinition>

template<class D>
class UDTType : public Type {
public:
    DEF_PTR_PROP_RW(D, definition);
    
protected:
    UDTType(base::Arena *arena, D *definition, const SourcePosition &source_position)
        : Type(arena, UDTTraits<D>::category, UDTTraits<D>::type, nullptr, source_position)
        , definition_(DCHECK_NOTNULL(definition)) {}
    
    D *definition_;
}; // class UDTType

class ClassType : public UDTType<ClassDefinition> {
public:
    ClassType(base::Arena *arena, ClassDefinition *definition, const SourcePosition &source_position)
        : UDTType<ClassDefinition>(arena, definition, source_position) {}
}; // class ClassType

class StructType : public UDTType<StructDefinition> {
public:
    StructType(base::Arena *arena, StructDefinition *definition, const SourcePosition &source_position)
        : UDTType<StructDefinition>(arena, definition, source_position) {}
}; // class StructType

class InterfaceType : public UDTType<InterfaceDefinition> {
public:
    InterfaceType(base::Arena *arena, InterfaceDefinition *definition, const SourcePosition &source_position)
        : UDTType<InterfaceDefinition>(arena, definition, source_position) {}
}; // class InterfaceType

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_AST_H_
