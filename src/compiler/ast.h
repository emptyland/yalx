#pragma once
#ifndef YALX_COMPILER_AST_H_
#define YALX_COMPILER_AST_H_

#include "compiler/node.h"
#include "base/checking.h"
#include "base/arena-utils.h"
#include <functional>

namespace yalx {

namespace cpl {

class AstVisitor;

class AstNode : public Node {
public:
    AstNode(Node::Kind kind, SourcePosition source_position): Node(kind, source_position) {}
    
    virtual int Accept(AstVisitor *visitor) = 0;
}; // class AstNode


class AstVisitor {
public:
#define DEFINE_METHOD(name) virtual int Visit##name(name *node) = 0;
    DECLARE_AST_NODES(DEFINE_METHOD)
#undef DEFINE_METHOD
}; // class AstVisitor


#define DECLARE_AST_NODE(name) \
    int Accept(AstVisitor *visitor) override { return visitor->Visit##name(this); }

class Statement;
class Declaration;
class Expression;


//----------------------------------------------------------------------------------------------------------------------
// Package
//----------------------------------------------------------------------------------------------------------------------
class SymbolDepsNode : public base::ArenaObject {
public:
    SymbolDepsNode(base::Arena *arena, std::string_view name, Statement *ast)
    : name_(name)
    , ast_(ast)
    , backwards_(arena) {}
    
    DEF_VAL_GETTER(std::string_view, name);
    DEF_PTR_GETTER(Statement, ast);
    DEF_ARENA_VECTOR_GETTER(SymbolDepsNode *, backward);
    
    void AddBackward(SymbolDepsNode *node) {
        if (auto iter = std::find(backwards_.begin(), backwards_.end(), node); iter == backwards_.end()) {
            backwards_.push_back(node);
        }
    }
private:
    std::string_view name_;
    Statement *ast_;
    base::ArenaVector<SymbolDepsNode *> backwards_;
}; // class SymbolDepsNode

class Package : public AstNode {
public:
    struct Import {
        Package  *pkg = nullptr;
        FileUnit *file_unit = nullptr;
        AstNode  *entry = nullptr;
    }; // struct Import
    
    using ImportMap = base::ArenaMap<std::string_view, Import>;
    
    
    Package(base::Arena *arena, const String *id, const String *path, const String *full_path, const String *name);
    
    DEF_PTR_PROP_RW(const String, id);
    DEF_PTR_PROP_RW(const String, path);
    DEF_PTR_PROP_RW(const String, full_path);
    DEF_PTR_PROP_RW(const String, name);
    DEF_ARENA_VECTOR_GETTER(FileUnit *, source_file);
    DEF_ARENA_VECTOR_GETTER(Package *, reference);
    DEF_ARENA_VECTOR_GETTER(Package *, dependence);
    DEF_VAL_PROP_RW(ImportMap, imports);
    
    bool IsTerminator() const { return dependences_.empty(); }
    
    Import *import(std::string_view path) {
        auto iter = imports_.find(path);
        return iter == imports_.end() ? nullptr : &iter->second;
    }
    
    SymbolDepsNode *FindOrInsertDeps(base::Arena *arena, std::string_view name, Statement *ast) {
        if (auto iter = symbols_deps_.find(name); iter != symbols_deps_.end()) {
            return iter->second;
        }
        auto node = new (arena) SymbolDepsNode(arena, name, ast);
        symbols_deps_[name] = node;
        return node;
    }
    
    SymbolDepsNode *FindDepsOrNull(std::string_view name) {
        if (auto iter = symbols_deps_.find(name); iter != symbols_deps_.end()) {
            return iter->second;
        }
        return nullptr;
    }
    
    const base::ArenaMap<std::string_view, SymbolDepsNode *> &deps_of_symbols() const { return symbols_deps_; }
    
    DECLARE_AST_NODE(Package);
    friend class Compiler;
private:
    void Prepare();
    
    const String *id_;
    const String *path_;
    const String *full_path_;
    const String *name_;
    //base::Arena *arena_;
    base::ArenaMap<std::string_view, SymbolDepsNode *> symbols_deps_;
    base::ArenaVector<FileUnit *> source_files_;
    base::ArenaVector<Package *> references_;
    base::ArenaVector<Package *> dependences_;
    ImportMap imports_;
}; // class Package

//----------------------------------------------------------------------------------------------------------------------
// FileUnit
//----------------------------------------------------------------------------------------------------------------------
class FileUnit : public AstNode {
public:
    class ImportEntry : public AstNode {
    public:
        ImportEntry(const String *original_package_name, const String *package_path, const String *alias,
                    const SourcePosition &source_position);
        
        int Accept(AstVisitor *visitor) override {}
        
        DEF_PTR_PROP_RW(const String, original_package_name);
        DEF_PTR_PROP_RW(const String, package_path);
        DEF_PTR_PROP_RW(const String, alias);
    private:
        const String *original_package_name_;
        const String *package_path_;
        const String *alias_;
    }; // class ImportEntry
    
    
    FileUnit(base::Arena *arena, String *file_name, String *file_full_path, const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(const String, file_name);
    DEF_PTR_PROP_RW(const String, file_full_path);
    DEF_PTR_PROP_RW(const String, package_name);
    DEF_PTR_GETTER(base::Arena, arena);
    DEF_ARENA_VECTOR_GETTER(ImportEntry *, import);
    DEF_ARENA_VECTOR_GETTER(Statement *, statement);
    DEF_ARENA_VECTOR_GETTER(FunctionDeclaration *, fun);
    DEF_ARENA_VECTOR_GETTER(VariableDeclaration *, var);
    DEF_ARENA_VECTOR_GETTER(ClassDefinition *, class_def);
    DEF_ARENA_VECTOR_GETTER(StructDefinition *, struct_def);
    DEF_ARENA_VECTOR_GETTER(InterfaceDefinition *, interface);
    DEF_ARENA_VECTOR_GETTER(ObjectDeclaration *, object);
    DEF_ARENA_VECTOR_GETTER(AnnotationDefinition *, annotation);
    
    void Add(Statement *stmt);
    
    DECLARE_AST_NODE(FileUnit);
private:
    const String *file_name_;
    const String *file_full_path_;
    const String *package_name_ = nullptr;
    base::ArenaVector<ImportEntry *> imports_;
    base::ArenaVector<FunctionDeclaration *> funs_;
    base::ArenaVector<VariableDeclaration *> vars_;
    base::ArenaVector<ClassDefinition *> class_defs_;
    base::ArenaVector<StructDefinition *> struct_defs_;
    base::ArenaVector<ObjectDeclaration *> objects_;
    base::ArenaVector<InterfaceDefinition *> interfaces_;
    base::ArenaVector<AnnotationDefinition *> annotations_;
    base::ArenaVector<Statement *> statements_;
    base::Arena *arena_;
}; // class FileUnit


//----------------------------------------------------------------------------------------------------------------------
// Statement
//----------------------------------------------------------------------------------------------------------------------
class Statement : public AstNode {
public:
    virtual bool IsExplicitExpression() const { return false; }
    
    std::tuple<AstNode *, Statement *> Owns(bool force);
    Package *Pack(bool force);
    
    bool IsNotTemplate() const;
    bool IsTemplate() const { return !IsNotTemplate(); }
    
protected:
    Statement(Kind kind, const SourcePosition &source_position);
}; // class Statement


class Block : public Statement {
public:
    Block(base::Arena *arena, const SourcePosition &source_position);
    
    DEF_ARENA_VECTOR_GETTER(Statement *, statement);
    DECLARE_AST_NODE(Block);
private:
    base::ArenaVector<Statement *> statements_;
}; // class Block


class List : public Statement {
public:
    List(base::Arena *arena, const SourcePosition &source_position);
    
    DEF_ARENA_VECTOR_GETTER(Expression *, expression);
    DECLARE_AST_NODE(List);
private:
    base::ArenaVector<Expression *> expressions_;
}; // class Block

class Assignment : public Statement {
public:
    Assignment(base::Arena *arena, const SourcePosition &source_position);
    
    DEF_ARENA_VECTOR_GETTER(Expression *, lval);
    DEF_ARENA_VECTOR_GETTER(Expression *, rval);
    DEF_VAL_PROP_RW(bool, initial);
    DECLARE_AST_NODE(Assignment);
private:
    base::ArenaVector<Expression *> lvals_;
    base::ArenaVector<Expression *> rvals_;
    bool initial_ = false;
}; // class Assignment

class Return : public Statement {
public:
    Return(base::Arena *arena, const SourcePosition &source_position);
    
    DEF_ARENA_VECTOR_GETTER(Expression *, returnning_val);
    DECLARE_AST_NODE(Return);
private:
    base::ArenaVector<Expression *> returnning_vals_;
}; // class Return

class Throw : public Statement {
public:
    Throw(Expression *throwing_val, const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(Expression, throwing_val);
    DECLARE_AST_NODE(Throw);
private:
    Expression *throwing_val_;
}; // class Throw

class Break : public Statement {
public:
    Break(const SourcePosition &source_position);
    DECLARE_AST_NODE(Break);
}; // class Break

class Continue : public Statement {
public:
    Continue(const SourcePosition &source_position);
    DECLARE_AST_NODE(Continue);
}; // class Continue

class RunCoroutine : public Statement {
public:
    RunCoroutine(Calling *entry, const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(Calling, entry);
    
    DECLARE_AST_NODE(RunCoroutine);
private:
    Calling *entry_;
}; // class RunCoroutine

// class GenericParameter : Node
//     = Identifier(): Symbol *
//     + identifier: Type *
//     + constraint: Type *
class GenericParameter : public Node {
public:
    GenericParameter(const String *name, Type *constraint, const SourcePosition &source_position);
    
    DEF_PTR_GETTER(const String, name);
    DEF_PTR_PROP_RW(Type, constraint);
    DEF_PTR_PROP_RW(Type, instantiation);
private:
    const String *const name_;
    Type *constraint_;
    Type *instantiation_ = nullptr;
}; // class GenericParameter


class Circulation : public Statement {
public:
    DEF_PTR_PROP_RW(Block, body);

    static bool Is(const AstNode *node);
    static bool IsNot(const AstNode *node) { return !Is(node); }
protected:
    Circulation(Node::Kind kind, Block *body, const SourcePosition &source_position);
    
    Block *body_;
}; // class Circulation


class ConditionLoop : public Circulation {
public:
    DEF_PTR_PROP_RW(Statement, initializer);
    DEF_PTR_PROP_RW(Expression, condition);
    DEF_VAL_GETTER(bool, execute_first); // do { ... } while(cond) or do {...} unless(cond)
    bool test_first() { return !execute_first(); } // while (cond) {...} or unless(cond) {...}

protected:
    ConditionLoop(Node::Kind kind, Block *body, bool execute_first, Statement *initializer, Expression *condition,
                  const SourcePosition &source_position);
    
    Statement *initializer_;
    Expression *condition_;
    const bool execute_first_;
}; // class ConditionLoop


class WhileLoop : public ConditionLoop {
public:
    WhileLoop(Statement *initializer, bool execute_first, Expression *condition, Block *body,
              const SourcePosition &source_position);
    DECLARE_AST_NODE(WhileLoop);
}; // class WhileLoop


class UnlessLoop : public ConditionLoop {
public:
    UnlessLoop(Statement *initializer, bool execute_first, Expression *condition, Block *body,
               const SourcePosition &source_position);
    DECLARE_AST_NODE(UnlessLoop);
}; // class UnlessLoop


class ForeachLoop : public Circulation {
public:
    enum Iteration {
        kIterator,
        kCloseBound,
        kOpenBound,
    };
    
    struct IntRange {
        Expression *lower = nullptr;
        Expression *upper = nullptr;
        bool close = true;
    };
    
    ForeachLoop(Identifier *iterative_destination, Expression *iterable, Block *body,
                const SourcePosition &source_position);
    ForeachLoop(Identifier *iterative_destination, IntRange range, Block *body, const SourcePosition &source_position);
    
    DEF_PTR_GETTER(Identifier, iterative_destination);
    DEF_PTR_PROP_RW(VariableDeclaration, iterative_destination_var);
    DEF_VAL_GETTER(Iteration, iteration);
    Expression *iterable() const { assert(iteration() == kIterator); return iterable_; }
    const IntRange &range() const { assert(iteration() != kIterator); return range_; }
    IntRange *mutable_range() { assert(iteration() != kIterator); return &range_; }
    
    DECLARE_AST_NODE(ForeachLoop);
private:
    Identifier *iterative_destination_;
    union {
        Expression *iterable_;
        IntRange    range_;
    };
    Iteration iteration_;
    VariableDeclaration *iterative_destination_var_ = nullptr;
}; // class ForLoop

//----------------------------------------------------------------------------------------------------------------------
// Declaration
//----------------------------------------------------------------------------------------------------------------------
enum Access {
    kDefault, kExport, kPublic, kProtected, kPrivate
};

class Declaration : public Statement {
public:
    using Access = Access;
    
    DEF_VAL_PROP_RW(Access, access);
    DEF_PTR_PROP_RW(Package, package);
    DEF_PTR_PROP_RW(AstNode, owns);
    DEF_PTR_PROP_RW(Declaration, original);
    DEF_PTR_PROP_RW(AnnotationDeclaration, annotations);

    virtual const String *Identifier() const = 0;
    virtual Type *Type() const = 0;
    virtual Declaration *AtItem(size_t i) const = 0;
    virtual size_t ItemSize() const = 0;
    
    std::string FullName() const;
    const String *PackageName() const;
    
    static bool IsNot(AstNode *node) { return !Is(node); }
    static bool Is(AstNode *node);

protected:
    Declaration(base::Arena *arena, Kind kind, const SourcePosition &source_position);

private:
    Package *package_ = nullptr;
    AstNode *owns_ = nullptr; // <FileUnit|ClassDefinition|StructDefinition>
    Declaration *original_ = nullptr;
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
        Item(base::Arena *arena, VariableDeclaration *owns, const String *identifier, class Type *type,
             const SourcePosition &source_position);
        
        DEF_PTR_GETTER(VariableDeclaration, owns);
        DEF_PTR_PROP_RW(const String, identifier);
        DEF_PTR_PROP_RW(class Type, type);
        class Type **mutable_type() { return &type_; }
        
        const String *Identifier() const override { return identifier(); }
        class Type *Type() const override { return type(); }
        Declaration *AtItem(size_t i) const override { return nullptr; }
        size_t ItemSize() const override { return 0; }
        
        int Accept(AstVisitor *v) override {}
    private:
        VariableDeclaration *const owns_;
        const String *identifier_;
        class Type *type_;
    }; // class Item
    
    enum Constraint { kVal, kVar };
    
    VariableDeclaration(base::Arena *arena, bool is_volatile, Constraint constraint,
                        const SourcePosition &source_position);
    
    VariableDeclaration(base::Arena *arena, bool is_volatile, Constraint constraint, const String *identifier,
                        class Type *type, const SourcePosition &source_position);

    DEF_VAL_PROP_RW(Constraint, constraint);
    DEF_VAL_PROP_RW(bool, is_volatile);
    DEF_ARENA_VECTOR_GETTER(Item *, variable);
    DEF_ARENA_VECTOR_GETTER(Expression *, initilaizer);
    
    const String *Identifier() const override { return variable(0)->identifier(); }
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
// FunctionDeclaration
//----------------------------------------------------------------------------------------------------------------------
//class FunctionDeclaration : Declaration
//    + name: NString *
//    + decoration: {kNative, kAbstract, kOverride}
//    + is_reduce: bool // 'fun foo(): xxx {...}' or 'fun foo() -> xxx' syntax
//    + define_for: Definition *
//    + defined: LambdaLiteral *

class FunctionDeclaration : public Declaration {
public:
    enum Decoration {
        kDefault,
        kNative,
        kAbstract,
        kOverride,
    };
    
    FunctionDeclaration(base::Arena *arena, Decoration decoration, const String *name, FunctionPrototype *prototype,
                        bool is_reduce, const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(const String, name);
    DEF_VAL_GETTER(Decoration, decoration);
    DEF_VAL_GETTER(bool, is_reduce);
    DEF_PTR_PROP_RW(Definition, define_for);
    DEF_PTR_PROP_RW(FunctionPrototype, prototype);
    DEF_PTR_PROP_RW(Statement, body);
    DEF_ARENA_VECTOR_GETTER(GenericParameter *, generic_param);
    
    bool IsDefault() const { return decoration() == kDefault; }
    bool IsNative() const { return decoration() == kNative; }
    bool IsAbstract() const { return decoration() == kAbstract; }
    bool IsOverride() const { return decoration() == kOverride; }
    
    const String *Identifier() const override;
    class Type *Type() const override;
    Declaration *AtItem(size_t i) const override;
    size_t ItemSize() const override;
    
    DECLARE_AST_NODE(FunctionDeclaration);
private:
    const String *name_;
    Decoration decoration_;
    FunctionPrototype *prototype_;
    Statement *body_ = nullptr;
    Definition *define_for_ = nullptr;
    base::ArenaVector<GenericParameter *> generic_params_;
    bool is_reduce_;
}; // class FunctionDeclaration

class ObjectDeclaration : public Declaration {
public:
    ObjectDeclaration(base::Arena *arena, const String *name, const SourcePosition &source_position);

    DEF_PTR_PROP_RW(const String, name);
    DEF_PTR_PROP_RW(class Type, dummy);
    //DEF_PTR_PROP_RW(ClassDefinition, shadow_class);
    DEF_ARENA_VECTOR_GETTER(VariableDeclaration *, field);
    DEF_ARENA_VECTOR_GETTER(FunctionDeclaration *, method);
    
    const String *Identifier() const override;
    class Type *Type() const override;
    Declaration *AtItem(size_t i) const override;
    size_t ItemSize() const override;
    
    DECLARE_AST_NODE(ObjectDeclaration);
private:
    const String *name_;
    base::ArenaVector<VariableDeclaration *> fields_;
    base::ArenaVector<FunctionDeclaration *> methods_;
    class Type *dummy_ = nullptr;
    //ClassDefinition *shadow_class_ = nullptr;
}; // class ObjectDeclaration

//----------------------------------------------------------------------------------------------------------------------
// Definitions
//----------------------------------------------------------------------------------------------------------------------
struct FieldOfStructure {
    bool in_constructor; // in constructor
    int  as_constructor; // index of constructor parameter
    VariableDeclaration *declaration;
}; // struct Field

struct ParameterOfConstructor {
    bool field_declaration;
    union {
        int as_field; // index of field
        VariableDeclaration::Item *as_parameter;
    };
}; // struct Field

class Definition : public Statement {
public:
    DEF_PTR_GETTER(const String, name);
    DEF_PTR_PROP_RW(Package, package);
    DEF_PTR_PROP_RW(AstNode, owns);
    DEF_PTR_PROP_RW(Definition, original);
    DEF_PTR_PROP_RW(AnnotationDeclaration, annotations);
    DEF_VAL_PROP_RW(Access, access);
    DEF_VAL_PROP_RW(bool, has_instantiated);
    DEF_ARENA_VECTOR_GETTER(GenericParameter *, generic_param);
    
    std::string FullName() const;
    const String *PackageName() const;
    
    static bool IsNot(AstNode *node) { return !Is(node); }
    static bool Is(AstNode *node);
protected:
    Definition(base::Arena *arena, Kind kind, const String *name, const SourcePosition &source_position);
    
private:
    const String *name_;
    Package *package_ = nullptr;
    AstNode *owns_ = nullptr; // <FileUnit|ClassDefinition|StructDefinition>
    Definition *original_ = nullptr;
    base::ArenaVector<GenericParameter *> generic_params_;
    AnnotationDeclaration *annotations_ = nullptr;
    Access access_ = kDefault;
    bool has_instantiated_ = false;
}; // class Definition

class InterfaceDefinition : public Definition {
public:
    InterfaceDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position);
    
    Statement *FindSymbolOrNull(std::string_view name) const {
        for (auto fun : methods_) {
            if (name.compare(fun->name()->ToSlice()) == 0) {
                return fun;
            }
        }
        return nullptr;
    }

    DEF_ARENA_VECTOR_GETTER(FunctionDeclaration *, method);
    
    DECLARE_AST_NODE(InterfaceDefinition);
private:
    base::ArenaVector<FunctionDeclaration *> methods_;
}; // class InterfaceDefinition

class AnnotationDefinition : public Definition {
public:
    struct Member {
        VariableDeclaration::Item *field = nullptr;
        Expression *default_value = nullptr;
    }; // struct Member
    
    AnnotationDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position);
    
    DEF_ARENA_VECTOR_GETTER(Member, member);
    
    DECLARE_AST_NODE(AnnotationDefinition);
private:
    base::ArenaVector<Member> members_;
}; // class AnnotationDefinition

class IncompletableDefinition : public Definition {
public:
    using Field = FieldOfStructure;
    using Parameter = ParameterOfConstructor;
    
    DEF_ARENA_VECTOR_GETTER(Field, field);
    DEF_ARENA_VECTOR_GETTER(FunctionDeclaration *, method);
    DEF_ARENA_VECTOR_GETTER(Parameter, parameter);
    DEF_PTR_PROP_RW(Calling, super_calling);
    DEF_PTR_PROP_RW(FunctionDeclaration, primary_constructor);
    
    Statement *FindLocalSymbolOrNull(std::string_view name) const;
protected:
    IncompletableDefinition(Node::Kind kind, base::Arena *arena, const String *name,
                            const SourcePosition &source_position);
    
    base::ArenaVector<Parameter> parameters_;
    base::ArenaMap<std::string_view, size_t> named_parameters_;
    base::ArenaVector<Field> fields_;
    base::ArenaVector<FunctionDeclaration *> methods_;
    Calling *super_calling_ = nullptr;
    FunctionDeclaration *primary_constructor_ = nullptr;
}; // class IncompletableDefinition


class StructDefinition : public IncompletableDefinition {
public:
    StructDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position);
    
    Statement *FindSymbolOrNull(std::string_view name) const {
        return std::get<0>(FindSymbolWithOwns(name));
    }
    
    std::tuple<Statement *, const IncompletableDefinition *> FindSymbolWithOwns(std::string_view name) const {
        for (auto owns = this; owns != nullptr; owns = owns->base_of()) {
            if (auto ast = owns->FindLocalSymbolOrNull(name)) {
                return std::make_tuple(ast, owns);
            }
        }
        return std::make_tuple(nullptr, nullptr);
    }
    
    bool IsNotBaseOf(const IncompletableDefinition *target) const { return !IsBaseOf(target); }
    
    bool IsBaseOf(const IncompletableDefinition *target) const {
        for (auto owns = base_of(); owns != nullptr; owns = owns->base_of()) {
            if (owns == target) {
                return true;
            }
        }
        return false;
    }

    DEF_PTR_PROP_RW(StructDefinition, base_of);
    DECLARE_AST_NODE(StructDefinition);
private:
    StructDefinition *base_of_ = nullptr;
}; // class StructDefinition

class ClassDefinition : public IncompletableDefinition {
public:
    ClassDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position);

    DEF_PTR_PROP_RW(ClassDefinition, base_of);
    DEF_ARENA_VECTOR_GETTER(Type *, concept);
    Type **mutable_concept(size_t i) {
        assert(i < concepts_size());
        return &concepts_[i];
    }

    Statement *FindSymbolOrNull(std::string_view name) const {
        return std::get<0>(FindSymbolWithOwns(name));
    }
    
    std::tuple<Statement *, const IncompletableDefinition *> FindSymbolWithOwns(std::string_view name) const {
        for (auto owns = this; owns != nullptr; owns = owns->base_of()) {
            if (auto ast = owns->FindLocalSymbolOrNull(name)) {
                return std::make_tuple(ast, owns);
            }
        }
        return std::make_tuple(nullptr, nullptr);
    }
    
    bool IsNotBaseOf(const IncompletableDefinition *target) const { return !IsBaseOf(target); }
    
    bool IsBaseOf(const IncompletableDefinition *target) const {
        for (auto owns = base_of(); owns != nullptr; owns = owns->base_of()) {
            if (owns == target) {
                return true;
            }
        }
        return false;
    }
    
    bool IsNotConceptOf(const InterfaceDefinition *interface) const { return !IsConceptOf(interface); }
    bool IsConceptOf(const InterfaceDefinition *interface) const;
    
    DECLARE_AST_NODE(ClassDefinition);
private:
    ClassDefinition *base_of_ = nullptr;
    base::ArenaVector<Type *> concepts_;
    //base::ArenaVector<InterfaceDefinition *> implements_;
}; // class ClassDefinition

//----------------------------------------------------------------------------------------------------------------------
// Annotation
//----------------------------------------------------------------------------------------------------------------------
class AnnotationDeclaration : public AstNode {
public:
    AnnotationDeclaration(base::Arena *arena, const SourcePosition &source_position);

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
    
    Annotation(base::Arena *arena, Symbol *name, const SourcePosition &source_position);

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
    
    bool IsExplicitExpression() const override { return true; }
protected:
    Expression(Kind kind, bool is_lval, bool is_rval, const SourcePosition &source_position);
    
    bool is_lval_;
    bool is_rval_;
}; //class Expression


class Identifier : public Expression {
public:
    Identifier(const String *name, const SourcePosition &source_position);
    
    DEF_PTR_GETTER(const String, name);
    
    bool IsNotPlaceholder() const { return !IsPlaceholder(); }
    bool IsPlaceholder() const { return IsPlaceholder(name_); }
    
    constexpr static bool IsNotPlaceholder(const String *name) { return !IsPlaceholder(name); }
    constexpr static bool IsPlaceholder(const String *name) { return name->Equal("_"); }

    DECLARE_AST_NODE(Identifier);
private:
    const String *name_;
}; // class Identifier

class Instantiation : public Expression {
public:
    Instantiation(base::Arena *arena, Expression *primary, const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(Expression, primary);
    DEF_ARENA_VECTOR_GETTER(Type *, generic_arg);
    Type **mutable_generic_arg(size_t i) {
        assert(i < generic_args_size());
        return &generic_args_[i];
    }
    
    DEF_PTR_PROP_RW(Statement, instantiated);
    
    DECLARE_AST_NODE(Instantiation);
private:
    Expression *primary_; // <Identifer | Dot>
    base::ArenaVector<Type *> generic_args_;
    Statement *instantiated_ = nullptr;
}; // class Instantiation

class Literal : public Expression {
public:
    DEF_PTR_PROP_RW(Type, type);
    
protected:
    Literal(Kind kind, Type *type, const SourcePosition &source_position);
    
    Type *type_;
}; // class Literal


class UnitLiteral : public Literal {
public:
    UnitLiteral(base::Arena *arena, const SourcePosition &source_position);
    DECLARE_AST_NODE(UnitLiteral);
}; // class UnitLiteral

class EmptyLiteral : public Literal {
public:
    EmptyLiteral(const SourcePosition &source_position);
    DECLARE_AST_NODE(EmptyLiteral);
}; // class EmptyLiteral


class OptionLiteral : public Literal {
public:
    OptionLiteral(base::Arena *arena, Expression *value, const SourcePosition &source_position);
    DEF_PTR_PROP_RW(Expression, value);
    
    bool is_none() { return value() == nullptr; }
    bool is_some() { return !is_none(); }
    
    static OptionLiteral *None(base::Arena *arena, const SourcePosition &source_position) {
        return new (arena) OptionLiteral(arena, nullptr, source_position);
    }
    
    static OptionLiteral *Some(base::Arena *arena, Expression *value, const SourcePosition &source_position) {
        return new (arena) OptionLiteral(arena, value, source_position);
    }
    
    DECLARE_AST_NODE(OptionLiteral);
private:
    Expression *value_ = nullptr;
}; // class OptionLiteral

template<class T>
struct LiteralTraits {
    static constexpr Node::Kind kKind = Node::kMaxKinds;
    using NodeType = Node;
    
    static int Accept(NodeType *node, AstVisitor *visitor) {}
    static Type *Mold(base::Arena *, const SourcePosition &) { return nullptr; }
}; // struct LiteralTraits

template<> struct LiteralTraits<int> {
    static constexpr Node::Kind kKind = Node::kIntLiteral;
    using NodeType = IntLiteral;
    
    static int Accept(NodeType *node, AstVisitor *visitor) { return visitor->VisitIntLiteral(node); }
    static inline Type *Mold(base::Arena *arena, const SourcePosition &location);
}; // struct LiteralTraits<int>

template<> struct LiteralTraits<unsigned> {
    static constexpr Node::Kind kKind = Node::kUIntLiteral;
    using NodeType = UIntLiteral;
    
    static int Accept(NodeType *node, AstVisitor *visitor) { return visitor->VisitUIntLiteral(node); }
    static inline Type *Mold(base::Arena *arena, const SourcePosition &location);
}; // struct LiteralTraits<unsigned>

template<> struct LiteralTraits<int64_t> {
    static constexpr Node::Kind kKind = Node::kI64Literal;
    using NodeType = I64Literal;
    
    static int Accept(NodeType *node, AstVisitor *visitor) { return visitor->VisitI64Literal(node); }
    static inline Type *Mold(base::Arena *arena, const SourcePosition &location);
}; // struct LiteralTraits<int64_t>

template<> struct LiteralTraits<uint64_t> {
    static constexpr Node::Kind kKind = Node::kU64Literal;
    using NodeType = U64Literal;
    
    static int Accept(NodeType *node, AstVisitor *visitor) { return visitor->VisitU64Literal(node); }
    static inline Type *Mold(base::Arena *arena, const SourcePosition &location);
}; // struct LiteralTraits<uint64_t>

template<> struct LiteralTraits<float> {
    static constexpr Node::Kind kKind = Node::kF32Literal;
    using NodeType = F32Literal;
    
    static int Accept(NodeType *node, AstVisitor *visitor) { return visitor->VisitF32Literal(node); }
    static inline Type *Mold(base::Arena *arena, const SourcePosition &location);
}; // struct LiteralTraits<float>

template<> struct LiteralTraits<double> {
    static constexpr Node::Kind kKind = Node::kF64Literal;
    using NodeType = F64Literal;
    
    static int Accept(NodeType *node, AstVisitor *visitor) { return visitor->VisitF64Literal(node); }
    static inline Type *Mold(base::Arena *arena, const SourcePosition &location);
}; // struct LiteralTraits<double>

template<> struct LiteralTraits<bool> {
    static constexpr Node::Kind kKind = Node::kBoolLiteral;
    using NodeType = BoolLiteral;
    
    static int Accept(NodeType *node, AstVisitor *visitor) { return visitor->VisitBoolLiteral(node); }
    static inline Type *Mold(base::Arena *arena, const SourcePosition &location);
}; // struct LiteralTraits<bool>

template<> struct LiteralTraits<char32_t> {
    static constexpr Node::Kind kKind = Node::kCharLiteral;
    using NodeType = CharLiteral;
    
    static int Accept(NodeType *node, AstVisitor *visitor) { return visitor->VisitCharLiteral(node); }
    static inline Type *Mold(base::Arena *arena, const SourcePosition &location);
}; // struct LiteralTraits<bool>

template<> struct LiteralTraits<const String *> {
    static constexpr Node::Kind kKind = Node::kStringLiteral;
    using NodeType = StringLiteral;
    
    static int Accept(NodeType *node, AstVisitor *visitor) { return visitor->VisitStringLiteral(node); }
    static inline Type *Mold(base::Arena *arena, const SourcePosition &location);
}; // struct LiteralTraits<bool>

template<class T>
class ActualLiteral : public Literal {
public:
    DEF_VAL_PROP_RW(T, value);
    
    int Accept(AstVisitor *visitor) override {
        return LiteralTraits<T>::Accept(static_cast<typename LiteralTraits<T>::NodeType *>(this), visitor);
    }
protected:
    inline ActualLiteral(base::Arena *arena, T value, const SourcePosition &source_position)
        : Literal(LiteralTraits<T>::kKind, LiteralTraits<T>::Mold(arena, source_position), source_position)
        , value_(value) {}
    
    T value_;
}; // template<class T> class ActualLiteral


#define DEFINE_ACTUAL_LITERAL(name, type) \
    class name##Literal : public ActualLiteral<type> { \
    public: \
        name##Literal(base::Arena *arena, type value, const SourcePosition &source_position): \
            ActualLiteral<type>(arena, value, source_position) {} \
    }

DEFINE_ACTUAL_LITERAL(Int, int);
DEFINE_ACTUAL_LITERAL(UInt, unsigned);
DEFINE_ACTUAL_LITERAL(I64, int64_t);
DEFINE_ACTUAL_LITERAL(U64, uint64_t);
DEFINE_ACTUAL_LITERAL(F32, float);
DEFINE_ACTUAL_LITERAL(F64, double);
DEFINE_ACTUAL_LITERAL(Bool, bool);
DEFINE_ACTUAL_LITERAL(Char, char32_t);
DEFINE_ACTUAL_LITERAL(String, const String *);


class LambdaLiteral : public Literal {
public:
    LambdaLiteral(FunctionPrototype *prototype, Statement *body, const SourcePosition &source_position);

    DEF_PTR_PROP_RW(FunctionPrototype, prototype);
    DEF_PTR_PROP_RW(Statement, body);
    
    DECLARE_AST_NODE(LambdaLiteral);
private:
    FunctionPrototype *prototype_;
    Statement *body_;
}; // class LambdaLiteral

class ArrayInitializer : public Literal {
public:
    ArrayInitializer(base::Arena *arena, Type *type, int dimension_count, const SourcePosition &source_position);
    
    DEF_VAL_PROP_RW(int, dimension_count);
    DEF_ARENA_VECTOR_GETTER(AstNode *, dimension);
    DEF_PTR_PROP_RW(Expression, filling_value);

    DECLARE_AST_NODE(ArrayInitializer);
private:
    int dimension_count_;
    base::ArenaVector<AstNode *> dimensions_;
    Expression *filling_value_ = nullptr;
}; // class ArrayInitializer

class ChannelInitializer : public Literal {
public:
    ChannelInitializer(Type *type, Expression *capacity, const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(Expression, capacity);
    
    DECLARE_AST_NODE(ChannelInitializer);
private:
    Expression *capacity_;
}; // class ChannelInitializer

//class ExpressionWithOperands<N>(rval = true, lval = false) : Expression
//    = lhs(): Expression *
//    = rhs(): Expression *
//    = operand(i: int): Expression *
//    + operand_count: int
//    + operands: Expression *[N]
template<int N>
class ExpressionWithOperands : public Expression {
public:
    Expression *operand(int i) {
        assert(i >= 0 && i < operands_count_);
        return DCHECK_NOTNULL(operands_[i]);
    }
    
    void set_operand(int i, Expression *expr) {
        assert(i >= 0 && i < operands_count_);
        operands_[i] = DCHECK_NOTNULL(expr);
    }
    
protected:
    inline ExpressionWithOperands(Kind kind, const SourcePosition &source_position)
        : Expression(kind, false, true, source_position)
        , operands_count_(N) {
        int i = operands_count_;
        while (i-- > 0) {
            operands_[i] = nullptr;
        }
    }

    int operands_count_;
    Expression *operands_[N];
}; // class ExpressionWithOperands

class BinaryExpression : public ExpressionWithOperands<2> {
public:
    BinaryExpression(Kind kind, Expression *lhs, Expression *rhs, const SourcePosition &source_position);
    
    Expression *lhs() { return operands_[0]; }

    Expression *rhs() {
        assert(operands_count_ > 1);
        return operands_[1];
    }
}; // class BinaryExpression

class UnaryExpression : public ExpressionWithOperands<1> {
public:
    UnaryExpression(Kind kind, Expression *operand, const SourcePosition &source_position);
    Expression *operand() { return DCHECK_NOTNULL(operands_[0]); }
}; // class UnaryExpression

#define Unary_VARGS   Expression *operand
#define Unary_PARAMS  operand
#define Binary_VARGS  Expression *lhs, Expression *rhs
#define Binary_PARAMS lhs, rhs

#define DECLARE_EXPRESSION_WITH_OPERANDS(V) \
    V(Negative,        Unary)  \
    V(Add,             Binary) \
    V(Sub,             Binary) \
    V(Mul,             Binary) \
    V(Div,             Binary) \
    V(Mod,             Binary) \
    V(Equal,           Binary) \
    V(NotEqual,        Binary) \
    V(Less,            Binary) \
    V(LessEqual,       Binary) \
    V(Greater,         Binary) \
    V(GreaterEqual,    Binary) \
    V(And,             Binary) \
    V(Or,              Binary) \
    V(Not,             Unary)  \
    V(BitwiseAnd,      Binary) \
    V(BitwiseOr,       Binary) \
    V(BitwiseXor,      Binary) \
    V(BitwiseNegative, Unary)  \
    V(BitwiseShl,      Binary) \
    V(BitwiseShr,      Binary) \
    V(Recv,            Unary)  \
    V(Send,            Binary) \
    V(IndexedGet,      Binary) \
    V(AssertedGet,     Unary) \

#define DEFINE_CLASS(name, base) \
    class name : public base##Expression { \
    public: \
        name (base##_VARGS, const SourcePosition &source_position); \
        DECLARE_AST_NODE(name); \
    };
DECLARE_EXPRESSION_WITH_OPERANDS(DEFINE_CLASS)
#undef DEFINE_CLASS


class Dot : public Expression {
public:
    Dot(Expression *primary, const String *field, const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(Expression, primary);
    DEF_PTR_PROP_RW(const String, field);

    DECLARE_AST_NODE(Dot);
private:
    Expression *primary_;
    const String *field_;
}; // class Dot

class Casting : public Expression {
public:
    Casting(Expression *source, Type *destination, const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(Expression, source);
    DEF_PTR_PROP_RW(Type, destination);
    
    DECLARE_AST_NODE(Casting);
private:
    Expression *source_;
    Type *destination_;
}; // class Casting

class Testing : public Expression {
public:
    Testing(Expression *source, Type *destination, const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(Expression, source);
    DEF_PTR_PROP_RW(Type, destination);
    
    DECLARE_AST_NODE(Testing);
private:
    Expression *source_;
    Type *destination_;
}; // class Testing

class Calling : public Expression {
public:
    Calling(base::Arena *arena, Expression *callee, const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(Expression, callee);
    DEF_ARENA_VECTOR_GETTER(Expression *, arg);
    
    DECLARE_AST_NODE(Calling);
private:
    Expression *callee_;
    base::ArenaVector<Expression *> args_;
}; // class Calling

//class Constructing : public Expression {
//public:
//    Constructing(base::Arena *arena, IncompletableDefinition *mold, const SourcePosition &source_position);
//    
//    DEF_PTR_PROP_RW(IncompletableDefinition, mold);
//    DEF_ARENA_VECTOR_GETTER(Expression *, arg);
//    
//    DECLARE_AST_NODE(Constructing);
//private:
//    IncompletableDefinition *mold_; // <ClassDefinition|StructDefinition>
//    base::ArenaVector<Expression *> args_;
//};

class IfExpression : public Expression {
public:
    IfExpression(base::Arena *arena, Statement *initializer, Expression *condition, Statement *then_clause,
                 Statement *else_clause, const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(Statement, initializer);
    DEF_PTR_PROP_RW(Expression, condition);
    DEF_PTR_PROP_RW(Statement, then_clause);
    DEF_PTR_PROP_RW(Statement, else_clause);
    DEF_ARENA_VECTOR_GETTER(Type *, reduced_type);
    
    DECLARE_AST_NODE(IfExpression);
private:
    Statement *initializer_;
    Expression *condition_;
    Statement *then_clause_;
    Statement *else_clause_;
    base::ArenaVector<Type *> reduced_types_;
}; // class IfExpression

class CaseWhenPattern : public Node {
public:
    enum Pattern {
        kExpectValues,
        kTypeTesting,
        kBetweenTo,
        kStructMatching,
    };
    CaseWhenPattern(Pattern pattern, Statement *then_clause, const SourcePosition &source_position);
    
    DEF_VAL_GETTER(Pattern, pattern);
    DEF_PTR_PROP_RW(Statement, then_clause);
private:
    Pattern pattern_;
    Statement *then_clause_;
}; // class WhenCasePattern

class WhenExpression : public Expression {
public:
    using Case = CaseWhenPattern;

    #define DEFINE_CASE_METHODS(name) \
        static name##Case *Cast(Case *from) { \
            return from->pattern() == k##name ? static_cast<name##Case *>(from) : nullptr; \
        } \
        static const name##Case *Cast(const Case *from) { \
            return from->pattern() == k##name ? static_cast<const name##Case *>(from) : nullptr; \
        }
    
    // value -> then_clause
    class ExpectValuesCase : public Case {
    public:
        ExpectValuesCase(base::Arena *arena, Statement *then_clause, const SourcePosition &source_position);
        
        DEF_ARENA_VECTOR_GETTER(Expression *, match_value);

        DEFINE_CASE_METHODS(ExpectValues);
    private:
        base::ArenaVector<Expression *> match_values_;
    }; // class ExpectValueCase
    
    // variable: type -> then_clause
    class TypeTestingCase : public Case {
    public:
        TypeTestingCase(Identifier *name, Type *match_type, Statement *then_clause,
                        const SourcePosition &source_position);
        
        DEF_PTR_GETTER(Identifier, name);
        DEF_PTR_PROP_RW(Type, match_type);
        
        DEFINE_CASE_METHODS(TypeTesting);
    private:
        Identifier *name_;
        Type *match_type_;
    }; // class TypeTestingCase
    
    // `in' literal..literal
    class BetweenToCase : public Case {
    public:
        BetweenToCase(Expression *lower, Expression *upper, Statement *then_clause, bool is_close,
                      const SourcePosition &source_position);

        DEF_PTR_PROP_RW(Expression, lower);
        DEF_PTR_PROP_RW(Expression, upper);
        DEF_VAL_GETTER(bool, is_close);
        
        DEFINE_CASE_METHODS(BetweenTo);
    private:
        Expression *lower_;
        Expression *upper_;
        bool is_close_;
    }; // class BetweenToCase
    
    // Foo { name, id } -> name, ... id, ...
    class StructMatchingCase : public Case {
    public:
        StructMatchingCase(base::Arena *arena, Type *match_type, Statement *then_clause,
                           const SourcePosition &source_position);
        
        DEF_PTR_PROP_RW(Type, match_type);
        DEF_ARENA_VECTOR_GETTER(Identifier *, expected);
        
        DEFINE_CASE_METHODS(StructMatching);
    private:
        Type *match_type_;
        base::ArenaVector<Identifier *> expecteds_;
    }; // case StructMatchingCase
    
    #undef DEFINE_CASE_METHODS
    
    WhenExpression(base::Arena *arena, Statement *initializer, Expression *destination,
                   const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(Statement, initializer);
    DEF_PTR_PROP_RW(Expression, destination);
    DEF_ARENA_VECTOR_GETTER(Case *, case_clause);
    DEF_ARENA_VECTOR_GETTER(Type *, reduced_type);
    DEF_PTR_PROP_RW(Statement, else_clause);
    
    DECLARE_AST_NODE(WhenExpression);
private:
    Statement *initializer_;
    Expression *destination_;
    base::ArenaVector<Case *> case_clauses_;
    Statement *else_clause_ = nullptr;
    base::ArenaVector<Type *> reduced_types_;
}; // class WhenExpression

class TryCatchExpression : public Expression {
public:
    using CatchClause = WhenExpression::TypeTestingCase;
    
    TryCatchExpression(base::Arena *arena, Block *try_block, Block *finally_block,
                       const SourcePosition &source_position);
    
    DEF_PTR_PROP_RW(Block, try_block);
    DEF_PTR_PROP_RW(Block, finally_block);
    DEF_ARENA_VECTOR_GETTER(CatchClause *, catch_clause);
    DEF_ARENA_VECTOR_GETTER(Type *, reduced_type);
    
    DECLARE_AST_NODE(TryCatchExpression);
private:
    Block *try_block_;
    Block *finally_block_;
    base::ArenaVector<CatchClause *> catch_clauses_;
    base::ArenaVector<Type *> reduced_types_;
}; // class TryCatchExpression

class StringTemplate : public Expression {
public:
    StringTemplate(base::Arena *arena, const SourcePosition &source_position);
    
    DEF_ARENA_VECTOR_GETTER(Expression *, part);
    
    DECLARE_AST_NODE(StringTemplate);
private:
    base::ArenaVector<Expression *> parts_;
}; // class StringTemplate

//----------------------------------------------------------------------------------------------------------------------
// Types
//----------------------------------------------------------------------------------------------------------------------
#define DECLARE_TYPE_CATEGORIES(V) \
    V(Primary,   Type) \
    V(Channel,   ChannelType) \
    V(Array,     ArrayType) \
    V(Class,     ClassType) \
    V(Struct,    StructType) \
    V(Option,    OptionType) \
    V(Interface, InterfaceType) \
    V(Function,  FunctionPrototype)

#define DECLARE_PRIMARY_TYPE(V) \
    V(unit) \
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
    V(option) \
    V(interface) \
    V(function) \
    V(none) \
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
    
    using Linker = std::function<Type*(const Symbol *, Type *)>;
    
    Type(base::Arena *arena, Primary primary_type, const SourcePosition &source_position)
        : Type(arena, kPrimary, primary_type, nullptr, source_position) {}
    
    Type(base::Arena *arena, const Symbol *identifier, const SourcePosition &source_position)
        : Type(arena, kPrimary, kType_symbol, identifier, source_position) {}
    
    DEF_VAL_GETTER(Category, category);
    DEF_VAL_PROP_RW(Primary, primary_type);
    DEF_PTR_PROP_RW(const Symbol, identifier);
    DEF_ARENA_VECTOR_GETTER(Type *, generic_arg);
    
    static bool IsNot(Node *node) { return !Is(node); }
    static bool Is(Node *node);
    
#define DEFINE_METHOD(name, node) \
    bool Is##name##Type() const { return category_ == k##name; }
    DECLARE_TYPE_CATEGORIES(DEFINE_METHOD)
#undef  DEFINE_METHOD
    
    bool IsComparable() const;
    bool IsNumber() const { return IsIntegral() || IsFloating(); }
    bool IsIntegral() const { return IsUnsignedIntegral() || IsSignedIntegral(); }
    bool IsFloating() const;
    bool IsUnsignedIntegral() const;
    bool IsSignedIntegral() const;
    
    bool IsNotConditionVal() const { return !IsConditionVal(); }
    bool IsConditionVal() const { return IsOptionType() || primary_type() == kType_bool; }
    
    bool IsNotCharAndByte() const { return !IsCharOrByte(); }
    bool IsCharOrByte() const {
        return primary_type() == kType_i8 || primary_type() == kType_u8 || primary_type() == kType_char;
    }
    
    virtual bool Acceptable(const Type *rhs, bool *unlinked) const;
    virtual Type *Link(Linker &&linker);
    virtual std::string ToString() const;
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
    ArrayType(base::Arena *arena, Type *element_type, int dimension_count, const SourcePosition &source_position);
    
    int dimension_count() const { return static_cast<int>(dimension_capacitys_size()); }
    Type *element_type() const { return generic_arg(0); }
    DEF_ARENA_VECTOR_GETTER(Expression *, dimension_capacity);
    
    bool HasNotCapacities() const {
        return std::count(dimension_capacitys_.begin(), dimension_capacitys_.end(), nullptr) == dimension_count();
    }
    
    bool HasCapacities() const {
        return std::count_if(dimension_capacitys_.begin(),
                             dimension_capacitys_.end(),
                             [](auto expr) { return expr != nullptr; }) == dimension_count();
    }
    
    bool Acceptable(const Type *rhs, bool *unlinked) const override;
    std::string ToString() const override;
private:
    base::ArenaVector<Expression *> dimension_capacitys_;
}; // class ArrayType

class OptionType : public Type {
public:
    OptionType(base::Arena *arena, Type *element_type, const SourcePosition &source_position);
    
    Type *element_type() const { return generic_arg(0); }
    
    static bool DoesElementIs(const Type *type, Primary primary) {
        return !type->IsOptionType() ? false : (type->AsOptionType()->element_type()->primary_type() == primary);
    }
    
    bool Acceptable(const Type *rhs, bool *unlinked) const override;
    Type *Link(Linker &&linker) override;
    std::string ToString() const override;
}; // class OptionType

class ChannelType : public Type {
public:
    static constexpr int kInbility = 1;
    static constexpr int kOutbility = 2;
    
    ChannelType(base::Arena *arena, int ability, Type *element_type, const SourcePosition &source_position)
        : Type(arena, Type::kChannel, element_type->primary_type(), nullptr, source_position)
        , ability_(ability) {
        mutable_generic_args()->push_back(DCHECK_NOTNULL(element_type));
        assert(ability_ > 0);
    }

    DEF_VAL_GETTER(int, ability);
    Type *element_type() const { return generic_arg(0); }
    
    bool CanRead() const { return ability_ & kInbility; }
    bool CanWrite() const { return ability_ & kOutbility; }
    bool CanIO() const { return ability_ & (kInbility | kOutbility); }
    bool Readonly() const { return ability_ == kInbility; }

    bool Acceptable(const Type *rhs, bool *unlinked) const override;
    std::string ToString() const override;
private:
    int ability_ = kInbility | kOutbility;
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
    
    static bool DoNotClassBaseOf(const Type *type, ClassDefinition *base) {
        return !DoesClassBaseOf(type, base);
    }
    
    static bool DoesClassBaseOf(const Type *type, ClassDefinition *base) {
        return !type->IsClassType() ? false : (type->AsClassType()->definition()->IsBaseOf(base));
    }
    
    bool Acceptable(const Type *rhs, bool *unlinked) const override;
    std::string ToString() const override;
}; // class ClassType

class StructType : public UDTType<StructDefinition> {
public:
    StructType(base::Arena *arena, StructDefinition *definition, const SourcePosition &source_position)
        : UDTType<StructDefinition>(arena, definition, source_position) {}
    
    bool Acceptable(const Type *rhs, bool *unlinked) const override;
    std::string ToString() const override;
}; // class StructType

class InterfaceType : public UDTType<InterfaceDefinition> {
public:
    InterfaceType(base::Arena *arena, InterfaceDefinition *definition, const SourcePosition &source_position)
        : UDTType<InterfaceDefinition>(arena, definition, source_position) {}
    
    bool Acceptable(const Type *rhs, bool *unlinked) const override;
    std::string ToString() const override;
}; // class InterfaceType

class FunctionPrototype : public Type {
public:
    FunctionPrototype(base::Arena *arena, bool vargs, const SourcePosition &source_position)
        : Type(arena, Type::kFunction, Type::kType_function, nullptr, source_position)
        , params_(arena)
        , return_types_(arena)
        , vargs_(vargs) {}

    DEF_VAL_PROP_RW(bool, vargs);
    DEF_PTR_PROP_RW(String, signature);
    DEF_ARENA_VECTOR_GETTER(Node *, param);
    DEF_ARENA_VECTOR_GETTER(Type *, return_type);
    
    std::string MakeSignature() const;
    
    bool Acceptable(const Type *rhs, bool *unlinked) const override;
    Type *Link(Linker &&linker) override;
    std::string ToString() const override;
private:
    base::ArenaVector<Node *> params_; // <VariableDeclaration::Item | Type>
    base::ArenaVector<Type *> return_types_;
    bool vargs_;
    String *signature_ = nullptr;
}; // class FunctionPrototype


#define DEFINE_METHODS(name) \
inline name *Node::As##name() { return !Is##name() ? nullptr : static_cast<name *>(this); } \
    inline const name *Node::As##name() const { return !Is##name() ? nullptr : static_cast<const name *>(this); }
    DECLARE_AST_NODES(DEFINE_METHODS)
    DECLARE_TYPE_NODES(DEFINE_METHODS)
#undef DEFINE_METHODS

inline Type *LiteralTraits<int>::Mold(base::Arena *arena, const SourcePosition &location) {
    return new (arena) Type(arena, Type::kType_i32, location);
}

inline Type *LiteralTraits<unsigned>::Mold(base::Arena *arena, const SourcePosition &location) {
    return new (arena) Type(arena, Type::kType_u32, location);
}

inline Type *LiteralTraits<int64_t>::Mold(base::Arena *arena, const SourcePosition &location) {
    return new (arena) Type(arena, Type::kType_i64, location);
}

inline Type *LiteralTraits<uint64_t>::Mold(base::Arena *arena, const SourcePosition &location) {
    return new (arena) Type(arena, Type::kType_u64, location);
}

inline Type *LiteralTraits<float>::Mold(base::Arena *arena, const SourcePosition &location) {
    return new (arena) Type(arena, Type::kType_f32, location);
}

inline Type *LiteralTraits<double>::Mold(base::Arena *arena, const SourcePosition &location) {
    return new (arena) Type(arena, Type::kType_f64, location);
}

inline Type *LiteralTraits<bool>::Mold(base::Arena *arena, const SourcePosition &location) {
    return new (arena) Type(arena, Type::kType_bool, location);
}

inline Type *LiteralTraits<char32_t>::Mold(base::Arena *arena, const SourcePosition &location) {
    return new (arena) Type(arena, Type::kType_char, location);
}

inline Type *LiteralTraits<const String *>::Mold(base::Arena *arena, const SourcePosition &location) {
    return new (arena) Type(arena, Type::kType_string, location);
}

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_AST_H_
