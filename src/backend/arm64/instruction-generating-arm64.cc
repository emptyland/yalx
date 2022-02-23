#include "backend/arm64/instruction-generating-arm64.h"
#include "backend/stackslot-allocator.h"
#include "backend/operand-allocator.h"
#include "backend/linkage-symbols.h"
#include "backend/constants-pool.h"
#include "backend/instruction.h"
#include "arm64/asm-arm64.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/type.h"
#include "ir/operator.h"
#include "base/arena-utils.h"
#include "base/lazy-instance.h"

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
        
        void MoveTo(InstructionOperand *dest, InstructionOperand *src, ir::Value *val) override {
            owns_->Move(dest, src, val->type());
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
    , owns_(owns)
    , fun_(fun)
    , operands_(kStackConf.Get(), kRegConf.Get(),
                use_registers_allocation
                ? OperandAllocator::kRegisterFirst
                : OperandAllocator::kStackOnly, arena) {}
    
    DEF_PTR_GETTER(InstructionFunction, bundle);
    
    void Prepare() {
        bundle_ = new (arena_) InstructionFunction(arena_, symbols_->Symbolize(fun_->full_name()));
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
    
    InstructionOperand *Allocate(ir::Value *value, Policy policy, uint32_t imm_bits = 0);
    
    InstructionOperand *Constant(ir::Value *value, uint32_t imm_bits);
    
    void Move(InstructionOperand *dest, InstructionOperand *src, ir::Type ty);
    void MoveMachineWords(Instruction::Code load_op, Instruction::Code store_op, InstructionOperand *dest,
                          InstructionOperand *src, MachineRepresentation rep, MachineRepresentation scratch_rep);
    void MoveMachineFloat(Instruction::Code load_op, Instruction::Code store_op, InstructionOperand *dest,
                          InstructionOperand *src, MachineRepresentation rep);
    
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
    
    static size_t ReturningValSizeInBytes(const ir::PrototypeModel *proto);
    static size_t OverflowParametersSizeInBytes(const ir::Function *fun);

    base::Arena *const arena_;
    ir::StructureModel *const owns_;
    ir::Function *const fun_;
    ConstantsPool *const const_pool_;
    LinkageSymbols *const symbols_;
    InstructionBlockLabelGenerator *const labels_;
    OperandAllocator operands_;
    InstructionFunction *bundle_ = nullptr;
    InstructionBlock *current_block_ = nullptr;
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
    stack_total_size_ = ImmediateOperand::Word32(arena_, 0);
    blk->NewIO(Arm64Sub, operands_.registers()->stack_pointer(), operands_.registers()->stack_pointer(),
               stack_total_size_); // sub sp, sp, stack-total-size
    //stack_sp_fp_offset_ = ImmediateOperand::word32(arena_, 0);
    stack_sp_fp_location_ = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::sp.code(), -1, 0);
    // stp sp, lr, [sp, location]
    blk->NewIO(Arm64Stp, stack_sp_fp_location_, operands_.registers()->frame_pointer(),
               new (arena_) RegisterOperand(arm64::lr.code(), MachineRepresentation::kWord64));
    // add fp, sp, stack-used-size
    stack_used_size_ = ImmediateOperand::Word32(arena_, 0);
    blk->NewIO(Arm64Add, operands_.registers()->frame_pointer(), operands_.registers()->stack_pointer(),
               stack_used_size_);
    
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
        Select(instr);
        for (auto tmp : tmps_) { operands_.Free(tmp); }
        for (const auto &borrow : borrowed_registers_) {
            Move(borrow.old, borrow.bak, borrow.original->type());
            if (borrow.original) {
                operands_.Associate(borrow.original, borrow.old);
            } else {
                operands_.Free(borrow.bak);
                operands_.Free(borrow.target);
            }
        }
        instruction_position_++;
    }
    for (auto user : bb->phi_node_users()) {
        auto phi_slot = DCHECK_NOTNULL(operands_.Allocated(user.phi));
        auto dest = Allocate(user.dest, kAny);
        Move(phi_slot, dest, user.phi->type());
    }
    current_block_ = nullptr;
}

void Arm64FunctionInstructionSelector::Select(ir::Value *instr) {
    switch (instr->op()->value()) {
        case ir::Operator::kBr: {
            if (instr->op()->value_in() == 0) {
                auto ib = blocks_[instr->OutputControl(0)];
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
            auto opd = Allocate(instr, kReg);
            auto lhs = Allocate(instr->InputValue(0), kReg);
            auto rhs = Allocate(instr->InputValue(1), kRoI, 12/*imm_bits*/);
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
            UNREACHABLE();
        } break;
            
        case ir::Operator::kLoadFunAddr: {
            auto fun = ir::OperatorWith<const ir::Function *>::Data(instr->op());
            auto symbol = symbols_->Symbolize(fun->full_name());
            bundle()->AddExternalSymbol(fun->full_name()->ToSlice(), symbol);
            
            auto opd = Allocate(instr, kAny);
            auto rel = new (arena_) ReloactionOperand(symbol, nullptr);
            Move(opd, rel, instr->type());
        } break;
            
        case ir::Operator::kClosure: {
            // TODO:
            
        } break;;
            
        case ir::Operator::kCallRuntime: {
            // TODO:
            // PkgInitOnce -> pkg_init_once(void *fun, const char *name)
        } break;
            
        case ir::Operator::kCallDirectly:
            //CallDirectly(instr);
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
            
//            current()->NewIO(X64Add, operands_.registers()->stack_pointer(), stack_size_);
//            current()->NewO(X64Pop, operands_.registers()->frame_pointer());
            current()->New(ArchRet);
        } break;
            
        default:
            UNREACHABLE();
            break;
    }
}

InstructionOperand *Arm64FunctionInstructionSelector::Allocate(ir::Value *value, Policy policy, uint32_t imm_bits) {
    auto opd = operands_.Allocated(value);
    switch (policy) {
        case kReg: {
            if (opd && opd->IsRegister()) {
                return opd;
            }
            if (value->op()->IsConstant()) {
                auto kop = Constant(value, imm_bits);
                assert(kop->IsImmediate() || kop->IsReloaction());
                auto bak = operands_.Allocate(value->type());
                if (bak->IsRegister()) {
                    tmps_.push_back(bak);
                    Move(bak, kop, value->type());
                    return bak;
                }
                assert(bak->IsLocation());
                auto brd = operands_.BorrowRegister(value, bak);
                Move(bak, brd.target, value->type()); // Saving
                borrowed_registers_.push_back(brd);
                
                return brd.target;
            }
            if (!opd) {
                opd = operands_.Allocate(value);
            }
            if (!opd->IsRegister()) {
                auto bak = operands_.Allocate(value->type());
                auto brd = operands_.BorrowRegister(value, bak);
                Move(bak, brd.target, value->type()); // Saving
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
            auto brd = operands_.BorrowRegister(value, bak);
            Move(bak, brd.target, value->type()); // Saving
            borrowed_registers_.push_back(brd);

            return brd.target;
        } break;

        case kMoR: {
            if (opd) {
                return opd;
            }
            if (value->op()->IsConstant()) {
                auto kop = Constant(value, imm_bits);
                assert(kop->IsImmediate() || kop->IsReloaction());
                auto opd = operands_.Allocate(value->type());
                tmps_.push_back(opd);
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
                return Constant(value, imm_bits);
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
    UNREACHABLE();
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
            MoveMachineWords(Arm64Ldrb, Arm64Strb, dest, src, MachineRepresentation::kWord8,
                             MachineRepresentation::kWord32);
            break;
            
        case ir::Type::kInt16:
            MoveMachineWords(Arm64Ldrsw, Arm64StrW, dest, src, MachineRepresentation::kWord16,
                             MachineRepresentation::kWord32);
            break;
        case ir::Type::kUInt16:
            MoveMachineWords(Arm64LdrW, Arm64StrW, dest, src, MachineRepresentation::kWord16,
                             MachineRepresentation::kWord32);
            break;
            
        case ir::Type::kInt32:
        case ir::Type::kUInt32:
            MoveMachineWords(Arm64Ldr, Arm64Str, dest, src, MachineRepresentation::kWord32,
                             MachineRepresentation::kWord32);
            break;
            
        case ir::Type::kInt64:
        case ir::Type::kUInt64:
        case ir::Type::kReference:
        case ir::Type::kString:
            MoveMachineWords(Arm64Ldr, Arm64Str, dest, src, MachineRepresentation::kWord64,
                             MachineRepresentation::kWord64);
            break;
            
        case ir::Type::kFloat32:
            MoveMachineFloat(Arm64LdrS, Arm64StrS, dest, src, MachineRepresentation::kFloat32,
                             MachineRepresentation::kWord32);
            break;
            
        case ir::Type::kFloat64:
            MoveMachineFloat(Arm64LdrD, Arm64StrD, dest, src, MachineRepresentation::kFloat64,
                             MachineRepresentation::kWord64);
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

void Arm64FunctionInstructionSelector::MoveMachineWords(Instruction::Code load_op, Instruction::Code store_op,
                                                        InstructionOperand *dest, InstructionOperand *src,
                                                        MachineRepresentation rep, MachineRepresentation scratch_rep) {
    if (dest->IsRegister()) {
        if (src->IsRegister()) {
            current()->NewIO(Arm64Mov, dest, src);
        } else if (src->IsLocation()) {
            current()->NewIO(load_op, dest, src);
        } else if (src->IsConstant() || src->IsReloaction()) {
            auto tmp = operands_.registers()->GeneralScratch0(scratch_rep);
            current()->NewIO(Arm64Adrp, tmp, src);
            current()->NewIO(Arm64AddOff, tmp, tmp, src);
            auto loc = new (arena_) LocationOperand(Arm64Mode_MRI, kScratchGeneralRegister0, -1, 0);
            current()->NewIO(load_op, dest, loc);
        } else if (src->IsImmediate()) {
            current()->NewIO(Arm64Mov, dest, src);
        } else {
            UNREACHABLE();
        }
    } else if (dest->IsLocation()) {
        auto tmp0 = operands_.registers()->GeneralScratch0(scratch_rep);
        
        if (src->IsRegister()) {
            current()->NewIO(store_op, dest, src);
        } else if (src->IsLocation()) {
            // ldr x19, [src]
            // str x19, [dest]
            current()->NewIO(load_op, tmp0, src);
            current()->NewIO(store_op, dest, tmp0);
        } else if (src->IsReloaction() || src->IsConstant()) {
            // adrp x19, dest@PAGE
            // add x19, x19, dest@PAGEOFF
            // ldr x19, [x19, 0]
            // str x19, [dest]
            current()->NewIO(Arm64Adrp, tmp0, src);
            current()->NewIO(Arm64AddOff, tmp0, tmp0, src);
            auto loc0 = new (arena_) LocationOperand(Arm64Mode_MRI, tmp0->register_id(), -1, 0);
            current()->NewIO(load_op, tmp0, loc0);
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
            current()->NewIO(store_op, loc0, dest);
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
            auto loc1 = new (arena_) LocationOperand(Arm64Mode_MRI, tmp1->register_id(), -1, 0);
            current()->NewIO(load_op, tmp1, loc1);
            current()->NewIO(store_op, loc0, tmp1); // Saving `tmp' aka `src'
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
