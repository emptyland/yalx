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
using String = base::ArenaString;

class Model : public base::ArenaObject {
public:
    enum Constraint {
        kVal,
        kVar,
    };
    
    struct Field {
        const String *name;
        Constraint constraint;
        Access access;
        ptrdiff_t offset;
        Type type;
        bool is_volatile;
    };
    
    struct Method {
        Function *fun;
        Access access;
        int offset;
        bool is_native;
        bool is_override;
    };
    
    DEF_PTR_GETTER(const String, name);
    DEF_PTR_GETTER(const String, full_name);
    
    virtual std::tuple<Method, bool> FindMethod(std::string_view name) const;
    virtual std::tuple<Field, bool> FindField(std::string_view name) const;
    virtual size_t ReferenceSizeInBytes() const = 0;
protected:
    Model(const String *name, const String *full_name);
    
    const String *const name_;
    const String *const full_name_;
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
    
    void InsertMethod(Function *fun);

    std::tuple<Method, bool> FindMethod(std::string_view name) const override;
    size_t ReferenceSizeInBytes() const override;
private:
    base::ArenaMap<std::string_view, Method> methods_;
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
    enum Declaration {
        kClass,
        kStruct,
    };
    StructureModel(base::Arena *arena, const String *name, const String *full_name, Declaration declaration,
                   Module *owns, StructureModel *base_of);
    
    DEF_VAL_GETTER(Declaration, declaration);
    DEF_PTR_GETTER(Module, owns);
    DEF_PTR_GETTER(StructureModel, base_of);
    DEF_PTR_GETTER(Function, constructor);
    DEF_ARENA_VECTOR_GETTER(InterfaceModel *, implement);
    DEF_ARENA_VECTOR_GETTER(Field, field);
    DEF_ARENA_VECTOR_GETTER(Method, method);
    
    void InsertField(const Field &field);
    void InsertMethod(const Method &method);
    
    std::tuple<Method, bool> FindMethod(std::string_view name) const override;
    std::tuple<Field, bool> FindField(std::string_view name) const override;
    size_t ReferenceSizeInBytes() const override;
private:
    struct Index {
        size_t offset;
        bool field_or_method;
    };
    
    Declaration const declaration_;
    Module *const owns_;
    StructureModel *base_of_;
    Function *constructor_ = nullptr;
    base::ArenaVector<InterfaceModel *> implements_;
    base::ArenaMap<std::string_view, Index> members_;
    base::ArenaVector<Field> fields_;
    base::ArenaVector<Method> methods_;
}; // class StructureModel

} // namespace ir

} // namespace yalx


#endif // YALX_IR_METADATA_H_
