#include "ir/type.h"
#include "ir/metadata.h"

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

Type Type::Ref(Model *model, bool _nullable) {
    return Type(kReference,
                (DCHECK_NOTNULL(model)->ReferenceSizeInBytes() << 3),
                (_nullable ? kNullableBit : 0) | kReferenceBit,
                DCHECK_NOTNULL(model));
}

Type Type::Val(Model *model) {
    return Type(kValue,
                (DCHECK_NOTNULL(model)->ReferenceSizeInBytes() << 3),
                0,
                DCHECK_NOTNULL(model));
}

} // namespace ir

} // namespace yalx
