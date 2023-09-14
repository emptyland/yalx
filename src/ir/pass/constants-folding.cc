#include "ir/pass/constants-folding.h"
#include "ir/operators-factory.h"
#include <math.h>

namespace yalx::ir {

ConstantsFoldingPass::ConstantsFoldingPass(base::Arena *arena, OperatorsFactory *ops, ModulesMap *modules,
                                           cpl::SyntaxFeedback *feedback)
: Pass<ConstantsFoldingPass>(arena, ops, modules, feedback) {
}

void ConstantsFoldingPass::RunModule(Module *module) {
    for (size_t i = 0; i < module->values_size(); i++) {
        if (module->value(i)->IsAlive()) {
            auto value = module->value(i);
            (*module->mutable_values())[i] = FoldGlobalValue(value);
        }
    }
    
    ForeachUdt(module);
    ForeachFunction(module);
}

void ConstantsFoldingPass::RunFun(Function *fun) {
    for (auto blk : fun->blocks()) {
        blk->RemoveDeads(); // clear first
    }
    
    ForeachBasicBlock(fun);
    
    for (auto blk : fun->blocks()) {
        blk->RemoveDeads(); // clear again
    }
}

void ConstantsFoldingPass::RunBasicBlock(BasicBlock *block) {
    for (auto instr : block->instructions()) {
        bool folded = false;
        auto new_val = FoldValueIfNeeded(instr, &folded);
        if (folded) {
            for (auto [position, user] : instr->GetUsers()) {
                user->Replace(arena(), position, instr, new_val);
            }
            instr->Kill();
        }
    }
}

Value *ConstantsFoldingPass::FoldGlobalValue(Value *value) {
    int store_count = 0;
    Value *store = nullptr;
    for (auto edge : value->users()) {
        if (edge.user->Is(Operator::kStoreGlobal)) {
            store = edge.user;
            store_count++;
        }
    }

    if (store_count == 1) {
        bool folded = false;
        Value *new_val = nullptr;
        if (store->InputValue(1)->op()->IsConstant()) {
            new_val = store->InputValue(1);
            folded = true;
        } else {
            new_val = FoldValueIfNeeded(store->InputValue(1), &folded);
        }
        if (folded) {
            std::vector<Value *> loads;
            for (auto edge : value->users()) {
                if (edge.user->Is(Operator::kStoreGlobal)) {
                    edge.user->Kill();
                } else if (edge.user->Is(Operator::kLoadGlobal)) {
                    loads.push_back(edge.user);
                }
            }
            
            for (auto load : loads) {
                for (auto [position, user] : load->GetUsers()) {
                    user->Replace(arena(), position, load, new_val);
                }
                load->Kill();
            }
            return new_val;
        }
    }

    return value;
}

using FoldingOperator = Value *(base::Arena *, Operator *, Value *, Value *, OperatorsFactory *);

#define FOLDING_OP_ARGS base::Arena *arena, Operator *op, Value *lhs, Value *rhs, OperatorsFactory *ops

#define DECLARE_NUMBER_KINDS(V) \
    DECLARE_INTEGRAL_KINDS(V) \
    DECLARE_FLOATING_KINDS(V)

#define DECLARE_INTEGRAL_KINDS(V) \
    DECLARE_UNSIGNED_KINDS(V) \
    DECLARE_SIGNED_KINDS(V)

#define DECLARE_UNSIGNED_KINDS(V) \
    V(uint8_t,  Word8,  Word8) \
    V(uint16_t, Word16, Word16) \
    V(uint32_t, Word32, Word32) \
    V(uint64_t, Word64, Word64) \
    V(uint8_t,  U8,     UInt8) \
    V(uint16_t, U16,    UInt16) \
    V(uint32_t, U32,    UInt32) \
    V(uint64_t, U64,    UInt64)

#define DECLARE_SIGNED_KINDS(V) \
    V(int8_t,   I8,     Int8) \
    V(int16_t,  I16,    Int16) \
    V(int32_t,  I32,    Int32) \
    V(int64_t,  I64,    Int64)

#define DECLARE_FLOATING_KINDS(V) \
    V(float,    F32,    Float32) \
    V(double,   F64,    Float64)


#define DEFINE_ADD_OP(ty, name, kind) \
static Value *Add##name(FOLDING_OP_ARGS) { \
    auto value = OperatorWith<ty>::Data(lhs->op()) + OperatorWith<ty>::Data(rhs->op()); \
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->name##Constant(value)); \
}

#define DEFINE_SUB_OP(ty, name, kind) \
static Value *Sub##name(FOLDING_OP_ARGS) { \
    auto value = OperatorWith<ty>::Data(lhs->op()) - OperatorWith<ty>::Data(rhs->op()); \
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->name##Constant(value)); \
}

#define DEFINE_MUL_OP(ty, name, kind) \
static Value *Mul##name(FOLDING_OP_ARGS) { \
    auto value = OperatorWith<ty>::Data(lhs->op()) * OperatorWith<ty>::Data(rhs->op()); \
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->name##Constant(value)); \
}

#define DEFINE_DIV_OP(ty, name, kind) \
static Value *Div##name(FOLDING_OP_ARGS) { \
    if (OperatorWith<ty>::Data(rhs) == 0) { \
        return nullptr; \
    } \
    auto value = OperatorWith<ty>::Data(lhs->op()) / OperatorWith<ty>::Data(rhs->op()); \
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->name##Constant(value)); \
}

#define DEFINE_FDIV_OP(ty, name, kind) \
static Value *Div##name(FOLDING_OP_ARGS) { \
    auto value = OperatorWith<ty>::Data(lhs->op()) / OperatorWith<ty>::Data(rhs->op()); \
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->name##Constant(value)); \
}

#define DEFINE_REM_OP(ty, name, kind) \
static Value *Rem##name(FOLDING_OP_ARGS) { \
    if (OperatorWith<ty>::Data(rhs) == 0) { \
        return nullptr; \
    } \
    auto value = OperatorWith<ty>::Data(lhs->op()) % OperatorWith<ty>::Data(rhs->op()); \
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->name##Constant(value)); \
}

static Value *RemF32(FOLDING_OP_ARGS) {
    auto value = ::fmodf(OperatorWith<float>::Data(lhs->op()), OperatorWith<float>::Data(rhs->op()));
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->F32Constant(value));
}

static Value *RemF64(FOLDING_OP_ARGS) {
    auto value = ::fmod(OperatorWith<double>::Data(lhs->op()), OperatorWith<double>::Data(rhs->op()));
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->F64Constant(value));
}

#define DEFINE_AND_OP(ty, name, kind) \
static Value *And##name(FOLDING_OP_ARGS) { \
    auto value = OperatorWith<ty>::Data(lhs->op()) & OperatorWith<ty>::Data(rhs->op()); \
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->name##Constant(value)); \
}

#define DEFINE_FAND_OP(ty, name, kind) \
static Value *And##name(FOLDING_OP_ARGS) { \
    return nullptr; \
}

#define DEFINE_OR_OP(ty, name, kind) \
static Value *Or##name(FOLDING_OP_ARGS) { \
    auto value = OperatorWith<ty>::Data(lhs->op()) | OperatorWith<ty>::Data(rhs->op()); \
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->name##Constant(value)); \
}

#define DEFINE_FOR_OP(ty, name, kind) \
static Value *Or##name(FOLDING_OP_ARGS) { \
    return nullptr; \
}

#define DEFINE_XOR_OP(ty, name, kind) \
static Value *Xor##name(FOLDING_OP_ARGS) { \
    auto value = OperatorWith<ty>::Data(lhs->op()) ^ OperatorWith<ty>::Data(rhs->op()); \
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->name##Constant(value)); \
}

#define DEFINE_FXOR_OP(ty, name, kind) \
static Value *Xor##name(FOLDING_OP_ARGS) { \
    return nullptr; \
}

#define DEFINE_SHL_OP(ty, name, kind) \
static Value *Shl##name(FOLDING_OP_ARGS) { \
    auto value = OperatorWith<ty>::Data(lhs->op()) << OperatorWith<ty>::Data(rhs->op()); \
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->name##Constant(value)); \
}

#define DEFINE_FSHL_OP(ty, name, kind) \
static Value *Shl##name(FOLDING_OP_ARGS) { \
    return nullptr; \
}

#define DEFINE_SHR_OP(ty, name, kind) \
static Value *Shr##name(FOLDING_OP_ARGS) { \
    auto value = OperatorWith<ty>::Data(lhs->op()) >> OperatorWith<ty>::Data(rhs->op()); \
    return Value::New(arena, lhs->source_position(), lhs->type(), ops->name##Constant(value)); \
}

#define DEFINE_FSHR_OP(ty, name, kind) \
static Value *Shr##name(FOLDING_OP_ARGS) { \
    return nullptr; \
}


#define DEFINE_UCMP_OP(ty, name, kind) \
static Value *ICmp##name(FOLDING_OP_ARGS) { \
    bool value = false; \
    auto lval = OperatorWith<ty>::Data(lhs->op()), rval = OperatorWith<ty>::Data(rhs->op()); \
    switch (OperatorWith<IConditionId>::Data(op).value) { \
        case IConditionId::k_eq: \
            value = (lval == rval); \
            break; \
        case IConditionId::k_ne: \
            value = (lval != rval); \
            break; \
        case IConditionId::k_ult: \
            value = (lval < rval); \
            break; \
        case IConditionId::k_ule: \
            value = (lval <= rval); \
            break; \
        case IConditionId::k_ugt: \
            value = (lval > rval); \
            break; \
        case IConditionId::k_uge: \
            value = (lval >= rval); \
            break; \
        case IConditionId::k_sle: \
        case IConditionId::k_slt: \
        case IConditionId::k_sgt: \
        case IConditionId::k_sge: \
        default: \
            return nullptr; \
    } \
    return Value::New(arena, lhs->source_position(), Types::UInt8, ops->U8Constant(value)); \
}

#define DEFINE_ICMP_OP(ty, name, kind) \
static Value *ICmp##name(FOLDING_OP_ARGS) { \
    bool value = false; \
    auto lval = OperatorWith<ty>::Data(lhs->op()), rval = OperatorWith<ty>::Data(rhs->op()); \
    switch (OperatorWith<IConditionId>::Data(op).value) { \
        case IConditionId::k_eq: \
            value = (lval == rval); \
            break; \
        case IConditionId::k_ne: \
            value = (lval != rval); \
            break; \
        case IConditionId::k_slt: \
            value = (lval < rval); \
            break; \
        case IConditionId::k_sle: \
            value = (lval <= rval); \
            break; \
        case IConditionId::k_sgt: \
            value = (lval > rval); \
            break; \
        case IConditionId::k_sge: \
            value = (lval >= rval); \
            break; \
        case IConditionId::k_ule: \
        case IConditionId::k_ult: \
        case IConditionId::k_ugt: \
        case IConditionId::k_uge: \
        default: \
            return nullptr; \
    } \
    return Value::New(arena, lhs->source_position(), Types::UInt8, ops->U8Constant(value)); \
}

#define DEFINE_NCMP_OP(ty, name, kind) \
static Value *ICmp##name(FOLDING_OP_ARGS) { \
    return nullptr; \
}

#define DEFINE_XCMP_OP(ty, name, kind) \
static Value *FCmp##name(FOLDING_OP_ARGS) { \
    return nullptr; \
}

#define DEFINE_FCMP_OP(ty, name, kind) \
static Value *FCmp##name(FOLDING_OP_ARGS) { \
    bool value = false; \
    auto lval = OperatorWith<ty>::Data(lhs->op()), rval = OperatorWith<ty>::Data(rhs->op()); \
    switch (OperatorWith<FConditionId>::Data(op).value) { \
        case FConditionId::k_ueq: \
            value = (lval == rval); \
            break; \
        case FConditionId::k_une: \
            value = (lval != rval); \
            break; \
        case FConditionId::k_ult: \
            value = (lval < rval); \
            break; \
        case FConditionId::k_ule: \
            value = (lval <= rval); \
            break; \
        case FConditionId::k_ugt: \
            value = (lval > rval); \
            break; \
        case FConditionId::k_uge: \
            value = (lval >= rval); \
            break; \
        default: \
            return nullptr; \
    } \
    return Value::New(arena, lhs->source_position(), Types::UInt8, ops->U8Constant(value)); \
}

DECLARE_NUMBER_KINDS(DEFINE_ADD_OP)
DECLARE_NUMBER_KINDS(DEFINE_SUB_OP)
DECLARE_NUMBER_KINDS(DEFINE_MUL_OP)
DECLARE_INTEGRAL_KINDS(DEFINE_DIV_OP)
DECLARE_FLOATING_KINDS(DEFINE_FDIV_OP)
DECLARE_INTEGRAL_KINDS(DEFINE_REM_OP)
DECLARE_INTEGRAL_KINDS(DEFINE_AND_OP)
DECLARE_FLOATING_KINDS(DEFINE_FAND_OP)
DECLARE_INTEGRAL_KINDS(DEFINE_OR_OP)
DECLARE_FLOATING_KINDS(DEFINE_FOR_OP)
DECLARE_INTEGRAL_KINDS(DEFINE_XOR_OP)
DECLARE_FLOATING_KINDS(DEFINE_FXOR_OP)
DECLARE_INTEGRAL_KINDS(DEFINE_SHL_OP)
DECLARE_FLOATING_KINDS(DEFINE_FSHL_OP)
DECLARE_INTEGRAL_KINDS(DEFINE_SHR_OP)
DECLARE_FLOATING_KINDS(DEFINE_FSHR_OP)
DECLARE_UNSIGNED_KINDS(DEFINE_UCMP_OP)
DECLARE_SIGNED_KINDS(DEFINE_ICMP_OP)
DECLARE_FLOATING_KINDS(DEFINE_NCMP_OP)
DECLARE_INTEGRAL_KINDS(DEFINE_XCMP_OP)
DECLARE_FLOATING_KINDS(DEFINE_FCMP_OP)


enum BinaryOpIndex {
#define DEFINE_ENUM(name) k##Op_Index_##name,
    DECLARE_IR_BINARY(DEFINE_ENUM)
    DECLARE_IR_COMPARISON(DEFINE_ENUM)
#undef  DEFINE_ENUM
    kOp_Index_Max,
}; // enum BinaryIndex


enum NumberTyIndex {
#define DEFINE_ENUM(ty, name, kind) k##Ty_Index_##name,
    DECLARE_NUMBER_KINDS(DEFINE_ENUM)
#undef  DEFINE_ENUM
    kTy_Index_Max,
}; // enum NumberTyIndex

static FoldingOperator *kFoldingOperators[kOp_Index_Max][kTy_Index_Max] = {
#define DEFINE_ELEMENT(ty, name, kind) Add##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(Add)
#undef  DEFINE_ELEMENT
    
#define DEFINE_ELEMENT(ty, name, kind) Sub##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(Sub)
#undef  DEFINE_ELEMENT
    
#define DEFINE_ELEMENT(ty, name, kind) Mul##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(Mul)
#undef  DEFINE_ELEMENT
    
#define DEFINE_ELEMENT(ty, name, kind) Div##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(SDiv)
#undef  DEFINE_ELEMENT
    
#define DEFINE_ELEMENT(ty, name, kind) Rem##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(SRem)
#undef  DEFINE_ELEMENT
    
#define DEFINE_ELEMENT(ty, name, kind) Mul##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(UMul)
#undef  DEFINE_ELEMENT

#define DEFINE_ELEMENT(ty, name, kind) Div##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(UDiv)
#undef  DEFINE_ELEMENT

#define DEFINE_ELEMENT(ty, name, kind) Rem##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(URem)
#undef  DEFINE_ELEMENT

#define DEFINE_ELEMENT(ty, name, kind) Add##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(FAdd)
#undef  DEFINE_ELEMENT
    
#define DEFINE_ELEMENT(ty, name, kind) Sub##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(FSub)
#undef  DEFINE_ELEMENT

#define DEFINE_ELEMENT(ty, name, kind) Mul##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(FMul)
#undef  DEFINE_ELEMENT
    
#define DEFINE_ELEMENT(ty, name, kind) Div##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(FDiv)
#undef  DEFINE_ELEMENT
    
#define DEFINE_ELEMENT(ty, name, kind) Rem##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(FRem)
#undef  DEFINE_ELEMENT

#define DEFINE_ELEMENT(ty, name, kind) And##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(And)
#undef  DEFINE_ELEMENT

#define DEFINE_ELEMENT(ty, name, kind) Or##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(Or)
#undef  DEFINE_ELEMENT

#define DEFINE_ELEMENT(ty, name, kind) Xor##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(Xor)
#undef  DEFINE_ELEMENT

#define DEFINE_ELEMENT(ty, name, kind) Shl##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(Shl)
#undef  DEFINE_ELEMENT

#define DEFINE_ELEMENT(ty, name, kind) Shr##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(LShr)
#undef  DEFINE_ELEMENT
    
#define DEFINE_ELEMENT(ty, name, kind) Shr##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(AShr)
#undef  DEFINE_ELEMENT
    
#define DEFINE_ELEMENT(ty, name, kind) ICmp##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(ICmp)
#undef  DEFINE_ELEMENT
    
#define DEFINE_ELEMENT(ty, name, kind) FCmp##name,
    { DECLARE_NUMBER_KINDS(DEFINE_ELEMENT) }, // V(FCmp)
#undef  DEFINE_ELEMENT
};


static BinaryOpIndex ToIndex(const Operator *op) {
    switch (op->value()) {
#define DEFINE_CASE(name) case Operator::k##name: return kOp_Index_##name;
        DECLARE_IR_BINARY(DEFINE_CASE)
        DECLARE_IR_COMPARISON(DEFINE_CASE)
#undef  DEFINE_CASE
        default:
            return kOp_Index_Max;
    }
}

static NumberTyIndex ToIndex(const Type &ty) {
    switch (ty.kind()) {
#define DEFINE_CASE(ty, name, kind) case Type::k##kind: return kTy_Index_##name;
        DECLARE_NUMBER_KINDS(DEFINE_CASE)
#undef  DEFINE_CASE
        default:
            return kTy_Index_Max;
    }
}

Value *ConstantsFoldingPass::FoldValueIfNeeded(Value *input, bool *folded) {
    for (auto i = 0; i < input->op()->value_in(); i++) {
        if (!input->InputValue(i)->op()->IsConstant()) {
            *folded = false;
            return input;
        }
    }

    switch (input->op()->value()) {
#define DEFINE_CASE(name) case Operator::k##name:
        DECLARE_IR_BINARY(DEFINE_CASE)
        DECLARE_IR_COMPARISON(DEFINE_CASE)
#undef  DEFINE_CASE
        {
            auto op_index = ToIndex(input->op());
            auto ty_index = ToIndex(input->InputValue(0)->type());
            if (op_index == kOp_Index_Max || ty_index == kTy_Index_Max) {
                goto not_match;
            }
            auto val = kFoldingOperators[op_index][ty_index](arena(), input->op(), input->InputValue(0),
                                                             input->InputValue(1), ops());
            if (!val) {
                goto not_match;
            }
            *folded = true;
            return val;
        } break;
            
        not_match:
        default:
            *folded = false;
            break;
    }
    return input;
}

} // namespace yalx
