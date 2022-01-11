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
    DECLARE_IR_GLOBALS(V) \
    DECLARE_IR_CONVERSION(V)


#define DECLARE_IR_INNER(V) \
    V(FrameState) \
    V(Phi) \
    V(Br) \
    V(Argument) \
    V(Parameter) \
    V(HeapAlloc) \
    V(StackAlloc) \
    V(Ret)

#define DECLARE_IR_GLOBALS(V) \
    V(GlobalValue) \
    V(LoadGlobal) \
    V(StoreGlobal) \
    V(LazyValue) \
    V(LazyLoad) \
    V(LoadAccessField) \
    V(StoreAccessField) \
    V(LoadInlineField) \
    V(StoreInlineField) \
    V(LoadEffectField) \
    V(StoreEffectField)

#define DECLARE_IR_CALLING(V) \
    V(CallHandle) \
    V(CallVirtual) \
    V(CallAbstract) \
    V(CallDirectly) \
    V(CallIndirectly) \
    V(CallRuntime) \
    V(ReturningVal)

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
    V(Add) \
    V(Sub) \
    V(Mul) \
    V(Div) \
    V(UMul) \
    V(UDiv) \
    V(BitwiseAnd) \
    V(BitwiseOr) \
    V(BitwiseShl) \
    V(BitwiseShr) \


#define DECLARE_IR_CONVERSION(V) \
    V(TruncTo) \
    V(ZextTo) \
    V(SextTo) \
    V(FPTruncTo) \
    V(FPExtTo) \
    V(FPToUI) \
    V(FPToSI) \
    V(UIToFP) \
    V(SIToFP) \
    V(BitCastTo) \
    V(GetAddr) \
    V(Dref) \
    V(IfaceToRef) \
    V(RefToIface) \
    V(RefAssertedTo) \
    V(BoxingTo) \
    V(UnboxingTo) 


#define DECL_OPERATORS_WITH_DATA(V) \
    V(Argument,         int) \
    V(ReturningVal,     int) \
    V(HeapAlloc,        Model const *) \
    V(StackAlloc,       Model const *) \
    V(GlobalValue,      String const *) \
    V(LazyValue,        String const *) \
    V(LoadInlineField,  Handle const *) \
    V(LoadAccessField,  Handle const *) \
    V(LoadEffectField,  Handle const *) \
    V(StoreInlineField, Handle const *) \
    V(StoreAccessField, Handle const *) \
    V(StoreEffectField, Handle const *) \
    V(CallHandle,       Handle const *) \
    V(CallVirtual,      Handle const *) \
    V(CallAbstract,     Handle const *) \
    V(CallDirectly,     Function *) \
    DECL_CONSTANTS_WITH_DATA(V)

#define DECL_CONSTANTS_WITH_DATA(V) \
    V(Word8Constant, uint8_t) \
    V(Word16Constant, uint16_t) \
    V(Word32Constant, uint32_t) \
    V(Word64Constant, uint64_t) \
    V(I8Constant, int8_t) \
    V(I16Constant, int16_t) \
    V(I32Constant, int32_t) \
    V(I64Constant, int64_t) \
    V(U8Constant, uint8_t) \
    V(U16Constant, uint16_t) \
    V(U32Constant, uint32_t) \
    V(U64Constant, uint64_t) \
    V(F32Constant, float) \
    V(F64Constant, double) \
    V(StringConstant, String const *)

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
