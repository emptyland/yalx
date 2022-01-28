#pragma once
#ifndef YALX_BACKEND_MACHINE_TYPE_H_
#define YALX_BACKEND_MACHINE_TYPE_H_

#include "base/base.h"

namespace yalx {

namespace backend {

enum class MachineRepresentation : uint8_t {
    kNone,
    kBit,
    kWord8,
    kWord16,
    kWord32,
    kWord64,
    kFloat32,
    kFloat64,
};

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_MACHINE_TYPE_H_
