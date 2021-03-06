#include "ir/operator.h"


namespace yalx {

namespace ir {

Operator::Operator(Value value, uint32_t properties, int value_in, int control_in, int value_out, int control_out)
    : value_(value)
    , properties_(properties)
    , value_in_(value_in)
    , control_in_(control_in)
    , value_out_(value_out)
    , control_out_(control_out) {
}

bool Operator::IsConstant() const {
    switch (value()) {
#define DEFINE_CASE(name) case k##name:
        DECLARE_IR_CONSTANT(DEFINE_CASE)
#undef DEFINE_CASE
            return true;
        default:
            break;
    }
    return false;
}

} // namespace ir

} // namespace yalx
