#include "ir/metadata.h"
#include "ir/node.h"

namespace yalx {

namespace ir {

Model::Model(const String *name): name_(DCHECK_NOTNULL(name)) {}

std::tuple<Model::Method, bool> Model::FindMethod(std::string_view name) const {
    return std::make_tuple(Method{}, false);
}

std::tuple<Model::Field, bool> Model::FindField(std::string_view name) const {
    return std::make_tuple(Field{}, false);
}

InterfaceModel::InterfaceModel(base::Arena *arena, const String *name)
    : Model(name)
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

StructureModel::StructureModel(base::Arena *arena, const String *name, Declaration constraint, Module *owns,
                               StructureModel *base_of)
    : Model(name)
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
