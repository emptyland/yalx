#include "backend/x64/instruction-generating-x64.h"
#include "backend/operand-allocator.h"
#include "backend/constants-pool.h"
#include "backend/instruction.h"
#include "x64/asm-x64.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/type.h"
#include "ir/operator.h"
#include "base/arena-utils.h"
#include "base/lazy-instance.h"

namespace yalx {

namespace backend {

using namespace x64;

//RAX, RCX,
//RDX, RSI,
//RDI,
//R8-R11,
//ST(0)-ST(7) K0-K7,
//XMM0-XMM15, YMM0-YMM15 ZMM0-ZMM31
static const int kScratchGeneralRegister = kR13;
static const int kScratchFloatRegister   = xmm13.code();
static const int kScratchDoubleRegister  = xmm13.code();

static const int kAllocatableGeneralRegisters[] = {
    kRAX,
    kRCX,
    kRDX,
    kRSI,
    kRDI,
    kR8,
    kR9,  // 9
    kR10, // 10
    kR11,
    kR12,
    //kR13, // r13 = scratch
    kR14, // 14
    kR15,
};

static const int kCalleeSaveRegisters[] = {
    kRBX,
    kRBP,
    kR12,
    kR13,
    kR14,
    kR15,
};

static const int kAllocatableFloatRegisters[] = {
    xmm0.code(),
    xmm1.code(),
    xmm2.code(),
    xmm3.code(),
    xmm4.code(),
    xmm5.code(),
    xmm6.code(),
    xmm7.code(),
    xmm8.code(),
    xmm9.code(),
    xmm10.code(),
    xmm11.code(),
    xmm12.code(),
    //xmm13.code(), = scratch
    xmm14.code(),
    xmm15.code(),
};

static const int kAllocatableDoubleRegisters[] = {
    xmm0.code(),
    xmm1.code(),
    xmm2.code(),
    xmm3.code(),
    xmm4.code(),
    xmm5.code(),
    xmm6.code(),
    xmm7.code(),
    xmm8.code(),
    xmm9.code(),
    xmm10.code(),
    xmm11.code(),
    xmm12.code(),
    //xmm13.code(), = scratch
    xmm14.code(),
    xmm15.code(),
};

static const int kGeneralArgumentsRegisters[] = {
    Argv_0.code(),
    Argv_1.code(),
    Argv_2.code(),
    Argv_3.code(),
    Argv_4.code(),
    Argv_5.code(),
    Argv_6.code(),
    Argv_7.code(),
};

static const int kFloatArgumentsRegisters[] = {
    xmm0.code(),
    xmm1.code(),
    xmm2.code(),
    xmm3.code(),
    xmm4.code(),
    xmm5.code(),
    xmm6.code(),
    xmm7.code(),
};

constexpr static const size_t kNumberOfGeneralArgumentsRegisters = arraysize(kGeneralArgumentsRegisters);
constexpr static const size_t kNumberOfFloatArgumentsRegisters = arraysize(kFloatArgumentsRegisters);

struct X64RegisterConfigurationInitializer {
    static RegisterConfiguration *New(void *chunk) {
        return new (chunk) RegisterConfiguration(rbp.code()/*fp*/,
                                                 rsp.code()/*sp*/,
                                                 kScratchGeneralRegister,
                                                 kScratchFloatRegister,
                                                 kScratchDoubleRegister,
                                                 MachineRepresentation::kWord64,
                                                 16/*number_of_general_registers*/,
                                                 16/*number_of_float_registers*/,
                                                 16/*number_of_double_registers*/,
                                                 kAllocatableGeneralRegisters,
                                                 arraysize(kAllocatableGeneralRegisters),
                                                 kAllocatableFloatRegisters,
                                                 arraysize(kAllocatableFloatRegisters),
                                                 kAllocatableDoubleRegisters,
                                                 arraysize(kAllocatableDoubleRegisters));
    }
    
    static void Delete(void *) {}
};

struct X64StackConfigurationInitializer {
    static StackConfiguration *New(void *chunk) {
        return new (chunk) StackConfiguration(X64Mode_MRI,
                                              32, // saved size
                                              4,  // slot alignment size
                                              16, // stack alignment size
                                              rbp.code(),
                                              rsp.code());
    }
    
    static void Delete(void *) {}
};

static base::LazyInstance<RegisterConfiguration, X64RegisterConfigurationInitializer> kRegConf;
static base::LazyInstance<StackConfiguration, X64StackConfigurationInitializer>       kStackConf;

class X64FunctionInstructionSelector final {
public:
    enum Policy {
        kMoR, // in memory or register
        kRel, // can be relocation
        kAny, // can be a immediate number
    };
    
    X64FunctionInstructionSelector(base::Arena *arena,
                                   ConstantsPool *const_pool,
                                   ir::StructureModel *owns,
                                   ir::Function *fun,
                                   bool use_registers_allocation)
    : arena_(arena)
    , const_pool_(const_pool)
    , owns_(owns)
    , fun_(fun)
    , operands_(kStackConf.Get(), kRegConf.Get(),
                use_registers_allocation
                ? OperandAllocator::kRegisterFirst
                : OperandAllocator::kStackOnly, arena)
    , blocks_(arena) {}
    
    void Prepare() {
        int label = 0;
        for (auto bb : fun_->blocks()) {
            bb->RemoveDeads(); // Remove deads again for phi_node_users
            auto ib = new (arena_) InstructionBlock(arena_, label++);
            blocks_[bb] = ib;
        }
        operands_.Prepare(fun_);
    }
    
    void Run() {
        auto blk = blocks_[fun_->entry()];
        blk->NewI(X64Push, operands_.registers()->frame_pointer()); // push rbp
        // movq rsp->rbp
        blk->NewIO(X64Movq, operands_.registers()->frame_pointer(), operands_.registers()->stack_pointer());
        stack_size_ = ImmediateOperand::Word32(arena_, 0);
        blk->NewIO(X64Sub, operands_.registers()->stack_pointer(), stack_size_);
        
        ProcessParameters(blk);
        for (auto blk : fun_->blocks()) {
            ProcessBasicBlock(blk);
        }
        
        stack_size_->Set32(static_cast<int32_t>(operands_.slots()->max_stack_size()));
    }
    
private:
    InstructionBlock *current() { return DCHECK_NOTNULL(current_block_); }
    
    void ProcessBasicBlock(ir::BasicBlock *bb) {
        current_block_ = blocks_[bb];
        for (auto user : bb->phi_node_users()) {
            Allocate(user.phi, kMoR);
        }
        for (auto instr : bb->instructions()) {
            operands_.ReleaseDeads(instruction_position_++);
            tmps_.clear();
            Select(instr);
            for (auto tmp : tmps_) { operands_.Free(tmp); }
        }
        for (auto user : bb->phi_node_users()) {
            auto phi_slot = DCHECK_NOTNULL(operands_.Allocated(user.phi));
            auto dest = Allocate(user.dest, kAny);
            Move(phi_slot, dest, user.phi->type());
        }
        current_block_ = nullptr;
    }
    
    
    void Select(ir::Value *val);
    void ProcessParameters(InstructionBlock *block);
    InstructionOperand *CopyArgumentValue(InstructionBlock *block, ir::Type ty, InstructionOperand *from);
    
    InstructionOperand *Allocate(ir::Value *value, Policy policy/*, bool *is_tmp = nullptr*/);
    
    InstructionOperand *Constant(ir::Value *value);
    
    void Move(InstructionOperand *dest, InstructionOperand *src, ir::Type ty);
    
    bool CanDirectlyMove(InstructionOperand *dest, InstructionOperand *src) {
        assert(dest->IsRegister() || dest->IsLocation());
        if (dest->IsRegister()) {
            return true;
        }
        if (dest->IsLocation()) {
            return src->IsImmediate();
        }
        return false;
    }
    
    size_t ComputeReturningValSizeInBytes() {
        size_t bytes = 0;
        for (auto ty : fun_->prototype()->return_types()) {
            bytes += ty.ReferenceSizeInBytes();
        }
        return bytes;
    }
    
    base::Arena *const arena_;
    ir::StructureModel *const owns_;
    ir::Function *const fun_;
    ConstantsPool *const const_pool_;
    OperandAllocator operands_;
    base::ArenaMap<ir::BasicBlock *, InstructionBlock *> blocks_;
    InstructionBlock *current_block_ = nullptr;
    std::vector<InstructionOperand *> tmps_;
    int instruction_position_ = 0;
    ImmediateOperand *stack_size_ = nullptr;
}; // class X64FunctionInstructionSelector

void X64FunctionInstructionSelector::Select(ir::Value *val) {
    InstructionOperand *opd = nullptr;
    if (val->type().kind() != ir::Type::kVoid) {
        assert(!val->op()->IsConstant());
        opd = Allocate(val, kMoR);
    }
    switch (val->op()->value()) {
        case ir::Operator::kBr: {
            if (val->op()->value_in() == 0) {
                auto ib = blocks_[val->OutputControl(0)];
                auto label = new (arena_) ReloactionOperand(nullptr/*symbol_name*/, ib);
                current()->NewI(ArchJmp, label);
                current()->New(ArchNop);
                current()->AddSuccessor(ib);
                ib->AddPredecessors(current());
                return;
            }
            // TODO: condition br
            UNREACHABLE();
        } break;
            
        case ir::Operator::kAdd: {
            auto lhs = Allocate(val->InputValue(0), kAny);
            auto rhs = Allocate(val->InputValue(1), kAny);
            Move(opd, lhs, val->type());
            current()->NewIO(X64Add, opd, rhs);
        } break;
            
        case ir::Operator::kRet: {
            
            current()->New(ArchRet);
        } break;
            
        default:
            UNREACHABLE();
            break;
    }
}

void X64FunctionInstructionSelector::ProcessParameters(InstructionBlock *block) {
    auto returning_val_size = ComputeReturningValSizeInBytes();
    int number_of_float_args = kNumberOfFloatArgumentsRegisters;
    int number_of_general_args = kNumberOfGeneralArgumentsRegisters;
    for (auto param : fun_->paramaters()) {
        auto ty = param->type();
        if (ty.IsFloating()) {
            auto index = kNumberOfFloatArgumentsRegisters - number_of_float_args;
            auto arg = operands_.AllocateReigster(param, kFloatArgumentsRegisters[index]);
            assert(arg != nullptr);
            number_of_float_args--;
        } else {
            auto index = kNumberOfGeneralArgumentsRegisters - number_of_general_args;
            auto arg = operands_.AllocateReigster(param, kGeneralArgumentsRegisters[index]);
            assert(arg != nullptr);
            if (param->type().kind() == ir::Type::kValue && !param->type().IsPointer()) {
                auto opd = CopyArgumentValue(block, param->type(), arg);
                operands_.Move(param, opd);
            }
            
            number_of_general_args--;
        }
    }
}

InstructionOperand *X64FunctionInstructionSelector::CopyArgumentValue(InstructionBlock *block, ir::Type ty,
                                                                      InstructionOperand *from) {
    assert(ty.kind() == ir::Type::kValue);
    assert(!ty.IsPointer());
    
    auto to = operands_.AllocateStackSlot(OperandAllocator::kVal, ty.ReferenceSizeInBytes(),
                                          StackSlotAllocator::kFit);
    if (ty.ReferenceSizeInBytes() > 64) {
        // TODO: use memcpy
        UNREACHABLE();
    } else {
        auto bytes = ty.ReferenceSizeInBytes();
        auto tmp = DCHECK_NOTNULL(operands_.AllocateReigster(OperandAllocator::kPtr, kPointerSize, rax.code()));
        if (from->IsRegister()) {
            for (int i = 0; i < bytes / 8; i++) {
                auto input = new (arena_) LocationOperand(X64Mode_MRI, from->AsRegister()->register_id(), 0, i * 8);
                block->NewIO(X64Movq, tmp, input);
                auto output = new (arena_) LocationOperand(X64Mode_MRI, to->register0_id(), 0, to->k() + i * 8);
                block->NewIO(X64Movq, output, tmp);
                bytes -= 8;
            }
        } else {
            assert(from->IsLocation());
            
            for (int i = 0; i < bytes / 8; i++) {
                auto input = new (arena_) LocationOperand(X64Mode_MRI, from->AsLocation()->register0_id(), 0,
                                                          from->AsLocation()->k() + i * 8);
                block->NewIO(X64Movq, tmp, input);
                auto output = new (arena_) LocationOperand(X64Mode_MRI, to->register0_id(), 0, to->k() + i * 8);
                block->NewIO(X64Movq, output, tmp);
                bytes -= 8;
            }
            
        }
        operands_.Free(tmp);
    }
    return to;
}

InstructionOperand *X64FunctionInstructionSelector::Allocate(ir::Value *value, Policy policy) {
    if (auto opd = operands_.Allocated(value)) {
        return opd;
    }
    switch (policy) {
        case kMoR: {
            if (value->op()->IsConstant()) {
                InstructionOperand *opd = operands_.Allocate(value->type());
                auto imm = Constant(value);
                Move(opd, imm, value->type());
                tmps_.push_back(opd);
                return opd;
            }
        } break;
            
        case kRel: {
            if (value->op()->IsConstant()) {
                auto imm = Constant(value);
                if (imm->IsConstant() || imm->IsReloaction()) {
                    return imm;
                }
                InstructionOperand *opd = operands_.Allocate(value->type());
                Move(opd, imm, value->type());
                tmps_.push_back(opd);
                return opd;
            }
        } break;
            
        case kAny: {
            if (value->op()->IsConstant()) {
                return Constant(value);
            }
        } break;
            
        default:
            UNREACHABLE();
            break;
    }
    return operands_.Allocate(value);
}

InstructionOperand *X64FunctionInstructionSelector::Constant(ir::Value *value) {
    switch (value->op()->value()) {
        case ir::Operator::kI8Constant:
        case ir::Operator::kU8Constant:
            return ImmediateOperand::Word8(arena_, ir::OperatorWith<int8_t>::Data(value->op()));
            
        case ir::Operator::kI16Constant:
        case ir::Operator::kU16Constant:
            return ImmediateOperand::Word16(arena_, ir::OperatorWith<int16_t>::Data(value->op()));
            
        case ir::Operator::kI32Constant:
        case ir::Operator::kU32Constant:
            return ImmediateOperand::Word32(arena_, ir::OperatorWith<int32_t>::Data(value->op()));
            
        case ir::Operator::kI64Constant:
        case ir::Operator::kU64Constant: {
            auto id = const_pool_->FindOrInsertWord64(ir::OperatorWith<uint64_t>::Data(value->op()));
            return new (arena_) ConstantOperand(ConstantOperand::kNumber, id);
        } break;
            
        case ir::Operator::kF32Constant: {
            auto id = const_pool_->FindOrInsertFloat32(ir::OperatorWith<float>::Data(value->op()));
            return new (arena_) ConstantOperand(ConstantOperand::kNumber, id);
        } break;
            
        case ir::Operator::kF64Constant: {
            auto id = const_pool_->FindOrInsertFloat64(ir::OperatorWith<double>::Data(value->op()));
            return new (arena_) ConstantOperand(ConstantOperand::kNumber, id);
        } break;
            
        case ir::Operator::kStringConstant: {
            auto id = const_pool_->FindOrInsertString(ir::OperatorWith<const String *>::Data(value->op()));
            return new (arena_) ConstantOperand(ConstantOperand::kString, id);
        } break;
            
        case ir::Operator::kNilConstant:
            return ImmediateOperand::Word64(arena_, 0);
            
        default:
            UNREACHABLE();
            break;
    }
}

void X64FunctionInstructionSelector::Move(InstructionOperand *dest, InstructionOperand *src, ir::Type ty) {
    assert(dest->IsRegister() || dest->IsLocation());
    switch (ty.kind()) {
        case ir::Type::kInt8:
        case ir::Type::kUInt8:
            if (CanDirectlyMove(dest, src)) {
                current()->NewIO(X64Movb, dest, src);
            } else {
                auto tmp = operands_.registers()->GeneralScratch(MachineRepresentation::kWord8);
                current()->NewIO(X64Movb, tmp, src);
                current()->NewIO(X64Movb, dest, src);
            }
            break;
            
        case ir::Type::kInt16:
        case ir::Type::kUInt16:
            if (CanDirectlyMove(dest, src)) {
                current()->NewIO(X64Movw, dest, src);
            } else {
                auto tmp = operands_.registers()->GeneralScratch(MachineRepresentation::kWord16);
                current()->NewIO(X64Movw, tmp, src);
                current()->NewIO(X64Movw, dest, src);
            }
            break;
            
        case ir::Type::kInt32:
        case ir::Type::kUInt32:
            if (CanDirectlyMove(dest, src)) {
                current()->NewIO(X64Movl, dest, src);
            } else {
                auto tmp = operands_.registers()->GeneralScratch(MachineRepresentation::kWord32);
                current()->NewIO(X64Movl, tmp, src);
                current()->NewIO(X64Movl, dest, src);
            }
            break;
            
        case ir::Type::kInt64:
        case ir::Type::kUInt64:
        case ir::Type::kReference:
        case ir::Type::kString:
            if (CanDirectlyMove(dest, src)) {
                current()->NewIO(X64Movq, dest, src);
            } else {
                auto tmp = operands_.registers()->GeneralScratch(MachineRepresentation::kWord64);
                current()->NewIO(X64Movq, tmp, src);
                current()->NewIO(X64Movq, dest, src);
            }
            break;
            
        case ir::Type::kFloat32:
            if (CanDirectlyMove(dest, src)) {
                current()->NewIO(X64Movss, dest, src);
            } else {
                auto tmp = operands_.registers()->float_scratch();
                current()->NewIO(X64Movss, tmp, src);
                current()->NewIO(X64Movss, dest, tmp);
            }
            break;
            
        case ir::Type::kFloat64:
            if (CanDirectlyMove(dest, src)) {
                current()->NewIO(X64Movsd, dest, src);
            } else {
                auto tmp = operands_.registers()->double_scratch();
                current()->NewIO(X64Movsd, tmp, src);
                current()->NewIO(X64Movsd, dest, tmp);
            }
            
            break;
            
        case ir::Type::kValue: {
            if (ty.IsPointer()) {
                if (CanDirectlyMove(dest, src)) {
                    current()->NewIO(X64Movq, dest, src);
                } else {
                    auto tmp = operands_.registers()->GeneralScratch(MachineRepresentation::kWord64);
                    current()->NewIO(X64Movq, tmp, src);
                    current()->NewIO(X64Movq, dest, src);
                }
                return;
            }
            
            // TODO:
            UNREACHABLE();
        } break;
            
            
        default:
            UNREACHABLE();
            break;
    }
}

X64InstructionGenerator::X64InstructionGenerator(base::Arena *arena, ir::Module *module, ConstantsPool *const_pool)
: arena_(arena)
, module_(module)
, const_pool_(const_pool) {
    kRegConf.Get();
    kStackConf.Get();
}

void X64InstructionGenerator::Run() {
    PrepareGlobalValues();
    
    for (auto udt : module_->structures()) {
        for (auto method : udt->methods()) {
            GenerateFun(udt, method.fun);
        }
    }
    for (auto fun : module_->funs()) {
        GenerateFun(nullptr/*owns*/, fun);
    }
}

void X64InstructionGenerator::GenerateFun(ir::StructureModel *owns, ir::Function *fun) {
    X64FunctionInstructionSelector selector(arena_, const_pool_, owns, fun, false/*use_registers_allocation*/);
    selector.Prepare();
    selector.Run();
}

void X64InstructionGenerator::PrepareGlobalValues() {
    for (auto global_val : module_->values()) {
        if (global_val->op()->IsConstant()) {
            UniquifyConstant(global_val);
        } else {
            
        }
    }
}

std::tuple<int, bool> X64InstructionGenerator::UniquifyConstant(ir::Value *kval) {
    int id = 0;
    bool is_string = false;
    switch (kval->op()->value()) {
        case ir::Operator::kWord8Constant:
        case ir::Operator::kU8Constant:
        case ir::Operator::kI8Constant:
            id = const_pool_->FindOrInsertWord8(ir::OperatorWith<uint8_t>::Data(kval->op()));
            break;
            
        case ir::Operator::kWord16Constant:
        case ir::Operator::kU16Constant:
        case ir::Operator::kI16Constant:
            id = const_pool_->FindOrInsertWord16(ir::OperatorWith<uint16_t>::Data(kval->op()));
            break;
            
        case ir::Operator::kWord32Constant:
        case ir::Operator::kU32Constant:
        case ir::Operator::kI32Constant:
            id = const_pool_->FindOrInsertWord32(ir::OperatorWith<uint32_t>::Data(kval->op()));
            break;
            
        case ir::Operator::kWord64Constant:
        case ir::Operator::kU64Constant:
        case ir::Operator::kI64Constant:
            id = const_pool_->FindOrInsertWord64(ir::OperatorWith<uint64_t>::Data(kval->op()));
            break;
            
        case ir::Operator::kF32Constant:
            id = const_pool_->FindOrInsertFloat32(ir::OperatorWith<float>::Data(kval->op()));
            break;
            
        case ir::Operator::kF64Constant:
            id = const_pool_->FindOrInsertFloat64(ir::OperatorWith<double>::Data(kval->op()));
            break;
            
        case ir::Operator::kStringConstant:
            id = const_pool_->FindOrInsertString(ir::OperatorWith<const String *>::Data(kval->op()));
            is_string = true;
            break;
            
        default:
            UNREACHABLE();
            break;
    }
    return std::make_tuple(id, is_string);
}

} // namespace backend

} // namespace yalx
