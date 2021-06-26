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
