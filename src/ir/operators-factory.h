#pragma once
#ifndef YALX_IR_OPERATORS_FACTORY_H_
#define YALX_IR_OPERATORS_FACTORY_H_

#include "ir/operator.h"
#include "base/arena.h"
#include "base/base.h"

namespace yalx {

namespace ir {

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
        return new (arena_) Operator(Operator::kStoreGlobal, 0, 1/*value_in*/, 0/*control_in*/, 0/*value_out*/,
                                     0/*control_out*/);
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
    
    DEFINE_CONSTANT(String, String*)
    
#undef DEFINE_CONSTANT
    
#define DEFINE_BINARY(name) \
    Operator *name() { \
        return new (arena_) Operator(Operator::k##name, 0, 2, 0, 1, 0); \
    }
    
    DECLARE_IR_BINARY(DEFINE_BINARY)
    
#undef DEFINE_BINARY
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(OperatorsFactory);
private:
    base::Arena *arena_;
}; // class OperatorsFactory

} // namespace ir

} // namespace yalx

#endif // YALX_IR_OPERATORS_FACTORY_H_
