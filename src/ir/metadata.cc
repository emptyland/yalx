#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/utils.h"
#include "compiler/constants.h"
#include "runtime/object/any.h"
#include "base/io.h"
#include <stack>

namespace yalx {

namespace ir {

Model::Model(const String *name, const String *full_name, Constraint constraint, Declaration declaration)
: name_(DCHECK_NOTNULL(name))
, full_name_(DCHECK_NOTNULL(full_name))
, constraint_(constraint)
, declaration_(declaration) {}

std::optional<Model::Method> Model::FindMethod(std::string_view name) const {
    return std::nullopt;
}

std::optional<Model::Field> Model::FindField(std::string_view name) const {
    return std::nullopt;
}

Model::Member Model::GetMember(const Handle *handle) const {
    return static_cast<const Field *>(nullptr);
}

void Model::PrintTo(int indent, base::PrintingWriter *printer) const {
    printer->Indent(indent)->Writeln(full_name()->ToSlice());
}

Handle *Model::FindMemberOrNull(std::string_view name) const {
    return nullptr;
}

bool Model::IsBaseOf(const Model *base) const {
    return base == this || base->full_name()->Equal(cpl::kAnyClassFullName);
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
size_t PrototypeModel::PlacementSizeInBytes() const { return kPointerSize; }

void PrototypeModel::PrintTo(int indent, base::PrintingWriter *printer) const {
    printer
    ->Indent(indent)
    ->Write(ToString(&params_[0], params_size(), vargs(), &return_types_[0], return_types_size()));
}

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
        .in_vtab = 0,
        .in_itab = 0,
    });
    return handle;
}

Model::Member InterfaceModel::GetMember(const Handle *handle) const {
    assert(handle->owns() == this);
    assert(handle->offset() >= 0);
    assert(handle->offset() < methods_size());
    return &methods_[handle->offset()];
}

std::optional<Model::Method> InterfaceModel::FindMethod(std::string_view name) const {
    if (auto iter = members_.find(name); iter == members_.end()) {
        return std::nullopt;
    } else {
        return std::optional<Model::Method>{methods_[iter->second->offset()]};
    }
}

// ownership: yalx_any_t *;
// vtab     : yalx_fun_t **;
size_t InterfaceModel::ReferenceSizeInBytes() const { return kPointerSize * 2; } // TODO:
size_t InterfaceModel::PlacementSizeInBytes() const { return kPointerSize * 2; } // TODO:

void InterfaceModel::PrintTo(int indent, base::PrintingWriter *printer) const {
    printer->Indent(indent)->Println("interface %s @%s {", name()->data(), full_name()->data());
    PrintingContext ctx(indent + 1);
    for (auto method : methods()) {
        method.fun->PrintTo(&ctx, printer);
    }
    printer->Indent(indent)->Println("} // %s", full_name()->data());
}

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
size_t ArrayModel::PlacementSizeInBytes() const { UNREACHABLE(); }

StructureModel::StructureModel(base::Arena *arena, const String *name, const String *full_name, Declaration declaration,
                               Module *owns, StructureModel *base_of)
: Model(name, full_name, declaration == kClass ? kRef : kVal, declaration)
, owns_(owns)
, arena_(arena)
, base_of_(base_of)
, interfaces_(arena)
, fields_(arena)
, members_(arena)
, methods_(arena)
, vtab_(arena)
, itab_(arena) {
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

Model::Member StructureModel::GetMember(const Handle *handle) const {
    const StructureModel *model = nullptr;
    for (auto it = this; it != nullptr; it = it->base_of()) {
        if (handle->owns() == it) {
            model = it;
            break;
        }
    }
    assert(model);
    assert(handle->offset() >= 0);
    if (handle->IsMethod()) {
        assert(handle->offset() < model->methods_size());
        return &model->methods_[handle->offset()];
    } else {
        assert(handle->offset() < model->fields_size());
        return &model->fields_[handle->offset()];
    }
}

std::optional<Model::Field> StructureModel::FindField(std::string_view name) const {
    auto iter = members_.find(name);
    if (iter == members_.end() || !iter->second->IsField()) {
        return std::nullopt;
    }
    return std::optional<Model::Field>{fields_[iter->second->offset()]};
}

std::optional<Model::Method> StructureModel::FindMethod(std::string_view name) const {
    for (auto model = this; model != nullptr; model = model->base_of()) {
        auto iter = model->members_.find(name);
        if (iter != model->members_.end() && iter->second->IsMethod()) {
            return std::optional<Model::Method>{model->methods_[iter->second->offset()]};
        }
    }
    return std::nullopt;
}

Handle *StructureModel::FindMemberOrNull(std::string_view name) const {
    for (auto model = this; model != nullptr; model = model->base_of()) {
        if (auto iter = model->members_.find(name); iter != model->members_.end()) {
            return iter->second;
        }
    }
    return nullptr;
}

size_t StructureModel::ReferenceSizeInBytes() const { return kPointerSize; }
size_t StructureModel::PlacementSizeInBytes() const {
    assert(placement_size_in_bytes_ > 0);
    return placement_size_in_bytes_;
}

void StructureModel::InstallVirtualTables(bool force) {
    if (declaration() == kClass && !base_of()) {
        for (auto [name, handle] : members_) {
            if (!handle->IsMethod()) {
                continue;
            }
            auto method = &methods_[handle->offset()];
            if (constructor_ == method->fun) {
                continue;
            }
            method->in_vtab = true;
            method->id_vtab = static_cast<int>(vtab_.size());
            vtab_.push_back(handle);
        }
        return;
    }
    //size_t offset = 0;
    for (auto iface : interfaces()) {
        for (auto method : iface->methods()) {
            auto had = DCHECK_NOTNULL(FindMemberOrNull(method.fun->name()->ToSlice()));
            assert(had->IsMethod());
            itab_.push_back(had);
        }
        
        //offset += iface->methods_size();
    }
    
    std::vector<Handle *> incoming_vtab;
    size_t max_vtab_len = 0;
    for (auto base = base_of(); base != nullptr; base = base->base_of()) {
        max_vtab_len = std::max(max_vtab_len, base->vtab_.size());
        incoming_vtab.resize(max_vtab_len);
        for (size_t i = 0; i < max_vtab_len; i++) {
            incoming_vtab[i] = base->vtab_[i];
        }
    }

    bool should_override_vtab = false;
    for (auto [name, handle] : members_) {
        if (!handle->IsMethod()) {
            continue;
        }
        auto method = &methods_[handle->offset()];
        if (method->fun->decoration() == Function::kOverride && !method->in_vtab) {
            for (auto base = base_of(); base != nullptr; base = base->base_of()) {
                if (auto vf = base->FindMemberOrNull(handle->name()->ToSlice()); vf != nullptr && vf->IsMethod()) {
                    auto vm = &base->methods_[vf->offset()];
                    if (vm->in_vtab) {
                        method->in_vtab = true;
                        method->id_vtab = vm->id_vtab;
                        assert(method->id_vtab < incoming_vtab.size());
                        incoming_vtab[vm->id_vtab] = handle;
                    } else {
                        vm->in_vtab = true;
                        vm->id_vtab = static_cast<int>(base->vtab_.size());
                        base->vtab_.push_back(vf);
                        
                        assert(incoming_vtab.size() == vm->id_vtab);
                        method->in_vtab = true;
                        method->id_vtab = vm->id_vtab;
                        incoming_vtab.push_back(handle);
                    }
                    should_override_vtab = true;
                    break;
                }
            }
        }
    }
    
    if (should_override_vtab) {
        for (auto had : incoming_vtab) {
            vtab_.push_back(had);
        }
    }
}

int64_t StructureModel::UpdatePlacementSizeInBytes() {
    ptrdiff_t offset = 0;
    if (!base_of()) {
        offset = sizeof(struct yalx_value_any);
    } else if (base_of()->placement_size_in_bytes_ > 0) {
        offset = base_of()->PlacementSizeInBytes();
    } else {
        offset = base_of()->UpdatePlacementSizeInBytes();
    }
    assert(offset > 0);
    assert(offset % kPointerSize == 0);

    for (const auto &field : fields()) {
        //offset += field.type.bytes();
        size_t incoming_size = 0;
        switch (field.type.kind()) {
            case Type::kValue:
                incoming_size = field.type.model()->PlacementSizeInBytes();
                break;
            case Type::kReference:
                incoming_size = field.type.model()->ReferenceSizeInBytes();
                break;
            default:
                incoming_size = field.type.bytes();
                break;
        }
        
        if (((offset + incoming_size) & 0x1u) == 0u) { // check did offset alignment of 2
            offset += incoming_size;
        } else {
            offset = RoundUp(offset, 2) + incoming_size;
        }
    }
    // size must be aligment of kPointerSize
    placement_size_in_bytes_ = RoundUp(offset, kPointerSize);
}

bool StructureModel::IsBaseOf(const Model *base) const {
    for (auto it = this; it != nullptr; it = it->base_of()) {
        if (it == base) {
            return true;
        }
    }
    return false;
}

void StructureModel::PrintTo(int indent, base::PrintingWriter *printer) const {
    printer
    ->Indent(indent)
    ->Write(declaration() == kClass ? "class" : "struct")
    ->Println(" %s @%s {", name()->data(), full_name()->data());
    
    if (base_of()) {
        printer->Indent(indent + 1)->Println("[base_of: @%s]", base_of()->full_name()->data());
    }
    if (!vtab_.empty()) {
        printer->Indent(indent + 1)->Println("vtab[%zd] = {", vtab_.size());
        //int i = 0;
        for (auto item : vtab_) {
            if (item) {
                printer->Indent(indent + 2)->Println("%s::%s", item->owns()->name()->data(), item->name()->data());
            } else {
                printer->Indent(indent + 2)->Writeln("nil");
            }
            
        }
        printer->Indent(indent + 1)->Writeln("}");
    }
    
    if (!itab_.empty()) {
        printer->Indent(indent + 1)->Println("itab[%zd] = {", itab_.size());
        size_t offset = 0;
        for (auto iface : interfaces()) {
            printer
            ->Indent(indent + 1)
            ->Write(iface->full_name()->ToSlice())
            ->Writeln(":");
            for (size_t i = offset; i < offset + iface->methods_size(); i++) {
                auto const had = itab_[i];
                printer->Indent(indent + 2)->Println("%s::%s", had->owns()->name()->data(), had->name()->data());
            }
            offset += iface->methods_size();
        }
        printer->Indent(indent + 1)->Writeln("}");
    }
    
    for (const auto &field : fields()) {
        printer
        ->Indent(indent + 1)
        ->Write(field.name->ToSlice())
        ->Write(": ");
        field.type.PrintTo(printer);
        printer->Write("\n");
    }
    
    printer->Indent(indent)->Println("} // %s", full_name()->data());
}

bool StructureModel::In_itab(Handle *handle) const {
    for (auto it : itab_) {
        if (it == handle) {
            return true;
        }
    }
    return false;
}

bool StructureModel::In_vtab(Handle *handle) const {
    for (auto it : vtab_) {
        if (it == handle) {
            return true;
        }
    }
    return false;
}

} // namespace ir

} // namespace yalx
