#include "backend/x64/instruction-generating-x64.h"
#include "backend/operand-allocator.h"
#include "backend/constants-pool.h"
#include "backend/linkage-symbols.h"
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
static const int kRootRegister = kR14;
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
    //kR14, // r14 = root
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
                                                 kRootRegister,
                                                 kScratchGeneralRegister,
                                                 -1, // id_of_1
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
    
    class X64MovingDelegate final : public RegisterSavingScope::MovingDelegate {
    public:
        X64MovingDelegate(X64FunctionInstructionSelector *owns): owns_(owns) {}
        ~X64MovingDelegate() override = default;
        
        void MoveTo(InstructionOperand *dest, InstructionOperand *src, ir::Type ty) override {
            owns_->Move(dest, src, ty);
        }
        void Initialize() override {}
        void Finalize() override {}
        DISALLOW_IMPLICIT_CONSTRUCTORS(X64MovingDelegate);
    private:
        X64FunctionInstructionSelector *const owns_;
    }; // class X64MovingDelegate
    
    X64FunctionInstructionSelector(base::Arena *arena,
                                   ConstantsPool *const_pool,
                                   LinkageSymbols *symbols,
                                   InstructionBlockLabelGenerator *labels,
                                   ir::StructureModel *owns,
                                   ir::Function *fun,
                                   bool use_registers_allocation)
    : arena_(arena)
    , const_pool_(const_pool)
    , symbols_(symbols)
    , labels_(labels)
    , owns_(owns)
    , fun_(fun)
    , moving_delegate_(this)
    , operands_(kStackConf.Get(), kRegConf.Get(),
                use_registers_allocation
                ? OperandAllocator::kRegisterFirst
                : OperandAllocator::kStackOnly, arena) {}
    
    DEF_PTR_GETTER(InstructionFunction, bundle);
    
    void Prepare() {
        bundle_ = new (arena_) InstructionFunction(arena_, symbols_->Mangle(fun_->full_name()));
        for (auto bb : fun_->blocks()) {
            bb->RemoveDeads(); // Remove deads again for phi_node_users
            auto ib = bundle_->NewBlock(labels_->NextLable());
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
        
        const auto stack_size = RoundUp(operands_.slots()->max_stack_size(), kStackConf->stack_alignment_size());
        stack_size_->Set32(stack_size);
        for (auto adjust : calling_stack_adjust_) {
            assert(stack_size >= adjust->word32());
            adjust->Set32(stack_size - adjust->word32());
        }
    }
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(X64FunctionInstructionSelector);
private:
    InstructionBlock *current() { return DCHECK_NOTNULL(current_block_); }
    
    void ProcessBasicBlock(ir::BasicBlock *bb) {
        current_block_ = blocks_[bb];
        for (auto user : bb->phi_node_users()) {
            Allocate(user.phi, kMoR);
        }
        for (auto instr : bb->instructions()) {
            operands_.ReleaseDeads(instruction_position_);
            tmps_.clear();
            Select(instr);
            for (auto tmp : tmps_) { operands_.Free(tmp); }
            instruction_position_++;
        }
        for (auto user : bb->phi_node_users()) {
            auto phi_slot = DCHECK_NOTNULL(operands_.Allocated(user.phi));
            auto dest = Allocate(user.dest, kAny);
            Move(phi_slot, dest, user.phi->type());
        }
        current_block_ = nullptr;
    }
    
    
    void Select(ir::Value *val);
    void CallDirectly(ir::Value *val);
    void ProcessParameters(InstructionBlock *block);
    InstructionOperand *CopyArgumentValue(InstructionBlock *block, ir::Type ty, InstructionOperand *from);
    
    InstructionOperand *Allocate(ir::Value *value, Policy policy/*, bool *is_tmp = nullptr*/);
    
    InstructionOperand *Constant(ir::Value *value);
    
    void Move(InstructionOperand *dest, InstructionOperand *src, ir::Type ty);
    
    bool CanDirectlyMove(InstructionOperand *dest, InstructionOperand *src) {
        assert(dest->IsRegister() || dest->IsLocation());
        if (dest->IsRegister() || src->IsRegister()) {
            return true;
        }
        if (dest->IsLocation()) {
            return src->IsImmediate();
        }
        return false;
    }
    
    static size_t ReturningValSizeInBytes(const ir::PrototypeModel *proto) {
        size_t size_in_bytes = 0;
        for (auto ty : proto->return_types()) {
            if (ty.kind() == ir::Type::kVoid) {
                continue;
            }
            size_in_bytes = RoundUp(size_in_bytes, kStackConf->slot_alignment_size());
            size_in_bytes += ty.ReferenceSizeInBytes();
        }
        return size_in_bytes;
    }
    
    static size_t OverflowParametersSizeInBytes(const ir::Function *fun) {
        size_t size_in_bytes = 0;
        int float_count = kNumberOfFloatArgumentsRegisters;
        int general_count = kNumberOfGeneralArgumentsRegisters;
        //bool overflow = false;
        for (auto param : fun->paramaters()) {
            if (param->type().IsFloating()) {
                if (--float_count < 0) {
                    auto size = RoundUp(param->type().ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
                    size_in_bytes += size;
                }
            } else {
                if (--general_count < 0) {
                    auto size = RoundUp(param->type().ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
                    size_in_bytes += size;
                }
            }
        }
        return size_in_bytes;
    }
    
    static size_t OverflowArgumentsSizeInBytes(ir::Value *call, std::vector<ir::Value *> *overflow) {
        size_t size_in_bytes = 0;
        int float_count = kNumberOfFloatArgumentsRegisters;
        int general_count = kNumberOfGeneralArgumentsRegisters;
        for (int i = 0; i < call->op()->value_in(); i++) {
            auto arg = call->InputValue(i);
            if (arg->type().IsFloating()) {
                if (--float_count < 0) {
                    auto size = RoundUp(arg->type().ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
                    size_in_bytes += size;
                    overflow->push_back(arg);
                }
            } else {
                if (--general_count < 0) {
                    auto size = RoundUp(arg->type().ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
                    size_in_bytes += size;
                    overflow->push_back(arg);
                }
            }
        }
        return size_in_bytes;
    }
    
    base::Arena *const arena_;
    ir::StructureModel *const owns_;
    ir::Function *const fun_;
    ConstantsPool *const const_pool_;
    LinkageSymbols *const symbols_;
    InstructionBlockLabelGenerator *const labels_;
    X64MovingDelegate moving_delegate_;
    OperandAllocator operands_;
    InstructionFunction *bundle_ = nullptr;
    InstructionBlock *current_block_ = nullptr;
    int instruction_position_ = 0;
    ImmediateOperand *stack_size_ = nullptr;
    std::map<ir::BasicBlock *, InstructionBlock *> blocks_;
    std::vector<InstructionOperand *> tmps_;
    std::vector<ImmediateOperand *> calling_stack_adjust_;
}; // class X64FunctionInstructionSelector

void X64FunctionInstructionSelector::Select(ir::Value *val) {
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
            
        case ir::Operator::kLoadEffectField: {
            UNREACHABLE();
        } break;
            
        case ir::Operator::kLoadEffectAddress: {
            UNREACHABLE();
        } break;
            
        case ir::Operator::kAdd: {
            auto opd = Allocate(val, kMoR);
            auto lhs = Allocate(val->InputValue(0), kAny);
            auto rhs = Allocate(val->InputValue(1), kAny);
            Move(opd, lhs, val->type());
            switch (ToMachineRepresentation(val->type())) {
                case MachineRepresentation::kWord8:
                    current()->NewIO(X64Add8, opd, rhs);
                    break;
                case MachineRepresentation::kWord16:
                    current()->NewIO(X64Add16, opd, rhs);
                    break;
                case MachineRepresentation::kWord32:
                    current()->NewIO(X64Add32, opd, rhs);
                    break;
                case MachineRepresentation::kWord64:
                    current()->NewIO(X64Add, opd, rhs);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case ir::Operator::kFAdd: {
            auto opd = Allocate(val, kMoR);
            auto lhs = Allocate(val->InputValue(0), kAny);
            auto rhs = Allocate(val->InputValue(1), kAny);
            Move(opd, lhs, val->type());
            switch (ToMachineRepresentation(val->type())) {
                case MachineRepresentation::kFloat32:
                    current()->NewIO(SSEFloat32Add, opd, rhs);
                    break;
                case MachineRepresentation::kFloat64:
                    current()->NewIO(SSEFloat64Add, opd, rhs);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case ir::Operator::kLoadFunAddr: {
            auto fun = ir::OperatorWith<const ir::Function *>::Data(val->op());
            auto symbol = symbols_->Mangle(fun->full_name());
            auto rel = bundle()->AddExternalSymbol(symbol, true/*fetch_address*/);
            auto opd = Allocate(val, kAny);

            Move(opd, rel, val->type());
        } break;
            
        case ir::Operator::kClosure: {
            // TODO:
            
        } break;;
            
        case ir::Operator::kCallRuntime: {
            // TODO:
            // PkgInitOnce -> pkg_init_once(void *fun, const char *name)
        } break;
            
        case ir::Operator::kCallDirectly:
            CallDirectly(val);
            break;
            
        case ir::Operator::kReturningVal:
            // Just Ignore this ir code
            break;
            
            //
            // +------------------+
            // | returning val[0] | <- bp + 16 + overflow-args-size + returning-val-size
            // +------------------+
            // | returning val[1] |
            // +------------------+
            // | returning val[2] |
            // +------------------+
            // |    ... ...       |
            // +------------------+
            // | overflow argv[0] | <- bp + 16 + overflow-args-size
            // +------------------+
            // | overflow argv[1] |
            // +------------------+
            // | overflow argv[2] |
            // +------------------+
            // |    ... ...       | <- bp + 16
            // +------------------+
            // | returning addr   | <- bp + 8
            // +------------------+
            // | saved bp         | <- bp + 0
            // +------------------+
            //
        case ir::Operator::kRet: {
            //printd("%s", fun_->full_name()->data());
            auto overflow_args_size = OverflowParametersSizeInBytes(fun_);
            auto returning_val_size = ReturningValSizeInBytes(fun_->prototype());
            auto caller_saving_size = RoundUp(overflow_args_size + returning_val_size,
                                              kStackConf->stack_alignment_size());
            auto caller_padding_size = caller_saving_size - returning_val_size - overflow_args_size;
            auto returning_val_offset = kPointerSize * 2 + caller_padding_size + overflow_args_size;
            // padding = 8   returning-vals = 12 offset = 24
            // padding = 12  returning-vals = 4  offset = 28
            //for (int i = 0; i < val->op()->value_in(); i++) {
            for (int i = val->op()->value_in() - 1; i >= 0; i--) {
                auto ty = fun_->prototype()->return_type(i);
                if (ty.kind() == ir::Type::kVoid) {
                    continue;
                }
                auto ret = Allocate(val->InputValue(i), kAny);
                auto opd = new (arena_) LocationOperand(X64Mode_MRI, rbp.code(), 0, returning_val_offset);
                returning_val_offset += RoundUp(ty.ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
                Move(opd, ret, ty);
            }
            
            current()->NewIO(X64Add, operands_.registers()->stack_pointer(), stack_size_);
            current()->NewO(X64Pop, operands_.registers()->frame_pointer());
            current()->New(ArchRet);
        } break;
            
        default:
            UNREACHABLE();
            break;
    }
}

// --------------
// return address |
// --------------
//    saved RBP   |
// --------------
//                |      -8
// --------------
//                |      -16
// --------------
//                |      -24
// --------------
//                | +32  -32  returning_vals[0] a
// --------------   +28  -36  returning_vals[1] b
//                | +24  -40  returning_vals[2]
// --------------
//                | +16  -48
// --------------
// return address | +8
// --------------
//    saved RBP   | 0
// --------------
void X64FunctionInstructionSelector::CallDirectly(ir::Value *val) {
    auto callee = ir::OperatorWith<ir::Function *>::Data(val->op());
    std::vector<ir::Value *> returning_vals;
    returning_vals.push_back(val);
    for (auto edge : val->users()) {
        if (edge.user->Is(ir::Operator::kReturningVal)) {
            returning_vals.push_back(edge.user);
        }
    }
    if (returning_vals.size() > 1) {
        std::sort(returning_vals.begin() + 1, returning_vals.end(), [](ir::Value *v1, ir::Value *v2) {
            return ir::OperatorWith<int>::Data(v1) < ir::OperatorWith<int>::Data(v2);
        });
    }
    std::vector<ir::Value *> overflow_args;
    RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
    size_t overflow_args_size = 0;
    int general_index = 0, float_index = 0;
    for (int i = 0; i < val->op()->value_in(); i++) {
        auto arg = val->InputValue(i);
        const auto size = RoundUp(arg->type().ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
        if (arg->type().IsFloating()) {
            if (float_index < kNumberOfFloatArgumentsRegisters) {
                saving_scope.AddExclude(arg, kFloatArgumentsRegisters[float_index], instruction_position_);
            } else {
                // overflow
                overflow_args_size += size;
                overflow_args.push_back(arg);
            }
            float_index++;
        } else {
            if (general_index < kNumberOfGeneralArgumentsRegisters) {
                saving_scope.AddExclude(arg, kGeneralArgumentsRegisters[general_index], instruction_position_);
            } else {
                // overflow
                overflow_args_size += size;
                overflow_args.push_back(arg);
            }
            general_index++;
        }
    }
    saving_scope.SaveAll();

    general_index = 0, float_index = 0;
    for (int i = 0; i < val->op()->value_in(); i++) {
        auto arg = val->InputValue(i);
        auto opd = Allocate(arg, kAny);
        if (arg->type().IsFloating()) {
            if (float_index < kNumberOfFloatArgumentsRegisters) {
                if (saving_scope.Include(kFloatArgumentsRegisters[float_index], false/*general*/)) {
                    auto dest = DCHECK_NOTNULL(operands_.AllocateReigster(arg->type(),
                                                                          kFloatArgumentsRegisters[float_index]));
                    Move(dest, opd, arg->type());
                    operands_.Associate(arg, dest);
                }
            }
            float_index++;
        } else {
            if (general_index < kNumberOfGeneralArgumentsRegisters) {
                if (saving_scope.Include(kGeneralArgumentsRegisters[general_index], true/*general*/)) {
                    auto dest = DCHECK_NOTNULL(operands_.AllocateReigster(arg->type(),
                                                                          kGeneralArgumentsRegisters[general_index]));
                    Move(dest, opd, arg->type());
                    operands_.Associate(arg, dest);
                }
            }
            general_index++;
        }
    }
    
    auto current_stack_size = operands_.slots()->stack_size();
    for (auto rv : returning_vals) {
        if (rv->type().kind() != ir::Type::kVoid) {
            operands_.AllocateStackSlot(rv, 0/*padding_size*/, StackSlotAllocator::kLinear);
        }
    }
    
    if (overflow_args_size > 0) {
        for (auto arg : overflow_args) {
            auto opd = operands_.AllocateStackSlot(arg->type(), 0/*padding_size*/, StackSlotAllocator::kLinear);
            tmps_.push_back(opd);
            Move(opd, Allocate(arg, kAny), arg->type());
        }
        
    }
    
    current_stack_size += ReturningValSizeInBytes(callee->prototype());
    current_stack_size += overflow_args_size;
    current_stack_size = RoundUp(current_stack_size, kStackConf->stack_alignment_size());
    
    auto adjust = ImmediateOperand::Word32(arena_, static_cast<int>(current_stack_size));
    calling_stack_adjust_.push_back(adjust);
    current()->NewIO(X64Add, operands_.registers()->stack_pointer(), adjust);
    auto rel = new (arena_) ReloactionOperand(symbols_->Mangle(callee->full_name()), nullptr);
    current()->NewI(ArchCall, rel);
    current()->NewIO(X64Sub, operands_.registers()->stack_pointer(), adjust);
}

void X64FunctionInstructionSelector::ProcessParameters(InstructionBlock *block) {
    auto returning_val_size = ReturningValSizeInBytes(fun_->prototype());
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
                operands_.Associate(param, opd);
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
                                          0/*padding_size*/, StackSlotAllocator::kFit);
    if (ty.ReferenceSizeInBytes() > 64) {
        auto rel = bundle_->AddExternalSymbol(kLibc_memcpy);
        USE(rel);
        
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
                auto tmp = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord8);
                current()->NewIO(X64Movb, tmp, src);
                current()->NewIO(X64Movb, dest, src);
            }
            break;
            
        case ir::Type::kInt16:
        case ir::Type::kUInt16:
            if (CanDirectlyMove(dest, src)) {
                current()->NewIO(X64Movw, dest, src);
            } else {
                auto tmp = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord16);
                current()->NewIO(X64Movw, tmp, src);
                current()->NewIO(X64Movw, dest, tmp);
            }
            break;
            
        case ir::Type::kInt32:
        case ir::Type::kUInt32:
            if (CanDirectlyMove(dest, src)) {
                current()->NewIO(X64Movl, dest, src);
            } else {
                auto tmp = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord32);
                current()->NewIO(X64Movl, tmp, src);
                current()->NewIO(X64Movl, dest, tmp);
            }
            break;
            
        case ir::Type::kInt64:
        case ir::Type::kUInt64:
        case ir::Type::kReference:
        case ir::Type::kString:
            if (CanDirectlyMove(dest, src)) {
                current()->NewIO(X64Movq, dest, src);
            } else {
                auto tmp = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
                current()->NewIO(X64Movq, tmp, src);
                current()->NewIO(X64Movq, dest, tmp);
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
                    auto tmp = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
                    current()->NewIO(X64Movq, tmp, src);
                    current()->NewIO(X64Movq, dest, tmp);
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

X64InstructionGenerator::X64InstructionGenerator(base::Arena *arena, ir::Module *module, ConstantsPool *const_pool,
                                                 LinkageSymbols *symbols, int optimizing_level)
: arena_(arena)
, module_(module)
, const_pool_(const_pool)
, symbols_(symbols)
, optimizing_level_(optimizing_level)
, funs_(arena)
, lables_(new InstructionBlockLabelGenerator()) {
    kRegConf.Get();
    kStackConf.Get();
}

X64InstructionGenerator::~X64InstructionGenerator() {
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
    X64FunctionInstructionSelector selector(arena_, const_pool_, symbols_, lables_.get(), owns, fun,
                                            optimizing_level_ > 0 /*use_registers_allocation*/);
    selector.Prepare();
    selector.Run();
    funs_[fun->full_name()->ToSlice()] = selector.bundle();
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
