#pragma once
#ifndef YALX_IR_OPERATORS_FACTORY_H_
#define YALX_IR_OPERATORS_FACTORY_H_

#include "ir/operator.h"
#include "ir/runtime.h"
#include "ir/condition.h"
#include "base/arena.h"
#include "base/base.h"

namespace yalx {

namespace ir {

class Handle;
class Function;
class StructureModel;
class InterfaceModel;
class ArrayModel;
class Model;


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
    
    Operator *Closure(Function *fun) {
        return new (arena_) OperatorWith<Function *>(Operator::kClosure, 0, 0/*value_in*/, 0/*control_in*/,
                                                     1/*value_out*/, 0/*control_out*/, fun);
    }
    
    Operator *Ret(int value_in) {
        return new (arena_) Operator(Operator::kRet, 0, value_in, 0/*control_in*/, 0/*value_out*/, 0/*control_out*/);
    }
    
    Operator *Unreachable() {
        return new (arena_) Operator(Operator::kUnreachable, 0, 0/*value_in*/, 0/*control_in*/, 0/*value_out*/,
                                     0/*control_out*/);
    }
    
    Operator *Unwind() {
        return new (arena_) Operator(Operator::kUnwind, 0, 0/*value_in*/, 0/*control_in*/, 0/*value_out*/,
                                     0/*control_out*/);
    }
    
    Operator *HeapAlloc(const StructureModel *model) {
        return new (arena_) OperatorWith<const StructureModel *>(Operator::kHeapAlloc, 0, 0/*value_in*/,
                                                                 0/*control_in*/, 1/*value_out*/, 0/*control_out*/,
                                                                 model);
    }
    
    Operator *StackAlloc(const StructureModel *model) {
        return new (arena_) OperatorWith<const StructureModel *>(Operator::kStackAlloc, 0, 0/*value_in*/,
                                                                 0/*control_in*/, 1/*value_out*/, 0/*control_out*/,
                                                                 model);
    }
    
    Operator *ArrayAlloc(const ArrayModel *model, int value_in) {
        return new (arena_) OperatorWith<const ArrayModel *>(Operator::kArrayAlloc, 0, value_in, 0/*control_in*/,
                                                             1/*value_out*/, 0/*control_out*/, model);
    }
    
    Operator *ArrayFill(const ArrayModel *model, int value_in) {
        return new (arena_) OperatorWith<const ArrayModel *>(Operator::kArrayFill, 0, value_in, 0/*control_in*/,
                                                             1/*value_out*/, 0/*control_out*/, model);
    }

    Operator *GlobalValue(const String *symbol) {
        return new (arena_) OperatorWith<const String *>(Operator::kGlobalValue, 0, 0/*value_in*/, 0/*control_in*/,
                                                         0/*value_out*/, 0/*control_out*/, symbol);
    }
    
    Operator *LazyValue(const String *symbol) {
        return new (arena_) OperatorWith<const String *>(Operator::kLazyValue, 0, 1/*value_in*/, 0/*control_in*/,
                                                         1/*value_out*/, 0/*control_out*/, symbol);
    }
    
    Operator *LazyLoad() {
        return new (arena_) Operator(Operator::kLazyLoad, 0, 1/*value_in*/, 0/*control_in*/, 1/*value_out*/,
                                     0/*control_out*/);
    }

    Operator *LoadGlobal() {
        return new (arena_) Operator(Operator::kLoadGlobal, 0, 1/*value_in*/, 0/*control_in*/, 1/*value_out*/,
                                     0/*control_out*/);
    }
    
    Operator *StoreGlobal() {
        return new (arena_) Operator(Operator::kStoreGlobal, 0, 2/*value_in*/, 0/*control_in*/, 0/*value_out*/,
                                     0/*control_out*/);
    }

    Operator *LoadFunAddr(const Function *fun) {
        return new (arena_) OperatorWith<const Function *>(Operator::kLoadFunAddr, 0, 0/*value_in*/, 0/*control_in*/,
                                                           0/*value_out*/, 0/*control_out*/, fun);
    }
    
    Operator *Catch() {
        return new (arena_) Operator(Operator::kCatch, 1, 0/*value_in*/, 0/*control_in*/, 0/*value_out*/,
                                     0/*control_out*/);
    }
    
    Operator *ArrayAt(const ArrayModel *type, int value_in) {
        return new (arena_) OperatorWith<const ArrayModel *>(Operator::kArrayAt, 0, value_in/*value_in*/,
                                                             0/*control_in*/, 1/*value_out*/, 0/*control_out*/, type);
    }
    
    Operator *ArraySet(const ArrayModel *type) {
        return new (arena_) OperatorWith<const ArrayModel *>(Operator::kArraySet, 0, 3/*value_in*/, 0/*control_in*/,
                                                             1/*value_out*/, 0/*control_out*/, type);
    }
    
    // Load Value type's pointer of field
    // input[0]: Value pinter
    Operator *LoadEffectAddress(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kLoadEffectAddress, 0, 1/*value_in*/, 0/*control_in*/,
                                                         0/*value_out*/, 0/*control_out*/, handle);
    }
    
    // Load Value type
    // input[0] must be a value
    Operator *LoadInlineField(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kLoadInlineField, 0, 1/*value_in*/, 0/*control_in*/,
                                                         0/*value_out*/, 0/*control_out*/, handle);
    }
    
    Operator *StoreInlineField(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kStoreInlineField, 0, 2/*value_in*/, 0/*control_in*/,
                                                         1/*value_out*/, 0/*control_out*/, handle);
    }
    
    // Load pointer type
    // input[0] must be a value pointer
    Operator *LoadAccessField(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kLoadAccessField, 0, 1/*value_in*/, 0/*control_in*/,
                                                         0/*value_out*/, 0/*control_out*/, handle);
    }
    
    Operator *StoreAccessField(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kStoreAccessField, 0, 2/*value_in*/, 0/*control_in*/,
                                                         1/*value_out*/, 0/*control_out*/, handle);
    }
    
    // Load ref type
    // input[0] must be a ref type object
    Operator *LoadEffectField(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kLoadEffectField, 0, 1/*value_in*/, 0/*control_in*/,
                                                         0/*value_out*/, 0/*control_out*/, handle);
    }
    
    Operator *StoreEffectField(const Handle *handle) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kStoreEffectField, 0, 2/*value_in*/, 0/*control_in*/,
                                                         1/*value_out*/, 0/*control_out*/, handle);
    }
    
    Operator *CallHandle(const Handle *handle, int value_out, int value_in, int control_out) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kCallHandle, 0, value_in, 0/*control_in*/,
                                                         value_out, control_out, handle);
    }
    
    Operator *CallVirtual(const Handle *handle, int value_out, int value_in, int control_out) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kCallVirtual, 0, value_in, 0/*control_in*/,
                                                         value_out, control_out, handle);
    }
    
    Operator *CallAbstract(const Handle *handle, int value_out, int value_in, int control_out) {
        return new (arena_) OperatorWith<const Handle *>(Operator::kCallAbstract, 0, value_in, 0/*control_in*/,
                                                         value_out, control_out, handle);
    }
    
    Operator *CallDirectly(Function *fun, int value_out, int value_in, int control_out) {
        return new (arena_) OperatorWith<Function *>(Operator::kCallDirectly, 0, value_in, 0/*control_in*/,
                                                     value_out, control_out, fun);
    }
    
    Operator *CallIndirectly(int value_out, int value_in, int control_out) {
        return new (arena_) Operator(Operator::kCallIndirectly, 0, value_in, 0/*control_in*/, value_out,
                                     control_out);
    }
    
    Operator *CallRuntime(int value_out, int value_in, int control_out, RuntimeId id) {
        return new (arena_) OperatorWith<RuntimeId>(Operator::kCallRuntime, 0, value_in, 0/*control_in*/, value_out,
                                                    control_out, id);
    }
    
    Operator *ReturningVal(int index) {
        return new (arena_) OperatorWith<int>(Operator::kReturningVal, 0, 1/*value_in*/, 0/*control_in*/, 1/*value_out*/,
                                              0/*control_out*/, index);
    }
    
#define DEFINE_CONSTANT(name, type) \
Operator *name(type value) { \
return new (arena_) OperatorWith<type>(Operator::k##name, 0, 0, 0, 0, 0, value); \
}
    
    DECL_CONSTANTS_WITH_DATA(DEFINE_CONSTANT)
    
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
    
    Operator *RefAssertedTo(int control_out) {
        return new (arena_) Operator(Operator::kRefAssertedTo, 0, 1/*value_in*/, 0/*control_in*/, 1/*value_out*/,
                                     control_out);
    }
    
    Operator *ICmp(IConditionId cond) {
        return new (arena_) OperatorWith<IConditionId>(Operator::kICmp, 0, 2/*value_in*/, 0/*control_in*/, 1/*value_out*/,
                                                       0/*control_out*/, cond);
    }
    
    Operator *FCmp(FConditionId cond) {
        return new (arena_) OperatorWith<FConditionId>(Operator::kFCmp, 0, 2/*value_in*/, 0/*control_in*/, 1/*value_out*/,
                                                       0/*control_out*/, cond);
    }
    
    Operator *IsInstanceOf(const Model *model) {
        return new (arena_) OperatorWith<const Model *>(Operator::kIsInstanceOf, 0, 1/*value_in*/, 0/*control_in*/,
                                                        1/*value_out*/, 0/*control_out*/, model);
    }
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(OperatorsFactory);
private:
    base::Arena *arena_;
}; // class OperatorsFactory

} // namespace ir

} // namespace yalx

#endif // YALX_IR_OPERATORS_FACTORY_H_
