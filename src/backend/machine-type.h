#pragma once
#ifndef YALX_BACKEND_MACHINE_TYPE_H_
#define YALX_BACKEND_MACHINE_TYPE_H_

#include "base/base.h"
#include <tuple>

namespace yalx {
namespace ir {
class Type;
} // namespace ir
namespace backend {

#define DECLARE_MACHINE_REPRESENTATION(V) \
    V(None,      none) \
    V(Bit,       bit) \
    V(Word8,     byte) \
    V(Word16,    word) \
    V(Word32,    dword) \
    V(Word64,    qword) \
    V(Pointer,   ptr) \
    V(Reference, ref) \
    V(Float32,   float) \
    V(Float64,   double)

enum class MachineRepresentation : uint8_t {
#define DEFINE_ENUM(name, alias) k##name,
    DECLARE_MACHINE_REPRESENTATION(DEFINE_ENUM)
#undef DEFINE_ENUM
}; // enum class MachineRepresentation

MachineRepresentation ToMachineRepresentation(const ir::Type ty);

const char *GetMachineRepresentationName(MachineRepresentation rep);
const char *GetMachineRepresentationAlias(MachineRepresentation rep);

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_MACHINE_TYPE_H_
