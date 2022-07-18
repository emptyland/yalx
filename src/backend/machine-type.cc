#include "backend/machine-type.h"
#include "ir/metadata.h"
#include "ir/type.h"

namespace yalx {
namespace backend {

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
            return MachineRepresentation::kWord64;
        case ir::Type::kValue:
            return (ty.IsPointer() || ty.IsCompactEnum()) ? MachineRepresentation::kWord64 : MachineRepresentation::kNone;
        default:
            return MachineRepresentation::kNone;
    }
}

} // namespace backend
} // namespace yalx
