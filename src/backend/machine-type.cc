#include "backend/machine-type.h"
#include "ir/metadata.h"
#include "ir/type.h"

namespace yalx::backend {

MachineRepresentation ToMachineRepresentation(const ir::Type ty) {
    switch (ty.kind()) {
        case ir::Type::kWord8:
        case ir::Type::kInt8:
        case ir::Type::kUInt8:
            return MachineRepresentation::kWord8;
        case ir::Type::kWord16:
        case ir::Type::kInt16:
        case ir::Type::kUInt16:
            return MachineRepresentation::kWord16;
        case ir::Type::kWord32:
        case ir::Type::kInt32:
        case ir::Type::kUInt32:
            return MachineRepresentation::kWord32;
        case ir::Type::kWord64:
        case ir::Type::kInt64:
        case ir::Type::kUInt64:
            return MachineRepresentation::kWord64;
        case ir::Type::kFloat32:
            return MachineRepresentation::kFloat32;
        case ir::Type::kFloat64:
            return MachineRepresentation::kFloat64;
        case ir::Type::kReference:
        case ir::Type::kString:
            return MachineRepresentation::kReference;
        case ir::Type::kValue:
            if (ty.IsPointer()) {
                return MachineRepresentation::kPointer;
            }
            if (ty.IsCompactEnum()) {
                return MachineRepresentation::kReference;
            }
            return MachineRepresentation::kNone;
        default:
            return MachineRepresentation::kNone;
    }
}

static const char *kNames[] = {
#define DEFINE_NAME(name, alias) #name,
    DECLARE_MACHINE_REPRESENTATION(DEFINE_NAME)
#undef  DEFINE_NAME
};

static const char *kAlias[] = {
#define DEFINE_NAME(name, alias) #alias,
    DECLARE_MACHINE_REPRESENTATION(DEFINE_NAME)
#undef  DEFINE_NAME
};

const char *GetMachineRepresentationName(MachineRepresentation rep) {
    auto i = static_cast<int>(rep);
    DCHECK(i >= 0 && i < arraysize(kNames));
    return kNames[i];
}

const char *GetMachineRepresentationAlias(MachineRepresentation rep) {
    auto i = static_cast<int>(rep);
    DCHECK(i >= 0 && i < arraysize(kNames));
    return kAlias[i];
}

} // namespace yalx
