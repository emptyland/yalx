#include "ir/type.h"
#include "ir/metadata.h"
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

Type Type::Ref(Model *model, bool nullable) {
    return Type(kReference,
                (DCHECK_NOTNULL(model)->ReferenceSizeInBytes() << 3),
                (nullable ? kNullableBit : 0) | kReferenceBit,
                DCHECK_NOTNULL(model));
}

Type Type::Val(Model *model, bool is_pointer) {
    return Type(kValue,
                (DCHECK_NOTNULL(model)->ReferenceSizeInBytes() << 3),
                (is_pointer ? kPointerBit : 0),
                DCHECK_NOTNULL(model));
}

void Type::PrintTo(base::PrintingWriter *printer) const {
    switch (kind()) {
        case kReference: {
            printer->Write("ref ")->Write(model()->full_name()->ToSlice());
            if (IsNullable()) {
                printer->Write("?");
            }
        } break;
            
        case kValue: {
            printer->Write("val ")->Write(model()->full_name()->ToSlice());
            if (IsPointer()) {
                printer->Write("*");
            }
        } break;
            
        default:
            printer->Write(kTypeNames[kind()]);
            break;
    }
}

} // namespace ir

} // namespace yalx
