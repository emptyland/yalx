#pragma once
#ifndef YALX_IR_CONSTANTS_H_
#define YALX_IR_CONSTANTS_H_

#include "ir/operator.h"
#include "base/base.h"

namespace yalx {

namespace ir {

class Type;

enum ConversionHint {
#define DEFINE_ENUM(name) k##name,
    DECLARE_IR_CONVERSION(DEFINE_ENUM)
#undef DEFINE_ENUM
    kKeep,
    kDeny,
}; // enum ConversionHint

ConversionHint GetConversionHint(const Type &dest, const Type &src);

extern const char *kOpcodeNames[Operator::kMaxValues];

} // namespace ir

} // namespace yalx

#endif // YALX_IR_CONSTANTS_H_
