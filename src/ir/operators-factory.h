#pragma once
#ifndef YALX_IR_OPERATORS_FACTORY_H_
#define YALX_IR_OPERATORS_FACTORY_H_

#include "ir/operator.h"
#include "base/arena.h"
#include "base/base.h"

namespace yalx {

namespace ir {

class Handle;
class Function;
class StructureModel;
class InterfaceModel;

class OperatorsFactory final {
public:
    OperatorsFactory(base::Arena *arena): arena_(arena) {}
    
    Operator *Argument(int hint) {
        return new (arena_) Operator(Operator::kArgument, hint, 0/*value_in*/, 0/*control_in*/, 0/*value_out*/,
                                     0/*control_out*/);
    }
    
    Operator *Phi(int value_in, int control_in) {
        return new (arena_) Operator(Operator::kPhi, 0, value_in, control_in, 1/*value_out*/, 0/*control_out*/);
    }
    
    Operator *Br(int value_in, int control_out) {
        return new (arena_) Operator(Operator::kBr, 0, value_in, 0/*control_in*/, 1/*value_out*/,
                                     control_out/*control_out*/);
    }
    
    Operator *Ret(int value_in) {
        return new (arena_) Operator(Operator::kRet, 0, value_in, 0/*control_in*/, 0/*value_out*/, 0/*control_out*/);
    }
    
    Operator *GlobalValue(const String *symbol) {
        return new (arena_) OperatorWith<const String *>(Operator::kGlobalValue, 0, 0/*value_in*/, 0/*control_in*/,
                                                         0/*value_out*/, 0/*control_out*/, symbol);
    }
    
    Operator *LazyValue(const String *symbol) {
        return new (arena_) OperatorWith<const String *>(Operator::kLazyValue, 0, 0/*value_in*/, 0/*control_in*/,
                                                         0/*value_out*/, 0/*control_out*/, symbol);
    }
    
    Operator *LoadGlobal() {
        return new (arena_) Operator(Operator::kLoadGlobal, 0, 1/*value_in*/, 0/*control_in*/, 1/*value_out*/,
                                     0/*control_out*/);
    }
    
    Operator *StoreGlobal() {
        return new (arena_) Operator(Operator::kStoreGlobal, 0, 2/*value_in*/, 0/*control_in*/, 0/*value_out*/,
                                     0/*control_out*/);
    }
    
    Operator *LoadInlineField(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kLoadInlineField, 0, 1/*value_in*/, 0/*control_in*/,
                                                         0/*value_out*/, 0/*control_out*/, handle);
    }
    
    Operator *StoreInlineField(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kStoreInlineField, 0, 1/*value_in*/, 0/*control_in*/,
                                                         1/*value_out*/, 0/*control_out*/, handle);
    }
    
    Operator *LoadAccessField(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kLoadAccessField, 0, 1/*value_in*/, 0/*control_in*/,
                                                         0/*value_out*/, 0/*control_out*/, handle);
    }
    
    Operator *StoreAccessField(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kStoreAccessField, 0, 1/*value_in*/, 0/*control_in*/,
                                                         1/*value_out*/, 0/*control_out*/, handle);
    }
    
    Operator *LoadEffectField(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kLoadEffectField, 0, 1/*value_in*/, 0/*control_in*/,
                                                         0/*value_out*/, 0/*control_out*/, handle);
    }
    
    Operator *StoreEffectField(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kStoreEffectField, 0, 1/*value_in*/, 0/*control_in*/,
                                                         1/*value_out*/, 0/*control_out*/, handle);
    }
    
    Operator *CallHandle(const Handle *handle, int value_out, int value_in) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kCallHandle, 0, value_in, 0/*control_in*/,
                                                         value_out, 0/*control_out*/, handle);
    }
    
    Operator *CallVirtual(const Handle *handle, int value_out, int value_in) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kCallVirtual, 0, value_in, 0/*control_in*/,
                                                         value_out, 0/*control_out*/, handle);
    }
    
    Operator *CallAbstract(const Handle *handle, int value_out, int value_in) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kCallAbstract, 0, value_in, 0/*control_in*/,
                                                         value_out, 0/*control_out*/, handle);
    }
    
    Operator *CallDirectly(Function *fun, int value_out, int value_in) {
        return new (arena_) OperatorWith<Function *>(Operator::kCallDirectly, 0, value_in, 0/*control_in*/,
                                                     value_out, 0/*control_out*/, fun);
    }
    
    Operator *CallIndirectly(int value_out, int value_in) {
        return new (arena_) Operator(Operator::kCallIndirectly, 0, value_in, 0/*control_in*/, value_out,
                                     0/*control_out*/);
    }
    
    Operator *ReturningVal(int index) {
        return new (arena_) OperatorWith<int>(Operator::kReturningVal, 0, 1/*value_in*/, 0/*control_in*/, 1/*value_out*/,
                                              0/*control_out*/, index);
    }
    
#define DEFINE_CONSTANT(name, type) \
Operator *name##Constant(type value) { \
return new (arena_) OperatorWith<type>(Operator::k##name##Constant, 0, 0, 0, 0, 0, value); \
}
    DEFINE_CONSTANT(Word8, uint8_t)
    DEFINE_CONSTANT(Word16, uint16_t)
    DEFINE_CONSTANT(Word32, uint32_t)
    DEFINE_CONSTANT(Word64, uint64_t)
    
    DEFINE_CONSTANT(I8, int8_t)
    DEFINE_CONSTANT(I16, int16_t)
    DEFINE_CONSTANT(I32, int32_t)
    DEFINE_CONSTANT(I64, int64_t)
    DEFINE_CONSTANT(U8, uint8_t)
    DEFINE_CONSTANT(U16, uint16_t)
    DEFINE_CONSTANT(U32, uint32_t)
    DEFINE_CONSTANT(U64, uint64_t)
    DEFINE_CONSTANT(F32, float)
    DEFINE_CONSTANT(F64, double)
    
    DEFINE_CONSTANT(String, String const*)
    
#undef DEFINE_CONSTANT
    
    Operator *NilConstant() {
        return new (arena_) Operator(Operator::kNilConstant, 0, 0/*value_in*/, 0/*control_in*/, 1/*value_out*/,
                                     0/*control_out*/);
    }
    
#define DEFINE_BINARY(name) \
Operator *name() { \
return new (arena_) Operator(Operator::k##name, 0, 2, 0, 1, 0); \
}
    
    DECLARE_IR_BINARY(DEFINE_BINARY)
    
#undef DEFINE_BINARY

#define DEFINE_CONVERSION(name) \
    Operator *name() { \
        return new (arena_) Operator(Operator::k##name, 0, 1/*value_in*/, 0/*control_in*/, 1/*value_out*/, \
                                     0/*control_out*/); \
    }
    
    DECLARE_IR_CONVERSION(DEFINE_CONVERSION)
    
#undef DEFINE_CONVERSION
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(OperatorsFactory);
private:
    base::Arena *arena_;
}; // class OperatorsFactory

} // namespace ir

} // namespace yalx

#endif // YALX_IR_OPERATORS_FACTORY_H_
