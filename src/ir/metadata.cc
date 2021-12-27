#include "ir/metadata.h"
#include "ir/node.h"

namespace yalx {

namespace ir {

Model::Model(const String *name, const String *full_name)
: name_(DCHECK_NOTNULL(name))
, full_name_(DCHECK_NOTNULL(full_name)){}

std::tuple<Model::Method, bool> Model::FindMethod(std::string_view name) const {
    return std::make_tuple(Method{}, false);
}

std::tuple<Model::Field, bool> Model::FindField(std::string_view name) const {
    return std::make_tuple(Field{}, false);
}

PrototypeModel::PrototypeModel(base::Arena *arena, const String *name, bool vargs)
: Model(name, name)
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
: Model(name, full_name)
, methods_(arena) {
}

void InterfaceModel::InsertMethod(Function *fun) {
    assert(!fun->entry() && fun->blocks().empty() && "Function must be a prototype");
    assert(methods_.find(fun->name()->ToSlice()) == methods_.end() && "Duplicated method name");
    methods_[fun->name()->ToSlice()] = {
        .fun = fun,
        .access = kPublic,
        .offset = static_cast<int>(methods_.size()),
        .is_native = false,
        .is_override = false,
    };
}

std::tuple<Model::Method, bool> InterfaceModel::FindMethod(std::string_view name) const {
    if (auto iter = methods_.find(name); iter == methods_.end()) {
        return std::make_tuple(Method{}, false);
    } else {
        return std::make_tuple(iter->second, true);
    }
}

size_t InterfaceModel::ReferenceSizeInBytes() const { return kPointerSize; }

ArrayModel::ArrayModel(base::Arena *arena, const String *name, const String *full_name,
                       int dimension_count, const Type element_type)
: Model(name, full_name)
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

StructureModel::StructureModel(base::Arena *arena, const String *name, const String *full_name, Declaration constraint,
                               Module *owns, StructureModel *base_of)
: Model(name, full_name)
, declaration_(constraint)
, owns_(owns)
, base_of_(base_of)
, implements_(arena)
, fields_(arena)
, members_(arena)
, methods_(arena) {
}

void StructureModel::InsertField(const Field &field) {
    auto iter = members_.find(field.name->ToSlice());
    assert(iter == members_.end() || !iter->second.field_or_method);
    Index index = {.offset = fields_.size(), .field_or_method = true};
    members_[field.name->ToSlice()] = index;
    fields_.push_back(field);
}

void StructureModel::InsertMethod(const Method &method) {
    auto name = method.fun->name()->ToSlice();
    auto iter = members_.find(name);
    assert(iter == members_.end() || iter->second.field_or_method);
    Index index = {.offset = fields_.size(), .field_or_method = false};
    members_[name] = index;
    methods_.push_back(method);
}

std::tuple<Model::Field, bool> StructureModel::FindField(std::string_view name) const {
    auto iter = members_.find(name);
    if (iter == members_.end() || !iter->second.field_or_method) {
        return std::make_tuple(Field{}, false);
    }
    return std::make_tuple(fields_[iter->second.offset], true);
}

std::tuple<Model::Method, bool> StructureModel::FindMethod(std::string_view name) const {
    auto iter = members_.find(name);
    if (iter == members_.end() || iter->second.field_or_method) {
        return std::make_tuple(Method{}, false);
    }
    return std::make_tuple(methods_[iter->second.offset], true);
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
