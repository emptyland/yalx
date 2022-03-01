#include "backend/arm64/instruction-generating-arm64.h"
#include "backend/stackslot-allocator.h"
#include "backend/operand-allocator.h"
#include "backend/linkage-symbols.h"
#include "backend/constants-pool.h"
#include "backend/instruction.h"
#include "arm64/asm-arm64.h"
#include "ir/condition.h"
#include "ir/metadata.h"
#include "ir/operator.h"
#include "ir/runtime.h"
#include "ir/node.h"
#include "ir/type.h"
#include "base/lazy-instance.h"
#include "base/arena-utils.h"
#include "base/format.h"

namespace yalx {
namespace backend {


// SP The Stack Pointer.
// r30 LR The Link Register.
// r29 FP The Frame Pointer
// r19…r28 Callee-saved registers
// r18 The Platform Register, if needed; otherwise a temporary register. See notes.
// r17 IP1 The second intra-procedure-call temporary register (can be used by call veneers and PLT code); at other times
//         may be used as a temporary register.
// r16 IP0 The first intra-procedure-call scratch register (can be used by call veneers and
//         PLT code); at other times may be used as a temporary register.
// r9…r15 Temporary registers
// r8 Indirect result location register
// r0…r7 Parameter/result registers

static const int kRootRegister            = arm64::kRootRegister.code(); // x26
static const int kScratchGeneralRegister0 = arm64::x19.code();
static const int kScratchGeneralRegister1 = arm64::x20.code();
static const int kScratchFloatRegister    = arm64::s19.code();
static const int kScratchDoubleRegister   = arm64::d19.code();

static const int kAllocatableGeneralRegisters[] = {
#define DEFINE_CODE(name) arm64::name.code(),
    ALWAYS_ALLOCATABLE_GENERAL_REGISTERS(DEFINE_CODE)
#undef  DEFINE_CODE
};

static const int kAllocatableFloatRegisters[] = {
#define DEFINE_CODE(name) arm64::name.code(),
    ALLOCATABLE_DOUBLE_REGISTERS(DEFINE_CODE)
#undef  DEFINE_CODE
};

static const int kAllocatableDoubleRegisters[] = {
#define DEFINE_CODE(name) arm64::name.code(),
    ALLOCATABLE_DOUBLE_REGISTERS(DEFINE_CODE)
#undef  DEFINE_CODE
};

static const int kGeneralArgumentsRegisters[] = {
    arm64::x0.code(),
    arm64::x1.code(),
    arm64::x3.code(),
    arm64::x4.code(),
    arm64::x5.code(),
    arm64::x6.code(),
    arm64::x7.code(),
};

static const int kFloatArgumentsRegisters[] = {
    arm64::s0.code(),
    arm64::s1.code(),
    arm64::s2.code(),
    arm64::s3.code(),
    arm64::s4.code(),
    arm64::s5.code(),
    arm64::s6.code(),
    arm64::s7.code(),
};

constexpr static const size_t kNumberOfGeneralArgumentsRegisters = arraysize(kGeneralArgumentsRegisters);
constexpr static const size_t kNumberOfFloatArgumentsRegisters = arraysize(kFloatArgumentsRegisters);

struct Arm64RegisterConfigurationInitializer {
    static RegisterConfiguration *New(void *chunk) {
        return new (chunk) RegisterConfiguration(arm64::fp.code()/*fp*/,
                                                 arm64::sp.code()/*sp*/,
                                                 kRootRegister,
                                                 kScratchGeneralRegister0,
                                                 kScratchGeneralRegister1,
                                                 kScratchFloatRegister,
                                                 kScratchDoubleRegister,
                                                 MachineRepresentation::kWord64,
                                                 32/*number_of_general_registers*/,
                                                 32/*number_of_float_registers*/,
                                                 32/*number_of_double_registers*/,
                                                 kAllocatableGeneralRegisters,
                                                 arraysize(kAllocatableGeneralRegisters),
                                                 kAllocatableFloatRegisters,
                                                 arraysize(kAllocatableFloatRegisters),
                                                 kAllocatableDoubleRegisters,
                                                 arraysize(kAllocatableDoubleRegisters));
    }
    
    static void Delete(void *) {}
};

struct Arm64StackConfigurationInitializer {
    static StackConfiguration *New(void *chunk) {
        return new (chunk) StackConfiguration(Arm64Mode_MRI,
                                              32, // saved size
                                              4,  // slot alignment size
                                              16, // stack alignment size
                                              arm64::fp.code(),
                                              arm64::sp.code());
    }

    static void Delete(void *) {}
};

static base::LazyInstance<RegisterConfiguration, Arm64RegisterConfigurationInitializer> kRegConf;
static base::LazyInstance<StackConfiguration, Arm64StackConfigurationInitializer>       kStackConf;

class Arm64FunctionInstructionSelector final {
public:
    enum Policy {
        kReg, // only register
        kMoR, // in memory or register
        kRoI, // can be register or immediate
        kAny,
    };
    
    class Arm64MovingDelegate final : public RegisterSavingScope::MovingDelegate {
    public:
        Arm64MovingDelegate(Arm64FunctionInstructionSelector *owns): owns_(owns) {}
        ~Arm64MovingDelegate() override = default;
        
        void MoveTo(InstructionOperand *dest, InstructionOperand *src, ir::Type ty) override {
            owns_->Move(dest, src, ty);
        }
        void Initialize() override {}
        void Finalize() override {}
        DISALLOW_IMPLICIT_CONSTRUCTORS(Arm64MovingDelegate);
    private:
        Arm64FunctionInstructionSelector *const owns_;
    }; // class Arm64MovingDelegate

    Arm64FunctionInstructionSelector(base::Arena *arena,
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
    , moving_delegate_(this)
    , owns_(owns)
    , fun_(fun)
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
    
    void Run();

    DISALLOW_IMPLICIT_CONSTRUCTORS(Arm64FunctionInstructionSelector);
private:
    InstructionBlock *current() { return DCHECK_NOTNULL(current_block_); }
    
    void AssociateParameters(InstructionBlock *block);
    void SelectBasicBlock(ir::BasicBlock *block);
    void Select(ir::Value *instr);
    
    void CallDirectly(ir::Value *instr);
    void CallRuntime(ir::Value *instr);
    void ConditionBr(ir::Value *instr);
    
    InstructionOperand *Allocate(ir::Value *value, Policy policy, uint32_t imm_bits = 0);
    InstructionOperand *Constant(ir::Value *value, uint32_t imm_bits);
    
    ConstantOperand *Word16Mask(uint16_t mask) {
        const auto id = const_pool_->FindOrInsertWord16(mask);
        return new (arena_) ConstantOperand(ConstantOperand::kNumber, id);
    }
    
    void Move(InstructionOperand *dest, InstructionOperand *src, ir::Type ty);
    void MoveMachineWords(Instruction::Code load_op, Instruction::Code store_op, InstructionOperand *dest,
                          InstructionOperand *src, MachineRepresentation rep, MachineRepresentation scratch_rep);
    void MoveMachineFloat(Instruction::Code load_op, Instruction::Code store_op, InstructionOperand *dest,
                          InstructionOperand *src, MachineRepresentation rep);
    
    bool IsPredecessorCompare() const {
        return prev_instr_ && (prev_instr_->Is(ir::Operator::kICmp) || prev_instr_->Is(ir::Operator::kFCmp));
    }
    
    static size_t ReturningValSizeInBytes(const ir::PrototypeModel *proto);
    static size_t OverflowParametersSizeInBytes(const ir::Function *fun);

    base::Arena *const arena_;
    ir::StructureModel *const owns_;
    ir::Function *const fun_;
    ConstantsPool *const const_pool_;
    LinkageSymbols *const symbols_;
    InstructionBlockLabelGenerator *const labels_;
    Arm64MovingDelegate moving_delegate_;
    OperandAllocator operands_;
    InstructionFunction *bundle_ = nullptr;
    InstructionBlock *current_block_ = nullptr;
    ir::Value *prev_instr_ = nullptr;
    int instruction_position_ = 0;
    ImmediateOperand *stack_total_size_ = nullptr;
    ImmediateOperand *stack_used_size_ = nullptr;
    LocationOperand *stack_sp_fp_location_ = nullptr;
    std::map<ir::BasicBlock *, InstructionBlock *> blocks_;
    std::vector<InstructionOperand *> tmps_;
    std::vector<OperandAllocator::BorrowedRecord> borrowed_registers_;
    std::vector<ImmediateOperand *> calling_stack_adjust_;
}; // class Arm64FunctionInstructionSelector

void Arm64FunctionInstructionSelector::Run() {
    auto blk = blocks_[fun_->entry()];
    auto sp = operands_.registers()->stack_pointer();
    auto fp = operands_.registers()->frame_pointer();
    stack_total_size_ = ImmediateOperand::Word32(arena_, 0);
    blk->NewIO(Arm64Sub, sp, sp, stack_total_size_); // sub sp, sp, stack-total-size
    //stack_sp_fp_offset_ = ImmediateOperand::word32(arena_, 0);
    stack_sp_fp_location_ = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::sp.code(), -1, 0);
    // stp sp, lr, [sp, location]
    blk->NewIO(Arm64Stp, stack_sp_fp_location_, fp,
               new (arena_) RegisterOperand(arm64::lr.code(), MachineRepresentation::kWord64));
    // add fp, sp, stack-used-size
    stack_used_size_ = ImmediateOperand::Word32(arena_, 0);
    blk->NewIO(Arm64Add, fp, sp, stack_used_size_);
    
    AssociateParameters(blk);
    for (auto blk : fun_->blocks()) {
        SelectBasicBlock(blk);
    }
    
    const auto stack_size = RoundUp(operands_.slots()->max_stack_size(), kStackConf->stack_alignment_size());
    stack_total_size_->Set32(stack_size + kPointerSize * 2);
    stack_used_size_->Set32(stack_size);
    stack_sp_fp_location_->set_k(stack_size);

    for (auto adjust : calling_stack_adjust_) {
        assert(stack_size >= adjust->word32());
        adjust->Set32(stack_size - adjust->word32());
    }
}

void Arm64FunctionInstructionSelector::AssociateParameters(InstructionBlock *block) {
    current_block_ = block;
    
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
                auto opd = operands_.AllocateStackSlot(param->type(), StackSlotAllocator::kLinear);
                Move(opd, arg, param->type());
                operands_.Associate(param, opd);
            }
            
            number_of_general_args--;
        }
    }
    
    current_block_ = nullptr;
}

void Arm64FunctionInstructionSelector::SelectBasicBlock(ir::BasicBlock *bb) {
    current_block_ = blocks_[bb];
    for (auto user : bb->phi_node_users()) {
        Allocate(user.phi, kMoR);
    }
    
    for (auto instr : bb->instructions()) {
        operands_.ReleaseDeads(instruction_position_);
        tmps_.clear();
        if (instr->op()->IsTerminator()) {
            for (auto user : bb->phi_node_users()) {
                auto phi_slot = DCHECK_NOTNULL(operands_.Allocated(user.phi));
                auto dest = Allocate(user.dest, kAny);
                Move(phi_slot, dest, user.phi->type());
            }
        }
        Select(instr);
        for (auto tmp : tmps_) { operands_.Free(tmp); }
        for (const auto &borrow : borrowed_registers_) {
            
            if (borrow.original) {
                Move(borrow.old, borrow.bak, borrow.original->type());
                operands_.Associate(borrow.original, borrow.old);
            } else {
                operands_.Free(borrow.bak);
                operands_.Free(borrow.target);
            }
        }
        prev_instr_ = instr;
        instruction_position_++;
    }
    
    current_block_ = nullptr;
}

void Arm64FunctionInstructionSelector::Select(ir::Value *instr) {
    switch (instr->op()->value()) {
        case ir::Operator::kBr: {
            if (instr->op()->value_in() == 0) {
                auto ib = blocks_[instr->OutputControl(0)];
                auto label = new (arena_) ReloactionOperand(nullptr/*symbol_name*/, ib);
                current()->NewO(ArchJmp, label);
                current()->New(ArchNop);
                current()->AddSuccessor(ib);
                ib->AddPredecessors(current());
                return;
            }
            ConditionBr(instr);
        } break;
            
        case ir::Operator::kLoadEffectField: {
            UNREACHABLE();
        } break;
            
        case ir::Operator::kLoadEffectAddress: {
            UNREACHABLE();
        } break;
            
        case ir::Operator::kICmp: {
            auto lhs = Allocate(instr->InputValue(0), kReg);
            auto rhs = Allocate(instr->InputValue(1), kRoI, 12/*imm_bits*/);
            switch (ToMachineRepresentation(instr->InputValue(0)->type())) {
                case MachineRepresentation::kWord8:
                case MachineRepresentation::kWord16:
                case MachineRepresentation::kWord32:
                    current()->NewII(Arm64Cmp32, lhs, rhs);
                    break;
                case MachineRepresentation::kWord64:
                    current()->NewII(Arm64Cmp, lhs, rhs);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            // TODO:
        } break;
            
        case ir::Operator::kFCmp: {
            auto lhs = Allocate(instr->InputValue(0), kReg);
            auto rhs = Allocate(instr->InputValue(1), kReg);
            switch (ToMachineRepresentation(instr->InputValue(0)->type())) {
                case MachineRepresentation::kFloat32:
                    current()->NewII(Arm64Float32Cmp, lhs, rhs);
                    break;
                case MachineRepresentation::kFloat64:
                    current()->NewII(Arm64Float64Cmp, lhs, rhs);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            // TODO:
        } break;
            
        case ir::Operator::kAdd: {
            auto lhs = Allocate(instr->InputValue(0), kReg);
            auto rhs = Allocate(instr->InputValue(1), kRoI, 12/*imm_bits*/);
            auto opd = Allocate(instr, kReg);
            switch (ToMachineRepresentation(instr->type())) {
                case MachineRepresentation::kWord8:
                case MachineRepresentation::kWord16:
                case MachineRepresentation::kWord32:
                    current()->NewIO(Arm64Add32, opd, lhs, rhs);
                    break;
                case MachineRepresentation::kWord64:
                    current()->NewIO(Arm64Add, opd, lhs, rhs);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case ir::Operator::kFAdd: {
            auto lhs = Allocate(instr->InputValue(0), kReg);
            auto rhs = Allocate(instr->InputValue(1), kReg);
            auto opd = Allocate(instr, kReg);
            switch (ToMachineRepresentation(instr->type())) {
                case MachineRepresentation::kFloat32:
                    current()->NewIO(Arm64Float32Abd, opd, lhs, rhs);
                    break;
                case MachineRepresentation::kFloat64:
                    current()->NewIO(Arm64Float64Add, opd, lhs, rhs);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case ir::Operator::kLoadFunAddr: {
            auto fun = ir::OperatorWith<const ir::Function *>::Data(instr->op());
            auto symbol = symbols_->Mangle(fun->full_name());
            bundle()->AddExternalSymbol(fun->full_name()->ToSlice(), symbol);
            
            auto opd = Allocate(instr, kAny);
            auto rel = new (arena_) ReloactionOperand(symbol, nullptr, true/*fetch_address*/);
            Move(opd, rel, instr->type());
        } break;
            
        case ir::Operator::kClosure: {
            // TODO:
            UNREACHABLE();
        } break;;
            
        case ir::Operator::kCallRuntime:
            CallRuntime(instr);
            break;
            
        case ir::Operator::kCallDirectly:
            CallDirectly(instr);
            break;
            
        case ir::Operator::kReturningVal:
            // Just Ignore this ir code
            break;
            
        case ir::Operator::kPhi:
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
            for (int i = instr->op()->value_in() - 1; i >= 0; i--) {
                auto ty = fun_->prototype()->return_type(i);
                if (ty.kind() == ir::Type::kVoid) {
                    continue;
                }
                auto ret = Allocate(instr->InputValue(i), kAny);
                auto opd = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::fp.code(), 0, returning_val_offset);
                returning_val_offset += RoundUp(ty.ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
                Move(opd, ret, ty);
            }
            
            // stp sp, lr, [sp, location]
            auto lr = new (arena_) RegisterOperand(arm64::lr.code(), MachineRepresentation::kWord64);
            current()->NewIO2(Arm64Ldp,
                              operands_.registers()->frame_pointer(),
                              lr,
                              stack_sp_fp_location_);
            current()->NewIO(Arm64Add,
                             operands_.registers()->stack_pointer(),
                             operands_.registers()->stack_pointer(),
                             stack_total_size_);
            current()->New(ArchRet);
        } break;
            
        default:
            UNREACHABLE();
            break;
    }
}

void Arm64FunctionInstructionSelector::CallDirectly(ir::Value *instr) {
    auto callee = ir::OperatorWith<ir::Function *>::Data(instr->op());
    std::vector<ir::Value *> returning_vals;
    returning_vals.push_back(instr);
    for (auto edge : instr->users()) {
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
    std::vector<InstructionOperand *> args;
    RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
    size_t overflow_args_size = 0;
    int general_index = 0, float_index = 0;
    for (int i = 0; i < instr->op()->value_in(); i++) {
        auto arg = instr->InputValue(i);
        const auto size = RoundUp(arg->type().ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
        if (arg->type().IsFloating()) {
            if (float_index < kNumberOfFloatArgumentsRegisters) {
                saving_scope.AddExclude(arg, kFloatArgumentsRegisters[float_index], instruction_position_);
                args.push_back(Allocate(arg, kAny));
            } else {
                // overflow
                overflow_args_size += size;
                overflow_args.push_back(arg);
            }
            float_index++;
        } else {
            if (general_index < kNumberOfGeneralArgumentsRegisters) {
                saving_scope.AddExclude(arg, kGeneralArgumentsRegisters[general_index], instruction_position_);
                args.push_back(Allocate(arg, kAny));
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
    for (int i = 0; i < instr->op()->value_in(); i++) {
        auto arg = instr->InputValue(i);
        if (arg->type().IsFloating()) {
            if (float_index < kNumberOfFloatArgumentsRegisters) {
                if (saving_scope.Include(kFloatArgumentsRegisters[float_index], false/*general*/)) {
                    auto dest = DCHECK_NOTNULL(operands_.AllocateReigster(arg->type(),
                                                                          kFloatArgumentsRegisters[float_index]));
                    Move(dest, args[i], arg->type());
                    operands_.Associate(arg, dest);
                }
            }
            float_index++;
        } else {
            if (general_index < kNumberOfGeneralArgumentsRegisters) {
                if (saving_scope.Include(kGeneralArgumentsRegisters[general_index], true/*general*/)) {
                    auto dest = DCHECK_NOTNULL(operands_.AllocateReigster(arg->type(),
                                                                          kGeneralArgumentsRegisters[general_index]));
                    Move(dest, args[i], arg->type());
                    operands_.Associate(arg, dest);
                }
            }
            general_index++;
        }
    }
    
    auto current_stack_size = operands_.slots()->stack_size();
    for (auto rv : returning_vals) {
        if (rv->type().kind() != ir::Type::kVoid) {
            operands_.AllocateStackSlot(rv, StackSlotAllocator::kLinear);
        }
    }
    
    if (overflow_args_size > 0) {
        for (auto arg : overflow_args) {
            auto opd = operands_.AllocateStackSlot(arg->type(), StackSlotAllocator::kLinear);
            tmps_.push_back(opd);
            Move(opd, Allocate(arg, kAny), arg->type());
        }
        
    }
    
    current_stack_size += ReturningValSizeInBytes(callee->prototype());
    current_stack_size += overflow_args_size;
    current_stack_size = RoundUp(current_stack_size, kStackConf->stack_alignment_size());
    
    auto adjust = ImmediateOperand::Word32(arena_, static_cast<int>(current_stack_size));
    calling_stack_adjust_.push_back(adjust);
    current()->NewIO(Arm64Add,
                     operands_.registers()->stack_pointer(),
                     operands_.registers()->stack_pointer(),
                     adjust);
    auto rel = new (arena_) ReloactionOperand(symbols_->Mangle(callee->full_name()), nullptr);
    current()->NewI(ArchCall, rel);
    current()->NewIO(Arm64Sub,
                     operands_.registers()->stack_pointer(),
                     operands_.registers()->stack_pointer(),
                     adjust);
}

void Arm64FunctionInstructionSelector::CallRuntime(ir::Value *instr) {
    const auto runtime_id = ir::OperatorWith<ir::RuntimeId>::Data(instr->op());
    const String *symbol = nullptr;
    switch (runtime_id.value) {
        case ir::RuntimeId::kPkgInitOnce: {
            symbol = kRt_pkg_init_once;
            assert(instr->op()->value_in() == 2);
        } break;
            
        default:
            UNREACHABLE();
            break;
    }
    
    RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
    std::vector<InstructionOperand *> args;
    int general_index = 0, float_index = 0;
    for (int i = 0; i < instr->op()->value_in(); i++) {
        auto arg = instr->InputValue(i);
        if (symbol == kRt_pkg_init_once && i == 1) {
            const auto const pkg_name = ir::OperatorWith<const String *>::Data(arg->op());
            const auto kid = const_pool_->FindOrInsertString(pkg_name);
            const auto linked_name = String::New(arena_, base::Sprintf("Lkzs.%d", kid));
            args.push_back(new (arena_) ReloactionOperand(linked_name, nullptr, true/*fetch_address*/));
            general_index++;
            continue;
        }
        const auto size = RoundUp(arg->type().ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
        if (arg->type().IsFloating()) {
            if (float_index < kNumberOfFloatArgumentsRegisters) {
                args.push_back(Allocate(arg, kAny));
                saving_scope.AddExclude(arg, kFloatArgumentsRegisters[float_index], instruction_position_);
            } else {
                // overflow
                UNREACHABLE();
            }
            float_index++;
        } else {
            if (general_index < kNumberOfGeneralArgumentsRegisters) {
                args.push_back(Allocate(arg, kAny));
                saving_scope.AddExclude(arg, kGeneralArgumentsRegisters[general_index], instruction_position_);
            } else {
                // overflow
                UNREACHABLE();
            }
            general_index++;
        }
    }
    if (instr->type().kind() != ir::Type::kVoid) {
        if (instr->type().IsFloating()) {
            if (operands_.IsFloatRegisterAlive(arm64::d0.code())) {
                // TODO:
                UNREACHABLE();
            }
        } else {
            if (operands_.IsGeneralRegisterAlive(arm64::x0.code())) {
                // TODO:
                UNREACHABLE();
            }
        }
    }
    saving_scope.SaveAll();
    
    general_index = 0, float_index = 0;
    for (int i = 0; i < instr->op()->value_in(); i++) {
        auto arg = instr->InputValue(i);
        if (arg->type().IsFloating()) {
            if (float_index < kNumberOfFloatArgumentsRegisters) {
                if (saving_scope.Include(kFloatArgumentsRegisters[float_index], false/*general*/)) {
                    auto dest = DCHECK_NOTNULL(operands_.AllocateReigster(arg->type(),
                                                                          kFloatArgumentsRegisters[float_index]));
                    Move(dest, args[i], arg->type());
                    operands_.Associate(arg, dest);
                }
            }
            float_index++;
        } else {
            if (general_index < kNumberOfGeneralArgumentsRegisters) {
                if (saving_scope.Include(kGeneralArgumentsRegisters[general_index], true/*general*/)) {
                    auto dest = DCHECK_NOTNULL(operands_.AllocateReigster(arg->type(),
                                                                          kGeneralArgumentsRegisters[general_index]));
                    Move(dest, args[i], arg->type());
                    operands_.Associate(arg, dest);
                }
            }
            general_index++;
        }
    }
    
    auto rel = new (arena_) ReloactionOperand(symbol, nullptr);
    current()->NewI(ArchCall, rel);
}

void Arm64FunctionInstructionSelector::ConditionBr(ir::Value *instr) {
    auto cond = instr->InputValue(0);
    auto if_true = blocks_[instr->OutputControl(0)];
    auto if_false = blocks_[instr->OutputControl(1)];
    auto label = new (arena_) ReloactionOperand(nullptr/*symbol_name*/, if_true);
    auto output = new (arena_) ReloactionOperand(nullptr, if_false);
    if (cond->Is(ir::Operator::kICmp)) {
        if (prev_instr_ != cond) {
            auto cond_val = Allocate(cond, kReg);
            auto zero = ImmediateOperand::Word8(arena_, 0);
            current()->NewII(Arm64Cmp32, cond_val, zero);
            current()->NewO(Arm64B_eq, output);
        } else {
            switch (ir::OperatorWith<ir::IConditionId>::Data(cond->op()).value) {
                case ir::IConditionId::k_eq:
                    current()->NewO(Arm64B_ne, output);
                    break;
                case ir::IConditionId::k_ne:
                    current()->NewO(Arm64B_eq, output);
                    break;
                case ir::IConditionId::k_slt:
                    current()->NewO(Arm64B_ge, output);
                    break;
                case ir::IConditionId::k_ult:
                    current()->NewO(Arm64B_eq, output);
                    current()->NewO(Arm64B_hi, output); // unsigned higher
                    break;
                case ir::IConditionId::k_sle:
                    current()->NewO(Arm64B_gt, output);
                    break;
                case ir::IConditionId::k_ule:
                    current()->NewO(Arm64B_hi, output); // unsigned higher
                    break;
                case ir::IConditionId::k_sgt:
                    current()->NewO(Arm64B_le, output);
                    break;
                case ir::IConditionId::k_ugt:
                    current()->NewO(Arm64B_ls, output); // unsigned lower or same
                    break;
                case ir::IConditionId::k_sge:
                    current()->NewO(Arm64B_lt, output);
                    break;
                case ir::IConditionId::k_uge:
                    current()->NewO(Arm64B_eq, label);
                    current()->NewO(Arm64B_ls, output); // unsigned lower or same
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        }
    } else if (cond->Is(ir::Operator::kFCmp)) {
        if (prev_instr_ != cond) {
            auto cond_val = Allocate(cond, kReg);
            auto zero = ImmediateOperand::Word8(arena_, 0);
            current()->NewII(Arm64Cmp32, cond_val, zero);
            current()->NewO(Arm64B_eq, output);
        } else {
            //      NZCV
            // NaN: 0011
            // EQ:  0110
            // LT:  1000
            // GT:  0010
            switch (ir::OperatorWith<ir::FConditionId>::Data(cond->op()).value) {
                    // oeq: yields true if both operands are not a QNAN and op1 is equal to op2.
                case ir::FConditionId::k_oeq:
                    current()->NewO(Arm64B_ne, output); // not equal or unordered
                    break;
                case ir::FConditionId::k_one:
                    current()->NewO(Arm64B_eq, output); // equal
                    break;
                case ir::FConditionId::k_olt:
                    current()->NewO(Arm64B_pl, output); // greater equal or undorderd
                    break;
                case ir::FConditionId::k_ole:
                    current()->NewO(Arm64B_hi, output); // greater or undorderd
                    break;
                case ir::FConditionId::k_ogt:
                    current()->NewO(Arm64B_le, output); // less equal or undordered
                    break;
                case ir::FConditionId::k_oge:
                    current()->NewO(Arm64B_lt, output); // less or undordered
                    break;
                // ord: yields true if both operands are not a QNAN.
                case ir::FConditionId::k_ord:
                    current()->NewO(Arm64B_vs, output); // unordered
                    break;
                // ueq: yields true if either operand is a QNAN or op1 is equal to op2.
                case ir::FConditionId::k_ueq:
                    current()->NewO(Arm64B_vs, label); // unordered
                    current()->NewO(Arm64B_ne, output); // not equal or unordered
                    break;
                // une: yields true if either operand is a QNAN or op1 is not equal to op2.
                case ir::FConditionId::k_une:
                    current()->NewO(Arm64B_vs, label); // unordered
                    current()->NewO(Arm64B_eq, output); // equal
                    break;
                case ir::FConditionId::k_ult:
                    current()->NewO(Arm64B_vs, label); // unordered
                    current()->NewO(Arm64B_ge, output); // greater or equal
                    break;
                case ir::FConditionId::k_ule:
                    current()->NewO(Arm64B_vs, label); // unordered
                    current()->NewO(Arm64B_hi, output); // greater or undorderd
                    break;
                case ir::FConditionId::k_ugt:
                    current()->NewO(Arm64B_vs, label); // unordered
                    current()->NewO(Arm64B_le, output); // less equal or undordered
                    break;
                case ir::FConditionId::k_uge:
                    current()->NewO(Arm64B_vs, label); // unordered
                    current()->NewO(Arm64B_lt, output);
                    break;
                // uno: yields true if either operand is a QNAN.
                case ir::FConditionId::k_uno:
                    current()->NewO(Arm64B_vc, output);
                    break;
                case ir::FConditionId::k_never:
                    current()->NewO(ArchJmp, output);
                    break;
                case ir::FConditionId::k_always:
                    current()->NewO(Arm64B_al, output);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        }
    } else {
        auto cond_val = Allocate(cond, kReg);
        auto zero = ImmediateOperand::Word8(arena_, 0);
        current()->NewII(Arm64Cmp, cond_val, zero);
        current()->NewO(Arm64B_eq, output);
    }
    
//    current()->NewO(ArchJmp, label);
//    current()->New(ArchNop);
//    current()->AddSuccessor(if_false);
//    current()->AddSuccessor(if_true);
//    if_false->AddPredecessors(current());
//    if_true->AddPredecessors(current());
}

InstructionOperand *Arm64FunctionInstructionSelector::Allocate(ir::Value *value, Policy policy, uint32_t imm_bits) {
    auto opd = operands_.Allocated(value);
    switch (policy) {
        case kReg: {
            if (opd && opd->IsRegister()) {
                return opd;
            }
            if (value->op()->IsConstant()) {
                auto kop = Constant(value, 0);
                assert(kop->IsImmediate() || kop->IsConstant());
                auto bak = operands_.Allocate(value->type());
                if (bak->IsRegister()) {
                    tmps_.push_back(bak);
                    Move(bak, kop, value->type());
                    return bak;
                }
                assert(bak->IsLocation());
                auto brd = operands_.BorrowRegister(value, bak);
                Move(bak, brd.target, value->type()); // Saving
                Move(brd.target, kop, value->type());
                borrowed_registers_.push_back(brd);
                
                return brd.target;
            }
            if (!opd) {
                opd = operands_.Allocate(value);
            }
            if (!opd->IsRegister()) {
                auto bak = operands_.Allocate(value->type());
                if (bak->IsRegister()) {
                    Move(bak, opd, value->type());
                    auto old = operands_.LinkTo(value, bak);
                    assert(old == opd);
                    tmps_.push_back(opd);
                    return bak;
                }
                auto brd = operands_.BorrowRegister(value, bak);
                Move(bak, brd.target, value->type()); // Saving
                Move(brd.target, opd, value->type());
                borrowed_registers_.push_back(brd);
                
                return brd.target;
            }
            return opd;
        } break;

        case kRoI: {
            if (opd && opd->IsRegister()) {
                return opd;
            }
            if (value->op()->IsConstant()) {
                auto opd = Constant(value, imm_bits);
                if (opd->IsRegister() || opd->IsImmediate()) {
                    return opd;
                }
                assert(opd->IsReloaction());
                auto bak = operands_.Allocate(value->type());
                auto brd = operands_.BorrowRegister(value, bak);
                Move(bak, brd.target, value->type()); // Saving
                Move(brd.target, opd, value->type()); //
                borrowed_registers_.push_back(brd);

                return brd.target;
            }
            if (!opd) {
                opd = operands_.Allocate(value);
            }
            if (opd->IsRegister()) {
                return opd;
            }

            assert(opd->IsLocation());
            auto bak = operands_.Allocate(value->type());
            if (bak->IsRegister()) {
                Move(bak, opd, value->type());
                auto old = operands_.LinkTo(value, bak);
                assert(old == opd);
                tmps_.push_back(opd);
                return bak;
            }
            auto brd = operands_.BorrowRegister(value, bak);
            Move(bak, brd.target, value->type()); // Saving
            Move(brd.target, opd, value->type());
            borrowed_registers_.push_back(brd);

            return brd.target;
        } break;

        case kMoR: {
            if (opd) {
                return opd;
            }
            if (value->op()->IsConstant()) {
                auto kop = Constant(value, 0/*imm_bits*/);
                assert(kop->IsImmediate() || kop->IsConstant());
                auto opd = operands_.Allocate(value);
                //tmps_.push_back(opd);
                Move(opd, kop, value->type());
                return opd;
            }
            return operands_.Allocate(value);
        } break;

        case kAny: {
            if (opd) {
                return opd;
            }
            
            if (value->op()->IsConstant()) {
//                if (value->Is(ir::Operator::kStringConstant)) {
//                    return Constant(value, imm_bits);
//                }
                opd = operands_.Allocate(value);
                //tmps_.push_back(opd);
                if (opd->IsRegister()) {
                    auto imm = Constant(value, 16);
                    if (imm->IsImmediate()) {
                        current()->NewIO(Arm64Mov, opd, imm);
                        return opd;
                    }
                    Move(opd, imm, value->type());
                    return opd;
                }
                auto kval = Constant(value, imm_bits);
                Move(opd, kval, value->type());
                return opd;
            }
            return operands_.Allocate(value);
        } break;
            
        default:
            UNREACHABLE();
            break;
    }
    return nullptr;
}

InstructionOperand *Arm64FunctionInstructionSelector::Constant(ir::Value *value, uint32_t imm_bits) {
    switch (value->op()->value()) {
        case ir::Operator::kWord8Constant:
        case ir::Operator::kU8Constant: {
            const auto kval = ir::OperatorWith<uint8_t>::Data(value->op());
            if (imm_bits > 0 && base::is_intn(kval, imm_bits)) {
                return ImmediateOperand::Word8(arena_, kval);
            } else {
                return new (arena_) ConstantOperand(ConstantOperand::kNumber, const_pool_->FindOrInsertWord8(kval));
            }
        } break;
        case ir::Operator::kWord16Constant:
        case ir::Operator::kU16Constant: {
            const auto kval = ir::OperatorWith<uint16_t>::Data(value->op());
            if (imm_bits > 0 && base::is_intn(kval, imm_bits)) {
                return ImmediateOperand::Word16(arena_, kval);
            } else {
                return new (arena_) ConstantOperand(ConstantOperand::kNumber, const_pool_->FindOrInsertWord16(kval));
            }
        } break;
        case ir::Operator::kWord32Constant:
        case ir::Operator::kU32Constant: {
            const auto kval = ir::OperatorWith<uint32_t>::Data(value->op());
            if (imm_bits > 0 && base::is_intn(kval, imm_bits)) {
                return ImmediateOperand::Word32(arena_, kval);
            } else {
                return new (arena_) ConstantOperand(ConstantOperand::kNumber, const_pool_->FindOrInsertWord32(kval));
            }
        } break;
        case ir::Operator::kWord64Constant:
        case ir::Operator::kU64Constant: {
            const auto kval = ir::OperatorWith<uint64_t>::Data(value->op());
            if (imm_bits > 0 && base::is_intn(kval, imm_bits)) {
                return ImmediateOperand::Word64(arena_, kval);
            } else {
                return new (arena_) ConstantOperand(ConstantOperand::kNumber, const_pool_->FindOrInsertWord64(kval));
            }
        } break;
        case ir::Operator::kI8Constant: {
            const auto kval = ir::OperatorWith<int8_t>::Data(value->op());
            if (kval < 0 || imm_bits == 0 || !base::is_intn(kval, imm_bits)) {
                return new (arena_) ConstantOperand(ConstantOperand::kNumber, const_pool_->FindOrInsertWord8(kval));
            } else {
                return ImmediateOperand::Word8(arena_, kval);
            }
        } break;
        case ir::Operator::kI16Constant: {
            const auto kval = ir::OperatorWith<int16_t>::Data(value->op());
            if (kval < 0 || imm_bits == 0 || !base::is_intn(kval, imm_bits)) {
                return new (arena_) ConstantOperand(ConstantOperand::kNumber, const_pool_->FindOrInsertWord16(kval));
            } else {
                return ImmediateOperand::Word16(arena_, kval);
            }
        } break;
        case ir::Operator::kI32Constant: {
            const auto kval = ir::OperatorWith<int32_t>::Data(value->op());
            if (kval < 0 || imm_bits == 0 || !base::is_intn(kval, imm_bits)) {
                return new (arena_) ConstantOperand(ConstantOperand::kNumber, const_pool_->FindOrInsertWord32(kval));
            } else {
                return ImmediateOperand::Word32(arena_, kval);
            }
        } break;
        case ir::Operator::kI64Constant: {
            const auto kval = ir::OperatorWith<int64_t>::Data(value->op());
            if (kval < 0 || imm_bits == 0 || !base::is_intn(kval, imm_bits)) {
                return new (arena_) ConstantOperand(ConstantOperand::kNumber, const_pool_->FindOrInsertWord64(kval));
            } else {
                return ImmediateOperand::Word64(arena_, kval);
            }
        } break;
        case ir::Operator::kF32Constant: {
            const auto kval = ir::OperatorWith<float>::Data(value->op());
            return new (arena_) ConstantOperand(ConstantOperand::kNumber, const_pool_->FindOrInsertFloat32(kval));
        } break;
        case ir::Operator::kF64Constant: {
            const auto kval = ir::OperatorWith<double>::Data(value->op());
            return new (arena_) ConstantOperand(ConstantOperand::kNumber, const_pool_->FindOrInsertFloat64(kval));
        } break;
        case ir::Operator::kStringConstant: {
            const auto kval = ir::OperatorWith<const String *>::Data(value->op());
            return new (arena_) ConstantOperand(ConstantOperand::kString, const_pool_->FindOrInsertString(kval));
        } break;
        case ir::Operator::kNilConstant:
            return ImmediateOperand::Word64(arena_, 0);
        default:
            UNREACHABLE();
            break;
    }
    return nullptr;
}

void Arm64FunctionInstructionSelector::Move(InstructionOperand *dest, InstructionOperand *src, ir::Type ty) {
    assert(dest->IsRegister() || dest->IsLocation());
    switch (ty.kind()) {
        case ir::Type::kInt8:
            MoveMachineWords(Arm64Ldrsb, Arm64Strb, dest, src, MachineRepresentation::kWord8,
                             MachineRepresentation::kWord32);
            break;
        case ir::Type::kUInt8:
        case ir::Type::kWord8:
            MoveMachineWords(Arm64Ldrb, Arm64Strb, dest, src, MachineRepresentation::kWord8,
                             MachineRepresentation::kWord32);
            break;
            
        case ir::Type::kInt16:
            MoveMachineWords(Arm64Ldrsw, Arm64StrW, dest, src, MachineRepresentation::kWord16,
                             MachineRepresentation::kWord32);
            break;
        case ir::Type::kUInt16:
        case ir::Type::kWord16:
            MoveMachineWords(Arm64LdrW, Arm64StrW, dest, src, MachineRepresentation::kWord16,
                             MachineRepresentation::kWord32);
            break;
            
        case ir::Type::kInt32:
        case ir::Type::kUInt32:
        case ir::Type::kWord32:
            MoveMachineWords(Arm64Ldr, Arm64Str, dest, src, MachineRepresentation::kWord32,
                             MachineRepresentation::kWord32);
            break;
            
        case ir::Type::kInt64:
        case ir::Type::kUInt64:
        case ir::Type::kWord64:
        case ir::Type::kReference:
        case ir::Type::kString:
            MoveMachineWords(Arm64Ldr, Arm64Str, dest, src, MachineRepresentation::kWord64,
                             MachineRepresentation::kWord64);
            break;
            
        case ir::Type::kFloat32:
            MoveMachineFloat(Arm64LdrS, Arm64StrS, dest, src, MachineRepresentation::kFloat32);
            break;
            
        case ir::Type::kFloat64:
            MoveMachineFloat(Arm64LdrD, Arm64StrD, dest, src, MachineRepresentation::kFloat64);
            break;
            
        case ir::Type::kValue: {
            if (ty.IsPointer()) {
                MoveMachineWords(Arm64Ldr, Arm64Str, dest, src, MachineRepresentation::kWord64,
                                 MachineRepresentation::kWord64);
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

void Arm64FunctionInstructionSelector::MoveMachineWords(Instruction::Code load_op, Instruction::Code store_op,
                                                        InstructionOperand *dest, InstructionOperand *src,
                                                        MachineRepresentation rep, MachineRepresentation scratch_rep) {
    const bool fetch_address = src->IsReloaction() && src->AsReloaction()->fetch_address();
    if (dest->IsRegister()) {
        if (src->IsRegister()) {
            current()->NewIO(Arm64Mov, dest, src);
        } else if (src->IsLocation()) {
            current()->NewIO(load_op, dest, src);
        } else if (src->IsConstant() || src->IsReloaction()) {
            auto tmp = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
            current()->NewIO(Arm64Adrp, tmp, src);
            if (fetch_address) {
                current()->NewIO(Arm64AddOff, dest, tmp, src);
            } else {
                current()->NewIO(Arm64AddOff, tmp, tmp, src);
                auto loc = new (arena_) LocationOperand(Arm64Mode_MRI, kScratchGeneralRegister0, -1, 0);
                current()->NewIO(load_op, dest, loc);
            }
        } else if (src->IsImmediate()) {
            current()->NewIO(Arm64Mov, dest, src);
        } else {
            UNREACHABLE();
        }
    } else if (dest->IsLocation()) {
        if (src->IsRegister()) {
            current()->NewIO(store_op, dest, src);
        } else if (src->IsLocation()) {
            // ldr x19, [src]
            // str x19, [dest]
            auto tmp0 = operands_.registers()->GeneralScratch0(scratch_rep);
            current()->NewIO(load_op, tmp0, src);
            current()->NewIO(store_op, dest, tmp0);
        } else if (src->IsReloaction() || src->IsConstant()) {
            // adrp x19, dest@PAGE
            // add x19, x19, dest@PAGEOFF
            // ldr x19, [x19, 0]
            // str x19, [dest]
            auto tmp0 = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
            current()->NewIO(Arm64Adrp, tmp0, src);
            current()->NewIO(Arm64AddOff, tmp0, tmp0, src);
            if (!fetch_address) {
                auto loc0 = new (arena_) LocationOperand(Arm64Mode_MRI, tmp0->register_id(), -1, 0);
                current()->NewIO(load_op, tmp0, loc0);
            }
            current()->NewIO(store_op, dest, tmp0);
        } else if (src->IsImmediate()) {
            current()->NewIO(Arm64Mov, dest, src);
        } else {
            UNREACHABLE();
        }
    } else if (dest->IsReloaction()) {
        auto tmp0 = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
        auto tmp1 = operands_.registers()->GeneralScratch1(MachineRepresentation::kWord64);
        current()->NewIO(Arm64Adrp, tmp0, dest);
        current()->NewIO(Arm64AddOff, tmp0, tmp0, dest);
        auto loc0 = new (arena_) LocationOperand(Arm64Mode_MRI, tmp0->register_id(), -1, 0);
        
        if (src->IsRegister()) {
            current()->NewIO(store_op, loc0, src);
        } else if (src->IsLocation()) {
            // adrp x19, dest@PAGE
            // add x19, x19, dest@PAGEOFF
            // ldr x20, [fp, src]
            // str x20, [x19, 0]
            
            current()->NewIO(load_op, tmp1, src);
            current()->NewIO(store_op, loc0, tmp1); // Saving `tmp' aka `src'
        } else if (src->IsConstant() || src->IsReloaction()) {
            // adrp x19, dest@PAGE
            // add x19, x19, dest@PAGEOFF
            // adrp x20, src@PAGE
            // add x20, x20, src@PAGEOFF
            // ldr x20, [x20, 0]
            // str x20, [x19, 0]
            current()->NewIO(Arm64Adrp, tmp1, src);
            current()->NewIO(Arm64AddOff, tmp1, tmp1, src);
            if (!fetch_address) {
                auto loc1 = new (arena_) LocationOperand(Arm64Mode_MRI, tmp1->register_id(), -1, 0);
                current()->NewIO(load_op, tmp1, loc1);
            }
            current()->NewIO(store_op, loc0, tmp1); // Saving `tmp' aka `src'
        }
    } else {
        UNREACHABLE();
    }
}

void Arm64FunctionInstructionSelector::MoveMachineFloat(Instruction::Code load_op, Instruction::Code store_op,
                                                        InstructionOperand *dest, InstructionOperand *src,
                                                        MachineRepresentation rep) {
    auto scratch = rep == MachineRepresentation::kFloat32
    ? operands_.registers()->float_scratch()
    : operands_.registers()->double_scratch();

    if (dest->IsRegister()) {
        if (src->IsRegister()) {
            current()->NewIO(Arm64FMov, dest, src);
        } else if (src->IsLocation()) {
            current()->NewIO(load_op, dest, src);
        } else if (src->IsReloaction() || src->IsConstant()) {
            auto tmp = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
            current()->NewIO(Arm64Adrp, tmp, src);
            current()->NewIO(Arm64AddOff, tmp, tmp, src);
            auto loc = new (arena_) LocationOperand(Arm64Mode_MRI, tmp->register_id(), -1, 0);
            current()->NewIO(load_op, dest, loc);
        } else {
            UNREACHABLE();
        }
    } else if (dest->IsLocation()) {
        if (src->IsRegister()) {
            current()->NewIO(store_op, dest, src);
        } else if (src->IsLocation()) {
            // ldr s19, [src]
            // str s19, [dest]
            current()->NewIO(load_op, scratch, src);
            current()->NewIO(store_op, dest, scratch);
        } else if (src->IsReloaction() || src->IsConstant()) {
            // adrp x19, src@PAGE
            // add x19, x19, src@PAGEOFF
            // ldr s19, [x19, 0]
            // str s19, [dest]
            auto tmp0 = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
            current()->NewIO(Arm64Adrp, tmp0, src);
            current()->NewIO(Arm64AddOff, tmp0, tmp0, src);
            auto loc0 = new (arena_) LocationOperand(Arm64Mode_MRI, tmp0->register_id(), -1, 0);
            current()->NewIO(load_op, scratch, loc0);
            current()->NewIO(store_op, dest, scratch);
        } else {
            UNREACHABLE();
        }
    } else if (dest->IsReloaction()) {
        auto tmp0 = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
        current()->NewIO(Arm64Adrp, tmp0, dest);
        current()->NewIO(Arm64AddOff, tmp0, tmp0, dest);
        auto loc0 = new (arena_) LocationOperand(Arm64Mode_MRI, tmp0->register_id(), -1, 0);
        
        if (src->IsRegister()) {
            current()->NewIO(store_op, loc0, src);
        } else if (src->IsLocation()) {
            // adrp x19, dest@PAGE
            // add x19, x19, dest@PAGEOFF
            // ldr s19, [src]
            // str s19, [x19, 0]
            current()->NewIO(load_op, scratch, src);
            current()->NewIO(store_op, loc0, scratch);
        } else if (src->IsReloaction() || src->IsConstant()) {
            // adrp x19, dest@PAGE
            // add x19, x19, dest@PAGEOFF
            // adrp x20, src@PAGE
            // add x20, x20, src@PAGEOFF
            // ldr s19, [x20, 0]
            // str s19, [x19, 0]
            auto tmp1 = operands_.registers()->GeneralScratch1(MachineRepresentation::kWord64);
            current()->NewIO(Arm64Adrp, tmp1, src);
            current()->NewIO(Arm64AddOff, tmp1, tmp1, src);
            auto loc1 = new (arena_) LocationOperand(Arm64Mode_MRI, tmp1->register_id(), -1, 0);
            current()->NewIO(load_op, scratch, loc1);
            current()->NewIO(store_op, loc0, scratch);
        } else {
            UNREACHABLE();
        }
    } else {
        UNREACHABLE();
    }
}

size_t Arm64FunctionInstructionSelector::ReturningValSizeInBytes(const ir::PrototypeModel *proto) {
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

size_t Arm64FunctionInstructionSelector::OverflowParametersSizeInBytes(const ir::Function *fun) {
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

Arm64InstructionGenerator::Arm64InstructionGenerator(base::Arena *arena, ir::Module *module, ConstantsPool *const_pool,
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

Arm64InstructionGenerator::~Arm64InstructionGenerator() {
}

void Arm64InstructionGenerator::Run() {
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

void Arm64InstructionGenerator::GenerateFun(ir::StructureModel *owns, ir::Function *fun) {
    Arm64FunctionInstructionSelector selector(arena_, const_pool_, symbols_, lables_.get(), owns, fun,
                                              optimizing_level_ > 0 /*use_registers_allocation*/);
    selector.Prepare();
    selector.Run();
    funs_[fun->full_name()->ToSlice()] = selector.bundle();
}

void Arm64InstructionGenerator::PrepareGlobalValues() {
    for (auto global_val : module_->values()) {
        if (global_val->op()->IsConstant()) {
            UniquifyConstant(global_val);
        } else {
            
        }
    }
}

std::tuple<int, bool> Arm64InstructionGenerator::UniquifyConstant(ir::Value *kval) {
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

const StackConfiguration *Arm64StackConf() { return kStackConf.Get(); }
const RegisterConfiguration *Arm64RegisterConf() { return kRegConf.Get(); }




} // namespace backend
} // namespace yalx
