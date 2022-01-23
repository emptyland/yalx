#pragma once
#ifndef YALX_IR_RUNTIME_H_
#define YALX_IR_RUNTIME_H_

#include "base/base.h"

namespace yalx {

namespace ir {

#define DECL_RUNTIME_IDS(V) \
    V(PkgInitOnce) \
    V(StringEQ) \
    V(StringNE) \
    V(StringLT) \
    V(StringLE) \
    V(StringGT) \
    V(StringGE) \
    V(Raise)


struct RuntimeId {
    enum Value {
    #define DEFINE_ENUM(name) k##name,
        DECL_RUNTIME_IDS(DEFINE_ENUM)
    #undef DEFINE_ENUM
    };
    
    constexpr static RuntimeId Of(Value code) {
        return {code};
    }
    
    const char *ToString() const;
    
    const Value value;
};



struct RuntimeLib {
#define DEFINE_CONST(name) static constexpr RuntimeId name = RuntimeId::Of(RuntimeId::k##name);
    DECL_RUNTIME_IDS(DEFINE_CONST)
#undef DEFINE_CONST
    
    DISALLOW_ALL_CONSTRUCTORS(RuntimeLib);
};

} // namespace ir

} // namespace yalx

#endif // YALX_IR_RUNTIME_H_
