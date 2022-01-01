#pragma once
#ifndef YALX_IR_OPERATOR_H_
#define YALX_IR_OPERATOR_H_

#include "base/checking.h"
#include "base/arena.h"

namespace yalx {
namespace base {
class ArenaString;
} // namespace base
namespace ir {

#define DECLARE_ALL_IR(V) \
    DECLARE_IR_INNER(V) \
    DECLARE_IR_CONSTANT(V) \
    DECLARE_IR_CALLING(V) \
    DECLARE_IR_BINARY(V) \
    DECLARE_IR_GLOBALS(V)


#define DECLARE_IR_INNER(V) \
    V(FrameState) \
    V(Phi) \
    V(Argument) \
    V(Parameter) \
    V(Ret)

#define DECLARE_IR_GLOBALS(V) \
    V(GlobalValue) \
    V(LoadGlobal) \
    V(StoreGlobal) \
    V(LazyValue) \
    V(LazyLoad)

#define DECLARE_IR_CALLING(V) \
    V(CallInline) \
    V(CallDirectly) \
    V(CallIndirectly) \
    V(CallRuntime)

#define DECLARE_IR_CONSTANT(V) \
    V(Word8Constant) \
    V(Word16Constant) \
    V(Word32Constant) \
    V(Word64Constant) \
    V(I8Constant) \
    V(I16Constant) \
    V(I32Constant) \
    V(I64Constant) \
    V(U8Constant) \
    V(U16Constant) \
    V(U32Constant) \
    V(U64Constant) \
    V(StringConstant) \
    V(F32Constant) \
    V(F64Constant) \
    V(NilConstant)

#define DECLARE_IR_BINARY(V) \
    V(Word32Add) \
    V(Word32Sub) \
    V(U32Mul) \
    V(U32Div) \
    V(I32Mul) \
    V(I32Div) \
    V(Word32BitwiseAnd) \
    V(Word32BitwiseOr) \
    V(Word32BitwiseShl) \
    V(U32BitwiseShr) \
    V(I32BitwiseShr)

// V(Word32BitwiseNegative) \

class Node;
class Value;
using String = base::ArenaString;

class Operator : public base::ArenaObject {
public:
    enum Value {
    #define DEFINE_ENUM(name) k##name,
        DECLARE_ALL_IR(DEFINE_ENUM)
    #undef DEFINE_ENUM
        kMaxValues,
    };
    
    DEF_VAL_GETTER(Value, value);
    DEF_VAL_GETTER(uint32_t, properties);
    DEF_VAL_GETTER(int, value_in);
    DEF_VAL_GETTER(int, control_in);
    DEF_VAL_GETTER(int, value_out);
    DEF_VAL_GETTER(int, control_out);
    
    const char *name() const { return kNames[value()]; }
    
    friend class OperatorsFactory;
protected:
    Operator(Value value, uint32_t properties, int value_in, int control_in, int value_out, int control_out);
    
    Value value_;
    uint32_t properties_;
    int value_in_;
    int control_in_;
    int value_out_;
    int control_out_;
    
    static const char *kNames[kMaxValues];
}; // class Operator


template<class T>
class OperatorWith : public Operator {
public:
    DEF_VAL_GETTER(T, data);
    
    static inline T Data(const Operator *op) { return Cast(op)->data(); }
    
    static inline T Data(const ir::Value *node);
    
    static inline OperatorWith<T> *Cast(Operator *op) { return static_cast<OperatorWith<T> *>(DCHECK_NOTNULL(op)); }
    
    static inline const OperatorWith<T> *Cast(const Operator *op) {
        return static_cast<const OperatorWith<T> *>(DCHECK_NOTNULL(op));
    }
    
    friend class OperatorsFactory;
private:
    inline OperatorWith(Value value, uint32_t properties, int value_in, int control_in, int value_out, int control_out,
                        T data)
        : Operator(value, properties, value_in, control_in, value_out, control_out)
        , data_(data) {}
    
    T data_;
}; // template<class T> class OperatorWith

} // namespace ir

} // namespace yalx

#endif // YALX_IR_OPERATOR_H_
