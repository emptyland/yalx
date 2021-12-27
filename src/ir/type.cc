#include "ir/type.h"
#include "ir/metadata.h"

namespace yalx {

namespace ir {

static const char *kTypeNames[] = {
#define DEFINE_KINDS(name, ...) #name,
    DECLARE_HIR_TYPES(DEFINE_KINDS)
    "Reference",
    "Value",
#undef DEFINE_KINDS
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
