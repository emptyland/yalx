#pragma once
#ifndef YALX_IR_CONDITION_H_
#define YALX_IR_CONDITION_H_

#include "base/base.h"

namespace yalx {

namespace ir {

// integral
#define DECL_INTEGRAL_CONDITIONS(V) \
    V(eq) \
    V(ne) \
    V(ult) \
    V(ule) \
    V(ugt) \
    V(uge) \
    V(slt) \
    V(sle) \
    V(sgt) \
    V(sge)

//false: no comparison, always returns false
//oeq: ordered and equal
//ogt: ordered and greater than
//oge: ordered and greater than or equal
//olt: ordered and less than
//ole: ordered and less than or equal
//one: ordered and not equal
//ord: ordered (no nans)
//ueq: unordered or equal
//ugt: unordered or greater than
//uge: unordered or greater than or equal
//ult: unordered or less than
//ule: unordered or less than or equal
//une: unordered or not equal
//uno: unordered (either nans)
//true: no comparison, always returns true
#define DECL_FLOATING_CONDITIONS(V) \
    V(never) \
    V(oeq) \
    V(ogt) \
    V(oge) \
    V(olt) \
    V(ole) \
    V(one) \
    V(ord) \
    V(ueq) \
    V(ugt) \
    V(uge) \
    V(ult) \
    V(ule) \
    V(une) \
    V(uno) \
    V(always)

struct IConditionId {
    enum Value {
#define DEFINE_ENUM(name) k_##name,
        DECL_INTEGRAL_CONDITIONS(DEFINE_ENUM)
#undef DEFINE_ENUM
    };
    
    constexpr static IConditionId Of(Value code) {
        return {code};
    }
    
    const char *ToString() const;
    
    const Value value;
}; // struct IntegralConditionId


struct FConditionId {
    enum Value {
#define DEFINE_ENUM(name) k_##name,
        DECL_FLOATING_CONDITIONS(DEFINE_ENUM)
#undef DEFINE_ENUM
    };
    
    constexpr static FConditionId Of(Value code) {
        return {code};
    }
    
    const char *ToString() const;
    
    const Value value;
}; // struct FConditionId

struct ICondition {
#define DEFINE_CONST(name) static constexpr IConditionId name = IConditionId::Of(IConditionId::k_##name);
    DECL_INTEGRAL_CONDITIONS(DEFINE_CONST)
#undef DEFINE_CONST
    
    DISALLOW_ALL_CONSTRUCTORS(ICondition);
};

struct FCondition {
#define DEFINE_CONST(name) static constexpr FConditionId name = FConditionId::Of(FConditionId::k_##name);
    DECL_FLOATING_CONDITIONS(DEFINE_CONST)
#undef DEFINE_CONST
    
    DISALLOW_ALL_CONSTRUCTORS(FCondition);
};

} // namespace ir

} // namespace yalx

#endif // YALX_IR_CONDITION_H_
