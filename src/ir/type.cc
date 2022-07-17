#include "ir/type.h"
#include "ir/metadata.h"
#include "compiler/constants.h"
#include "base/io.h"

namespace yalx {

namespace ir {

static const char *kTypeNames[] = {
//#define DEFINE_KINDS(name, ...) #name,
    //DECLARE_HIR_TYPES(DEFINE_KINDS)
    "void",
    "byte",
    "word",
    "dword",
    "qword",
    "i8",
    "i16",
    "i32",
    "i64",
    "u8",
    "u16",
    "u32",
    "u64",
    "f32",
    "f64",
    "string",
    "Reference",
    "Value",
    "Tuple"
//#undef DEFINE_KINDS
};

std::string_view Type::ToString() const {
    switch (kind()) {
        case kReference:
        case kValue:
            return model()->full_name()->ToSlice();
            
        default:
            return kTypeNames[kind()];
    }
    return "";
}

size_t Type::ReferenceSizeInBytes() const {
    if (IsPointer() || IsReference()) {
        return kPointerSize;
    }
    if (kind() == kValue) {
        return model()->PlacementSizeInBytes();
    }
    return bytes();
}

size_t Type::PlacementSizeInBytes() const {
    if (model()) {
        return model()->PlacementSizeInBytes();
    }
    return bytes();
}

Type Type::Ref(Model *model, bool nullable) {
    return Type(model->full_name()->Equal(cpl::kStringClassFullName) ? kString : kReference,
                static_cast<int>(DCHECK_NOTNULL(model)->ReferenceSizeInBytes() << 3),
                (nullable ? kNullableBit : 0) | kReferenceBit,
                DCHECK_NOTNULL(model));
}

Type Type::Val(Model *model, bool is_pointer) {
    return Type(kValue,
                static_cast<int>(DCHECK_NOTNULL(model)->ReferenceSizeInBytes() << 3),
                (is_pointer ? kPointerBit : 0),
                DCHECK_NOTNULL(model));
}

Type Type::Tuple(base::Arena *arena, Type *tuple, int size) {
    auto types = arena->NewArray<Type>(size);
    ::memcpy(types, tuple, size * sizeof(Type));
    return Type(kTuple, 0, types, size);
}

void Type::PrintTo(base::PrintingWriter *printer) const {
    switch (kind()) {
        case kReference: {
            printer->Write("ref[")->Write(model()->full_name()->ToSlice())->Write("]");
            if (IsNullable()) {
                printer->Write("?");
            }
        } break;
            
        case kValue: {
            printer->Write("val[")->Write(model()->full_name()->ToSlice())->Write("]");
            if (IsPointer()) {
                printer->Write("*");
            }
        } break;
            
        default:
            printer->Write(kTypeNames[kind()]);
            if (IsNullable()) {
                printer->Write("?");
            }
            break;
    }
}

} // namespace ir

} // namespace yalx
