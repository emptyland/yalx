#pragma once
#ifndef YALX_IR_METADATA_H_
#define YALX_IR_METADATA_H_

#include "ir/type.h"
#include "base/checking.h"
#include "base/arena-utils.h"
#include "base/arena.h"
#include <tuple>

namespace yalx {

namespace ir {

class Function;
class Module;
class Model;
using String = base::ArenaString;

class Handle : public base::ArenaObject {
public:
    DEF_PTR_GETTER(const Model, owns);
    DEF_VAL_GETTER(size_t, offset);
    
    const String *name() const {
        return reinterpret_cast<const String *>(reinterpret_cast<uintptr_t>(name_) & (~1ULL));
    }

    bool IsMethod() const { return !IsField(); }
    bool IsField() const { return reinterpret_cast<uintptr_t>(name_) & 1u; }
    
    static Handle *Field(base::Arena *arena, const Model *owns, const String *name, size_t offset) {
        return new (arena) Handle(owns, name, offset, true/*field_or_method*/);
    }
    
    static Handle *Method(base::Arena *arena, const Model *owns, const String *name, size_t offset) {
        return new (arena) Handle(owns, name, offset, false/*field_or_method*/);
    }
private:
    Handle(const Model *owns, const String *name, size_t offset, bool field_or_method)
    : owns_(owns)
    , name_(MakeTaggedName(name, field_or_method))
    , offset_(offset) {
    }
    
    static const String *MakeTaggedName(const String *name, bool field_or_method) {
        auto tagged = reinterpret_cast<uintptr_t>(name);
        return reinterpret_cast<const String *>(tagged | (field_or_method ? 1u : 0u));
    }
    
    const Model *owns_;
    const String *name_;
    const size_t offset_;
};

class Model : public base::ArenaObject {
public:
    enum Constraint {
        kVal,
        kRef,
    };
    
    enum Declaration {
        kClass,
        kStruct,
        kInterface,
        kFunction,
        kArray,
        kChannel,
    };
    
    struct Field {
        const String *name;
        Access access;
        ptrdiff_t offset;
        Type type;
        bool is_volatile;
    };
    
    struct Method {
        Function *fun;
        Access access;
        int offset;
    };
    
    DEF_PTR_GETTER(const String, name);
    DEF_PTR_GETTER(const String, full_name);
    DEF_VAL_PROP_RW(Constraint, constraint);
    DEF_VAL_GETTER(Declaration, declaration);
    
    virtual std::tuple<Method, bool> FindMethod(std::string_view name) const;
    virtual std::tuple<Field, bool> FindField(std::string_view name) const;
    virtual Handle *FindMemberOrNull(std::string_view name) const;
    virtual size_t ReferenceSizeInBytes() const = 0;
protected:
    Model(const String *name, const String *full_name, Constraint constraint, Declaration declaration);
    
    const String *const name_;
    const String *const full_name_;
    Declaration const declaration_;
    Constraint constraint_;
}; // class Model

class PrototypeModel : public Model {
public:
    PrototypeModel(base::Arena *arena, const String *name, bool vargs);
    
    static std::string ToString(const Type *params, const size_t params_size, const bool vargs,
                                const Type *return_types, const size_t return_types_size);
    
    size_t ReferenceSizeInBytes() const override;
    
    DEF_ARENA_VECTOR_GETTER(Type, param);
    DEF_ARENA_VECTOR_GETTER(Type, return_type);
    DEF_VAL_GETTER(bool, vargs);
private:
    base::ArenaVector<Type> params_;
    base::ArenaVector<Type> return_types_;
    bool const vargs_;
}; // class PrototypeModel

class InterfaceModel : public Model {
public:
    InterfaceModel(base::Arena *arena, const String *name, const String *full_name);
    
    DEF_ARENA_VECTOR_GETTER(Method, method);
    
    Handle *InsertMethod(Function *fun);

    std::tuple<Method, bool> FindMethod(std::string_view name) const override;
    size_t ReferenceSizeInBytes() const override;
private:
    base::Arena *const arena_;
    base::ArenaMap<std::string_view, Handle *> members_;
    base::ArenaVector<Method> methods_;
}; // class InterfaceModel

class ArrayModel : public Model {
public:
    ArrayModel(base::Arena *arena, const String *name, const String *full_name,
               int dimension_count, const Type element_type);
    
    static std::string ToString(int dimension_count, const Type element_type);
    
    size_t ReferenceSizeInBytes() const override;
private:
    const Type element_type_;
    const int dimension_count_;
}; // class ArrayModel

class ChannelModel : public Model {
public:
    static constexpr int kInbility = 1;
    static constexpr int kOutbility = 2;
    
    bool CanRead() const { return ability_ & kInbility; }
    bool CanWrite() const { return ability_ & kOutbility; }
    bool CanIO() const { return ability_ & (kInbility | kOutbility); }
    bool Readonly() const { return ability_ == kInbility; }
    
private:
    const Type element_type_;
    const int ability_;
}; // class ChannelModel

class StructureModel : public Model {
public:
    StructureModel(base::Arena *arena, const String *name, const String *full_name, Declaration declaration,
                   Module *owns, StructureModel *base_of);
    
    DEF_PTR_GETTER(Module, owns);
    DEF_PTR_PROP_RW(StructureModel, base_of);
    DEF_PTR_PROP_RW(Function, constructor);
    DEF_ARENA_VECTOR_GETTER(InterfaceModel *, implement);
    DEF_ARENA_VECTOR_GETTER(Field, field);
    DEF_ARENA_VECTOR_GETTER(Method, method);
    
    const base::ArenaMap<std::string_view, Handle *> &member_handles() { return members_; }
    
    Handle *InsertField(const Field &field);
    Handle *InsertMethod(const Method &method);
    
    std::tuple<Method, bool> FindMethod(std::string_view name) const override;
    std::tuple<Field, bool> FindField(std::string_view name) const override;
    Handle *FindMemberOrNull(std::string_view name) const override;
    size_t ReferenceSizeInBytes() const override;
private:
    Module *const owns_;
    base::Arena * const arena_;
    StructureModel *base_of_;
    Function *constructor_ = nullptr;
    base::ArenaVector<InterfaceModel *> implements_;
    base::ArenaMap<std::string_view, Handle *> members_;
    base::ArenaVector<Field> fields_;
    base::ArenaVector<Method> methods_;
}; // class StructureModel

} // namespace ir

} // namespace yalx


#endif // YALX_IR_METADATA_H_
