#include "ir/metadata.h"
#include "ir/node.h"

namespace yalx {

namespace ir {

Model::Model(const String *name, const String *full_name, Constraint constraint, Declaration declaration)
: name_(DCHECK_NOTNULL(name))
, full_name_(DCHECK_NOTNULL(full_name))
, constraint_(constraint)
, declaration_(declaration) {}

std::tuple<Model::Method, bool> Model::FindMethod(std::string_view name) const {
    return std::make_tuple(Method{}, false);
}

std::tuple<Model::Field, bool> Model::FindField(std::string_view name) const {
    return std::make_tuple(Field{}, false);
}

Handle *Model::FindMemberOrNull(std::string_view name) const {
    return nullptr;
}

PrototypeModel::PrototypeModel(base::Arena *arena, const String *name, bool vargs)
: Model(name, name, kRef, kFunction)
, params_(DCHECK_NOTNULL(arena))
, return_types_(arena)
, vargs_(vargs) {
}

std::string PrototypeModel::ToString(const Type *params, const size_t params_size, const bool vargs,
                                     const Type *return_types, const size_t return_types_size) {
    std::string full_name("fun (");
    for (size_t i = 0; i < params_size; i++) {
        if (i > 0) {
            full_name.append(",");
        }
        full_name.append(params[i].ToString());
    }
    if (vargs) {
        full_name.append("...");
    }
    if (return_types_size > 0) {
        full_name.append(")->(");
    } else {
        full_name.append(")->");
    }
    for (size_t i = 0; i < return_types_size; i++) {
        if (i > 0) {
            full_name.append(",");
        }
        full_name.append(return_types[i].ToString());
    }
    if (return_types_size > 0) {
        full_name.append(")");
    }
    return full_name;
}

size_t PrototypeModel::ReferenceSizeInBytes() const { return kPointerSize; }

InterfaceModel::InterfaceModel(base::Arena *arena, const String *name, const String *full_name)
: Model(name, full_name, kVal, kInterface)
, arena_(arena)
, methods_(arena)
, members_(arena) {
}

Handle *InterfaceModel::InsertMethod(Function *fun) {
    assert(fun->decoration() == Function::kAbstract && !fun->entry() && fun->blocks().empty()
           && "Function must be a prototype");
    auto name = fun->name()->ToSlice();
    auto iter = members_.find(name);
    assert(iter == members_.end() || !iter->second->IsMethod());
    auto handle = Handle::Method(arena_, this, fun->name(), methods_size());
    members_[name] = handle;
    methods_.push_back({
        .fun = fun,
        .access = kPublic,
        .offset = static_cast<int>(methods_size()),
    });
    return handle;
}

std::tuple<Model::Method, bool> InterfaceModel::FindMethod(std::string_view name) const {
    if (auto iter = members_.find(name); iter == members_.end()) {
        return std::make_tuple(Method{}, false);
    } else {
        return std::make_tuple(methods_[iter->second->offset()], true);
    }
}

size_t InterfaceModel::ReferenceSizeInBytes() const { return kPointerSize; }

ArrayModel::ArrayModel(base::Arena *arena, const String *name, const String *full_name,
                       int dimension_count, const Type element_type)
: Model(name, full_name, kRef, kArray)
, element_type_(element_type)
, dimension_count_(dimension_count) {
}

std::string ArrayModel::ToString(int dimension_count, const Type element_type) {
    std::string full_name(element_type.ToString());
    for (int i = 0; i < dimension_count; i++) {
        full_name.append("[]");
    }
    return full_name;
}

size_t ArrayModel::ReferenceSizeInBytes() const { return kPointerSize; }

StructureModel::StructureModel(base::Arena *arena, const String *name, const String *full_name, Declaration declaration,
                               Module *owns, StructureModel *base_of)
: Model(name, full_name, declaration == kClass ? kRef : kVal, declaration)
, owns_(owns)
, arena_(arena)
, base_of_(base_of)
, implements_(arena)
, fields_(arena)
, members_(arena)
, methods_(arena) {
}

Handle *StructureModel::InsertField(const Field &field) {
    auto iter = members_.find(field.name->ToSlice());
    assert(iter == members_.end() || !iter->second->IsField());
    //Index index = {.offset = fields_.size(), .field_or_method = true};
    auto handle = Handle::Field(arena_, this, field.name, fields_size());
    members_[field.name->ToSlice()] = handle;
    fields_.push_back(field);
    return handle;
}

Handle *StructureModel::InsertMethod(const Method &method) {
    auto name = method.fun->name()->ToSlice();
    auto iter = members_.find(name);
    assert(iter == members_.end() || !iter->second->IsMethod());
    auto handle = Handle::Method(arena_, this, method.fun->name(), methods_size());
    members_[name] = handle;
    methods_.push_back(method);
    return handle;
}

std::tuple<Model::Field, bool> StructureModel::FindField(std::string_view name) const {
    auto iter = members_.find(name);
    if (iter == members_.end() || !iter->second->IsField()) {
        return std::make_tuple(Field{}, false);
    }
    return std::make_tuple(fields_[iter->second->offset()], true);
}

std::tuple<Model::Method, bool> StructureModel::FindMethod(std::string_view name) const {
    for (auto model = this; model != nullptr; model = model->base_of()) {
        auto iter = model->members_.find(name);
        if (iter != model->members_.end() && iter->second->IsMethod()) {
            return std::make_tuple(model->methods_[iter->second->offset()], true);
        }
    }
    return std::make_tuple(Method{}, false);
}

Handle *StructureModel::FindMemberOrNull(std::string_view name) const {
    if (auto iter = members_.find(name); iter != members_.end()) {
        return iter->second;
    } else {
        return nullptr;
    }
}

size_t StructureModel::ReferenceSizeInBytes() const {
    if (declaration() == kClass) {
        return kPointerSize;
    }
    UNREACHABLE();
    return 0;
}

} // namespace ir

} // namespace yalx
