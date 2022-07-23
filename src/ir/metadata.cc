#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/utils.h"
#include "compiler/constants.h"
#include "runtime/object/arrays.h"
#include "runtime/object/any.h"
#include "base/io.h"
#include <stack>

namespace yalx {

namespace ir {

class OriginalShadow {
public:
    OriginalShadow(base::Arena *arena)
        : arena_(arena)
        , members_(arena)
        , fields_(arena)
        , methods_(arena) {}
    
    Handle *FindMemberOrNull(ArrayModel *owns, std::string_view name);
    
    Model::Member GetMember(const Handle *handle) const {
        //DCHECK(handle->owns()->declaration() == Model::kArray);
        if (handle->IsField()) {
            return &fields_[handle->offset()];
        } else {
            return &methods_[handle->offset()];
        }
    }
    
protected:
    void Put(Model::Field &&field) {
        fields_.emplace_back(field);
        PutHandleTag(field.name->ToSlice(), fields_.size() - 1, false/*is_method*/);
    }
    
    void Put(const std::string &name, Model::Method &&method) {
        methods_.emplace_back(method);
        if (name.empty()) {
            PutHandleTag(method.fun->name()->ToSlice(), methods_.size() - 1, true/*is_method*/);
        } else {
            PutHandleTag(arena()->Duplicate(name.data(), name.size()), methods_.size() - 1, true/*is_method*/);
        }
    }
    
    void PutHandleTag(std::string_view name, size_t index, bool is_method) {
        members_[name] = reinterpret_cast<Handle *>((index << 1) | (is_method ? 1 : 0));
    }
    
    std::tuple<size_t, bool> ParseHandleTag(Handle *handle) {
        auto tag = reinterpret_cast<size_t>(handle);
        return std::make_tuple(tag >> 1, tag & 0x1);
    }
    
    base::Arena *arena() const { return arena_; }
private:
    base::Arena *const arena_;
    base::ArenaMap<std::string_view, Handle *> members_;
    base::ArenaVector<Model::Field> fields_;
    base::ArenaVector<Model::Method> methods_;
}; // class PrimitiveShadow

Handle *OriginalShadow::FindMemberOrNull(ArrayModel *owns, std::string_view name) {
    std::string full_name(owns->full_name()->ToString());
    full_name.append("::").append(name.data(), name.size());
    if (auto iter = members_.find(full_name); iter != members_.end()) {
        return iter->second;
    }
    if (auto iter = members_.find(name); iter != members_.end()) {
        auto [index, is_method] = ParseHandleTag(iter->second);
        
        Handle *rs = nullptr;
        if (is_method) {
            const auto &method = methods_[index];
            rs = Handle::Method(arena_, owns, method.fun->name(), index);
        } else {
            const auto &field = fields_[index];
            rs = Handle::Field(arena_, owns, field.name, index);
        }
        auto dup = static_cast<char *>(arena()->Allocate(full_name.size() + 1));
        ::strncpy(dup, full_name.data(), full_name.size());
        members_[dup] = rs;
        return rs;
    }
    return nullptr;
}

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

size_t Model::RefsMarkSize() const { return 0; }

Model::RefMark Model::RefsMark(int i) const { UNREACHABLE(); }

bool Model::IsBaseOf(const Model *base) const {
    return base == this || base->full_name()->Equal(cpl::kAnyClassFullName);
}

class PrimitiveShadow {
public:
    PrimitiveShadow(base::Arena *const arena): arena_(arena) {}
    
    static uintptr_t Uniquely() {
        static int dummy;
        return reinterpret_cast<uintptr_t>(&dummy);
    }
    
    void Init();
    
    PrimitiveModel *kU8 = nullptr;
    PrimitiveModel *kI8 = nullptr;
    PrimitiveModel *kU16 = nullptr;
    PrimitiveModel *kI16 = nullptr;
    PrimitiveModel *kU32 = nullptr;
    PrimitiveModel *kI32 = nullptr;
    PrimitiveModel *kU64 = nullptr;
    PrimitiveModel *kI64 = nullptr;
    PrimitiveModel *kF32 = nullptr;
    PrimitiveModel *kF64 = nullptr;
private:
    base::Arena *const arena_;
}; // class PrimitiveShadow

void PrimitiveShadow::Init() {
    auto base_pkg = DCHECK_NOTNULL(arena_->Lazy<PackageContext>()->FindOrNull(cpl::kLangPackageFullName));
    
    kU8 = new (arena_) PrimitiveModel(arena_, String::New(arena_, "u8"), Types::UInt8);
    kU8->Put(arena_, "toString", {
        .fun = DCHECK_NOTNULL(base_pkg->FindFunOrNull("u8ToString")),
        .access = kPublic,
        .in_vtab = 0,
        .id_vtab = 0,
        .in_itab = 0,
    });
    
    kI8 = new (arena_) PrimitiveModel(arena_, String::New(arena_, "i8"), Types::Int8);
    kI8->Put(arena_, "toString", {
        .fun = DCHECK_NOTNULL(base_pkg->FindFunOrNull("i8ToString")),
        .access = kPublic,
        .in_vtab = 0,
        .id_vtab = 0,
        .in_itab = 0,
    });
    
    kU16 = new (arena_) PrimitiveModel(arena_, String::New(arena_, "u16"), Types::UInt16);
    kU16->Put(arena_, "toString", {
        .fun = DCHECK_NOTNULL(base_pkg->FindFunOrNull("u16ToString")),
        .access = kPublic,
        .in_vtab = 0,
        .id_vtab = 0,
        .in_itab = 0,
    });
    
    kI16 = new (arena_) PrimitiveModel(arena_, String::New(arena_, "i16"), Types::Int16);
    kI16->Put(arena_, "toString", {
        .fun = DCHECK_NOTNULL(base_pkg->FindFunOrNull("i16ToString")),
        .access = kPublic,
        .in_vtab = 0,
        .id_vtab = 0,
        .in_itab = 0,
    });
    
    kU32 = new (arena_) PrimitiveModel(arena_, String::New(arena_, "u32"), Types::UInt32);
    kU32->Put(arena_, "toString", {
        .fun = DCHECK_NOTNULL(base_pkg->FindFunOrNull("u32ToString")),
        .access = kPublic,
        .in_vtab = 0,
        .id_vtab = 0,
        .in_itab = 0,
    });
    
    kI32 = new (arena_) PrimitiveModel(arena_, String::New(arena_, "i32"), Types::Int32);
    kI32->Put(arena_, "toString", {
        .fun = DCHECK_NOTNULL(base_pkg->FindFunOrNull("i32ToString")),
        .access = kPublic,
        .in_vtab = 0,
        .id_vtab = 0,
        .in_itab = 0,
    });
    
    kU64 = new (arena_) PrimitiveModel(arena_, String::New(arena_, "u64"), Types::UInt64);
    kU64->Put(arena_, "toString", {
        .fun = DCHECK_NOTNULL(base_pkg->FindFunOrNull("u64ToString")),
        .access = kPublic,
        .in_vtab = 0,
        .id_vtab = 0,
        .in_itab = 0,
    });
    
    kI64 = new (arena_) PrimitiveModel(arena_, String::New(arena_, "i64"), Types::Int64);
    kI64->Put(arena_, "toString", {
        .fun = DCHECK_NOTNULL(base_pkg->FindFunOrNull("i64ToString")),
        .access = kPublic,
        .in_vtab = 0,
        .id_vtab = 0,
        .in_itab = 0,
    });
    
    kF32 = new (arena_) PrimitiveModel(arena_, String::New(arena_, "f32"), Types::Float32);
    kF32->Put(arena_, "toString", {
        .fun = DCHECK_NOTNULL(base_pkg->FindFunOrNull("f32ToString")),
        .access = kPublic,
        .in_vtab = 0,
        .id_vtab = 0,
        .in_itab = 0,
    });
    
    kF64 = new (arena_) PrimitiveModel(arena_, String::New(arena_, "f64"), Types::Float64);
    kF64->Put(arena_, "toString", {
        .fun = DCHECK_NOTNULL(base_pkg->FindFunOrNull("f64ToString")),
        .access = kPublic,
        .in_vtab = 0,
        .id_vtab = 0,
        .in_itab = 0,
    });
}

PrimitiveModel *PrimitiveModel::Get(const Type ty, base::Arena *arena) {
    auto shadow = arena->Lazy<PrimitiveShadow>();
    switch (ty.kind()) {
        case Type::kWord8:
        case Type::kUInt8:
            return shadow->kU8;
        case Type::kInt8:
            return shadow->kI8;
        case Type::kWord16:
        case Type::kUInt16:
            return shadow->kU16;
        case Type::kInt16:
            return shadow->kI16;
        case Type::kUInt32:
            return shadow->kU32;
        case Type::kInt32:
            return shadow->kI32;
        case Type::kUInt64:
            return shadow->kU64;
        case Type::kInt64:
            return shadow->kI64;
        case Type::kFloat32:
            return shadow->kF32;
        case Type::kFloat64:
            return shadow->kF64;
        default:
            break;
    }
    return nullptr;
}

Model::Member PrimitiveModel::GetMember(const Handle *handle) const {
    DCHECK(handle->owns() == this);
    DCHECK(handle->offset() >= 0 && handle->offset() < methods_.size());
    return &methods_[handle->offset()];
}

Handle *PrimitiveModel::FindMemberOrNull(std::string_view name) const {
    if (auto iter = members_.find(name); iter != members_.end()) {
        return iter->second;
    }
    return nullptr;
}

size_t PrimitiveModel::ReferenceSizeInBytes() const  { return type().bytes(); }
size_t PrimitiveModel::PlacementSizeInBytes() const { return type().bytes(); }

PrimitiveModel::PrimitiveModel(base::Arena *arena, const String *name, const Type ty)
: Model(name, name, kVal, kPrimitive)
, type_(ty)
, members_(arena)
, methods_(arena) {
}

void PrimitiveModel::Put(base::Arena *arena, std::string_view name, Method &&method) {
    auto handle = Handle::Method(arena, this, method.fun->name(), methods_.size());
    members_[name] = handle;
    methods_.emplace_back(method);
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
    DCHECK(fun->decoration() == Function::kAbstract && !fun->entry() && fun->blocks().empty()
           && "Function must be a prototype");
    auto name = fun->name()->ToSlice();
    auto iter = members_.find(name);
    DCHECK(iter == members_.end() || !iter->second->IsMethod());
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
    DCHECK(handle->owns() == this);
    DCHECK(handle->offset() >= 0);
    DCHECK(handle->offset() < methods_size());
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

class ArrayShadow final : public OriginalShadow {
public:
    ArrayShadow(base::Arena *arena): OriginalShadow(arena) {}
    
    static uintptr_t Uniquely() {
        static int dummy;
        return reinterpret_cast<uintptr_t>(&dummy);
    }
    
    void Init();
}; // class ArrayShadow

void ArrayShadow::Init() {
    Put({
        .name = String::New(arena(), "size"),
        .access = kPublic,
        .offset = offsetof(yalx_value_array, len),
        .type = Types::Int32,
        .enum_value = 0,
    });
    
    Put({
        .name = String::New(arena(), "rank"),
        .access = kPublic,
        .offset = offsetof(yalx_value_multi_dims_array, rank),
        .type = Types::Int32,
        .enum_value = 0,
    });
    
    auto base_pkg = DCHECK_NOTNULL(arena()->Lazy<PackageContext>()->FindOrNull(cpl::kLangPackageFullName));
    auto fun = DCHECK_NOTNULL(base_pkg->FindFunOrNull("multiDimsArrayGetLength"));
    
    //base_pkg->FindFunOrNull("");
    Put("getLength",
        Model::Method{
        .fun = fun,
        .access = kPublic,
        .in_vtab = 0,
        .id_vtab = 0,
        .in_itab = 0,
    });
}

ArrayModel::ArrayModel(base::Arena *arena, const String *name, const String *full_name,
                       int dimension_count, const Type element_type)
: Model(name, full_name, kRef, kArray)
, arena_(arena)
, element_type_(element_type)
, dimension_count_(dimension_count) {
}

std::string ArrayModel::ToString(int dimension_count, const Type element_type) {
    DCHECK(dimension_count > 0);
    std::string full_name;
    
    full_name.append("[");
    for (int i = 0; i < dimension_count - 1; i++) {
        full_name.append(",");
    }
    full_name.append("]");
    
    auto ty = element_type;
    while (ty.model() && ty.model()->declaration() == kArray) {
        auto ar = down_cast<ArrayModel>(ty.model());
        full_name.append("[");
        for (int i = 0; i < ar->dimension_count() - 1; i++) {
            full_name.append(",");
        }
        full_name.append("]");
        
        ty = ar->element_type();
    }
    
    return std::string(ty.ToString()).append(full_name);
}

Model::Member ArrayModel::GetMember(const Handle *handle) const {
    DCHECK(handle->owns()->declaration() == Model::kArray);
    auto shadow = arena_->Lazy<ArrayShadow>();
    return shadow->GetMember(handle);
}

Handle *ArrayModel::FindMemberOrNull(std::string_view name) const {
    auto shadow = arena_->Lazy<ArrayShadow>();
    return shadow->FindMemberOrNull(const_cast<ArrayModel *>(this), name);
}

size_t ArrayModel::ReferenceSizeInBytes() const { return kPointerSize; }
size_t ArrayModel::PlacementSizeInBytes() const { UNREACHABLE(); }

ChannelModel::ChannelModel(base::Arena *arena, const String *name, const String *full_name, const Type element_type)
: Model(name, full_name, kRef, kChannel)
, element_type_(element_type)
, ability_(kInbility | kOutbility) {
    
}

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
, itab_(arena)
, refs_marks_(arena) {
}

Handle *StructureModel::InsertField(const Field &field) {
    auto iter = members_.find(field.name->ToSlice());
    DCHECK(iter == members_.end() || !iter->second->IsField());
    //Index index = {.offset = fields_.size(), .field_or_method = true};
    auto handle = Handle::Field(arena_, this, field.name, fields_size());
    members_[field.name->ToSlice()] = handle;
    fields_.push_back(field);
    return handle;
}

Handle *StructureModel::InsertMethod(const Method &method) {
    auto name = method.fun->name()->ToSlice();
    auto iter = members_.find(name);
    DCHECK(iter == members_.end() || !iter->second->IsMethod());
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
    DCHECK(model);
    DCHECK(handle->offset() >= 0);
    if (handle->IsMethod()) {
        DCHECK(handle->offset() < model->methods_size());
        return &model->methods_[handle->offset()];
    } else {
        DCHECK(handle->offset() < model->fields_size());
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

size_t StructureModel::LazyPlacementSizeInBytes() {
    if (placement_size_in_bytes_ < 0) {
        return UpdatePlacementSizeInBytes();
    } else {
        return PlacementSizeInBytes();
    }
}

size_t StructureModel::ReferenceSizeInBytes() const { return kPointerSize; }

size_t StructureModel::PlacementSizeInBytes() const {
    DCHECK(placement_size_in_bytes_ > 0);
    return placement_size_in_bytes_;
    //return const_cast<StructureModel *>(this)->LazyPlacementSizeInBytes();
}

size_t StructureModel::RefsMarkSize() const { return refs_marks_size(); }

Model::RefMark StructureModel::RefsMark(int i) const { return refs_mark(i); }

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
            DCHECK(had->IsMethod());
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
                        DCHECK(method->id_vtab < incoming_vtab.size());
                        incoming_vtab[vm->id_vtab] = handle;
                    } else {
                        vm->in_vtab = true;
                        vm->id_vtab = static_cast<int>(base->vtab_.size());
                        base->vtab_.push_back(vf);
                        
                        DCHECK(incoming_vtab.size() == vm->id_vtab);
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
    
    // size must be aligment of kPointerSize
    switch (declaration()) {
        case Model::kStruct:
        case Model::kClass:
            placement_size_in_bytes_ = CalculateContinuousPlacementSize();
            break;
            
        case Model::kEnum:
            placement_size_in_bytes_ = CalculateCompactPlacementSize();
            break;
            
        default:
            UNREACHABLE();
            break;
    }
    return placement_size_in_bytes_;
}

int64_t StructureModel::CalculateCompactPlacementSize() {
    //ptrdiff_t offset = 0;
    size_t max_alignment = 0;
    int ref_ty_count = 0;
    int unit_ty_count = 0;
    for (auto &field : fields_) {
        switch (field.type.kind()) {
            case Type::kValue:
                DCHECK(!field.type.IsPointer());
                max_alignment = std::max(max_alignment, static_cast<size_t>(kPointerSize));
                break;
            case Type::kString:
            case Type::kReference:
                max_alignment = std::max(max_alignment, static_cast<size_t>(kPointerSize));
                ref_ty_count++;
                break;
            case Type::kTuple: {
                auto [_, a] = CalculateTypesPlacementSize(field.type.tuple(), field.type.bits());
                max_alignment = std::max(max_alignment, a);
            } break;
            case Type::kVoid:
                unit_ty_count++;
                break;
            default:
                max_alignment = std::max(max_alignment, static_cast<size_t>(field.type.bytes()));
                break;
        }
    }
    ptrdiff_t base_offset = 2;
    if (ref_ty_count == 1 && unit_ty_count == 1 && fields_size() == 2) {
        base_offset = 0;
    }
    
    refs_marks_.clear();
    size_t max_size = base_offset;
    for (auto &field : fields_) {
        size_t size = 0;
        switch (field.type.kind()) {
            case Type::kValue:
                DCHECK(!field.type.IsPointer());
                field.offset = RoundUp(base_offset, max_alignment);
                size = field.offset + down_cast<StructureModel>(field.type.model())->LazyPlacementSizeInBytes();
                AnalyzeRefsMark(field.type, field.offset);
                break;
            case Type::kString:
            case Type::kReference:
                field.offset = RoundUp(base_offset, max_alignment);
                size = field.offset + field.type.model()->ReferenceSizeInBytes();
                refs_marks_.push_back({field.type, field.offset});
                break;
            case Type::kTuple: {
                auto [s, _] = CalculateTypesPlacementSize(field.type.tuple(), field.type.bits());
                field.offset = RoundUp(base_offset, max_alignment);
                size = field.offset + s;
            } break;
            case Type::kVoid:
                field.offset = base_offset;
                break;
            default:
                if (field.name->Equal(kEnumCodeName)) {
                    field.offset = 0;
                } else {
                    field.offset = RoundUp(base_offset, max_alignment);
                }
                size = field.offset + field.type.bytes();
                break;
        }
        max_size = std::max(size, max_size);
    }

    return RoundUp(max_size, kPointerSize);
}

std::tuple<size_t, size_t> StructureModel::CalculateTypesPlacementSize(const Type *types, int n) {
    size_t size = 0, aligment = 0;
    for (int i = 0; i < n; i++) {
        auto ty = types[i];
        switch (ty.kind()) {
            case Type::kValue:
                aligment = kPointerSize;
                size = RoundUp(size, kPointerSize);
                AnalyzeRefsMark(ty, size);
                size += ty.PlacementSizeInBytes();
                break;

            case Type::kReference:
            case Type::kString:
                aligment = kPointerSize;
                size = RoundUp(size, kPointerSize);
                refs_marks_.push_back({ty, static_cast<ptrdiff_t>(size)});
                size += kPointerSize;
                break;

            case Type::kTuple: {
                auto [s, a] = CalculateTypesPlacementSize(ty.tuple(), ty.bits());
                size = RoundUp(size, a);
                size += s;
            } break;

            case Type::kVoid:
                break;

            default:
                aligment = std::min(kPointerSize, ty.bytes());
                size = RoundUp(size, ty.bytes());
                size += ty.bytes();
                break;
        }
    }

    return std::make_tuple(size, aligment);
}

void StructureModel::AnalyzeRefsMark(Type ty, ptrdiff_t offset) {
    DCHECK(ty.kind() == Type::kValue);
    DCHECK(!ty.IsPointer());
    
    switch (ty.model()->declaration()) {
        case Model::kEnum: {
            auto def = down_cast<StructureModel>(ty.model());
            if (ty.IsCompactEnum() || !def->refs_marks_.empty()) {
                refs_marks_.push_back({ty, offset});
            }
        } break;
            
        case Model::kStruct: {
            auto def = down_cast<StructureModel>(ty.model());
            for (auto mark : def->refs_marks_) {
                refs_marks_.push_back({mark.ty, offset + mark.offset});
            }
        } break;
            
        case Model::kInterface:
            DCHECK(!"TODO"); // TODO:
            break;
            
        default:
            UNREACHABLE();
            break;
    }
}

int64_t StructureModel::CalculateContinuousPlacementSize() {
    ptrdiff_t offset = 0;
    if (!base_of()) {
        offset = sizeof(struct yalx_value_any);
    } else if (base_of()->placement_size_in_bytes_ > 0) {
        offset = base_of()->PlacementSizeInBytes();
    } else {
        offset = base_of()->UpdatePlacementSizeInBytes();
    }
    DCHECK(offset > 0);
    DCHECK(offset % kPointerSize == 0);

    refs_marks_.clear();
    for (auto &field : fields_) {
        size_t incoming_size = 0;
        switch (field.type.kind()) {
            case Type::kValue:
                DCHECK(!field.type.IsPointer());
                field.offset = RoundUp(offset, kPointerSize);
                
                if (field.type.model()->IsStructure()) {
                    incoming_size = down_cast<StructureModel>(field.type.model())->LazyPlacementSizeInBytes();
                } else {
                    incoming_size = field.type.model()->PlacementSizeInBytes();
                }
                offset = field.offset + incoming_size;
                AnalyzeRefsMark(field.type, field.offset);
                break;
            case Type::kString:
            case Type::kReference:
                incoming_size = field.type.model()->ReferenceSizeInBytes();
                field.offset = RoundUp(offset, incoming_size);
                offset = field.offset + incoming_size;
                refs_marks_.push_back({field.type, field.offset});
                break;
            case Type::kTuple:
                UNREACHABLE(); // Not support yet
                break;
            default:
                incoming_size = field.type.bytes();
                field.offset = RoundUp(offset, incoming_size);
                offset = field.offset + incoming_size;
                break;
        }
    }
    // size must be aligment of kPointerSize
    return RoundUp(offset, kPointerSize);
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

bool StructureModel::In_itab(const Handle *handle) const {
    for (auto it : itab_) {
        if (it == handle) {
            return true;
        }
    }
    return false;
}

bool StructureModel::In_vtab(const Handle *handle) const {
    for (auto it : vtab_) {
        if (it == handle) {
            return true;
        }
    }
    return false;
}

bool StructureModel::IsCompactEnum() const {
    if (declaration() != kEnum) {
        return false;
    }
    int refs_ty_count = 0, unit_ty_count = 0;
    for (auto field : fields()) {
        switch (field.type.kind()) {
            case Type::kReference:
            case Type::kString:
                refs_ty_count++;
                break;
            case Type::kVoid:
                unit_ty_count++;
                break;
            default:
                break;
        }
    }
    return refs_ty_count == 1 && unit_ty_count == 1 && fields_size() == 2;
}

Handle *StructureModel::EnumCodeFieldIfNotCompactEnum() {
    if (IsCompactEnum()) {
        return nullptr;
    }
    
    if (auto iter = members_.find(kEnumCodeName); iter != members_.end()) {
        return iter->second;
    }
    return InsertField({
        .name = String::New(arena_, kEnumCodeName),
        .access = kPublic,
        .offset = 0,
        .type = Types::UInt16,
        .enum_value = 0,
    });
}

} // namespace ir

} // namespace yalx
