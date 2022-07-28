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
#include "ir/utils.h"
#include "compiler/constants.h"
#include "runtime/object/type.h"
#include "runtime/object/any.h"
#include "runtime/runtime.h"
#include "runtime/process.h"
#include "base/lazy-instance.h"
#include "base/arena-utils.h"
#include "base/format.h"
#include "base/io.h"

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
    arm64::x2.code(),
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
                                              0, // saved size
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
    
    // The handle of C code calling yalx code
    void BuildNativeHandle();
    
    // The stub of yalx code calling C code
    void BuildNativeStub();

    DISALLOW_IMPLICIT_CONSTRUCTORS(Arm64FunctionInstructionSelector);
private:
    InstructionBlock *current() { return DCHECK_NOTNULL(current_block_); }
    
    void AssociateParameters(InstructionBlock *block);
    void SelectBasicBlock(ir::BasicBlock *block);
    void Select(ir::Value *instr);
    
    void PutField(ir::Value *instr);
    void Concat(ir::Value *instr);
    void ArrayFill(ir::Value *instr);
    void ArrayAt(ir::Value *instr);
    void ArraySet(ir::Value *instr);
    void Closure(ir::Value *instr);
    void Call(ir::Value *instr);
    void CallRuntime(ir::Value *instr);
    void PassArguments(InstructionOperand *exclude, ir::Value *instr, RegisterSavingScope *saving_scope);
    void ConditionBr(ir::Value *instr);
    void BooleanValue(ir::Value *instr, InstructionOperand *opd);
    void ConditionSelect(ir::Value *instr, InstructionOperand *opd, InstructionOperand *true_val,
                         InstructionOperand *false_val);
    
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
    void MoveMemory(InstructionOperand *dest, InstructionOperand *src, int32_t size);
    void CopyMemoryFast(int register0_id, int register1_id, int adjust0, int adjust1, int32_t size);
    
    
    bool IsSuccessorConditionBr(ir::Value *cond) {
        DCHECK(cond->Is(ir::Operator::kICmp) || cond->Is(ir::Operator::kFCmp));
        //const auto i = instruction_position_ + 1;
        if (current_block_) {
            for (auto [bb, ib] : blocks_) {
                if (current() == ib) {
                    const auto i = bb->FindInstruction(cond);
                    if (i >= 0 && i < bb->instructions_size() - 1) {
                        auto instr = bb->instruction(i + 1);
                        return instr->Is(ir::Operator::kBr)
                            && instr->op()->value_in() == 1
                            && instr->InputValue(0) == cond;
                    }
                }
            }
        }
        return false;
    }
    
    void SetCatchHandler(ir::BasicBlock *handler) {
        auto iter = blocks_.find(handler);
        DCHECK(iter != blocks_.end());
        DCHECK(catch_handler_ == nullptr);
        catch_handler_ = iter->second;
    }
    
    void TypingNormalize(InstructionOperand **a, InstructionOperand **b, ir::Type type) {
        TypingNormalize(a, type);
        TypingNormalize(b, type);
    }
    
    void TypingNormalize(InstructionOperand **a, ir::Type type) {
        auto rep = ToMachineRepresentation(type);
        if (auto rl = (*a)->AsRegister()) {
            if (rl->rep() != rep) {
                *a = new (arena_) RegisterOperand(rl->register_id(), rep);
            }
        }
    }
    
    void InstallUnwindHandler();
    void UninstallUnwindHandler();
    void HandleCatch(InstructionBlock *handler);
    
    void SetUpHandleFrame(RegisterOperand *sp, RegisterOperand *fp, RegisterOperand *lr);
    void TearDownHandleFrame(RegisterOperand *sp, RegisterOperand *fp, RegisterOperand *lr);
    void CallOriginalFun();
    
    void SetUpStubFrame(RegisterOperand *sp, RegisterOperand *fp, RegisterOperand *lr);
    void TearDownStubFrame(RegisterOperand *sp, RegisterOperand *fp, RegisterOperand *lr, LocationOperand *scope);
    LocationOperand *CallNativeStub();
    
    static size_t ReturningValSizeInBytes(const ir::PrototypeModel *proto);
    static size_t ParametersSizeInBytes(const ir::Function *fun);
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
    InstructionBlock *catch_handler_ = nullptr;
    std::map<ir::BasicBlock *, InstructionBlock *> blocks_;
    std::vector<InstructionOperand *> tmps_;
    std::vector<OperandAllocator::BorrowedRecord> borrowed_registers_;
    std::vector<ImmediateOperand *> calling_stack_adjust_;
}; // class Arm64FunctionInstructionSelector

void Arm64FunctionInstructionSelector::InstallUnwindHandler() {
    DCHECK(fun_->should_unwind_handle());
    // struct unwind_node in top of stack
    auto addr = operands_.AllocateStackSlot(ir::Types::Word64, 0, StackSlotAllocator::kLinear);
    auto prev = operands_.AllocateStackSlot(ir::Types::Word64, 0, StackSlotAllocator::kLinear);
    DCHECK(prev->k() == -16);
    DCHECK(prev->register0_id() == arm64::fp.code());
    auto handler = operands_.registers()->frame_pointer();

    // current->prev = root->top_unwind_point
    // current->addr = &fun
    // root->top_unwind_point = current
    auto top = new (arena_) LocationOperand(Arm64Mode_MRI, kRootRegister, -1, ROOT_OFFSET_TOP_UNWIND);
    auto scratch = new (arena_) RegisterOperand(arm64::x0.code(), MachineRepresentation::kWord64);
    Move(scratch, top, ir::Types::Word64);
    Move(prev, scratch, ir::Types::Word64);
    
    auto rel = new (arena_) ReloactionOperand(bundle()->symbol(), nullptr, true/*fetch_addr*/);
    Move(scratch, rel, ir::Types::Word64);
    Move(addr, scratch, ir::Types::Word64);

    current()->NewIO(Arm64Sub, scratch, handler, ImmediateOperand::Word32(arena_, -prev->k()));
    Move(top, scratch, ir::Types::Word64);
}

void Arm64FunctionInstructionSelector::UninstallUnwindHandler() {
    DCHECK(fun_->should_unwind_handle());
    // struct unwind_node in top of stack
    //auto addr = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::fp.code(), -1, -8);
    auto prev = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::fp.code(), -1, -16);
    
    auto top = new (arena_) LocationOperand(Arm64Mode_MRI, kRootRegister, -1, ROOT_OFFSET_TOP_UNWIND);
    auto scratch = new (arena_) RegisterOperand(arm64::x0.code(), MachineRepresentation::kWord64);
    Move(scratch, top, ir::Types::Word64);
    auto top_prev = new (arena_) LocationOperand(Arm64Mode_MRI, scratch->register_id(), -1, 0);
    Move(top_prev, prev, ir::Types::Word64);
}

void Arm64FunctionInstructionSelector::BuildNativeHandle() {
    std::string buf;
    LinkageSymbols::BuildNativeHandle(&buf, fun_->full_name()->ToSlice());
    bundle_ = new (arena_) InstructionFunction(arena_, String::New(arena_, buf));
    current_block_ = bundle()->NewBlock(labels_->NextLable());
    auto sp = operands_.registers()->stack_pointer();
    auto fp = operands_.registers()->frame_pointer();
    auto lr = new (arena_) RegisterOperand(arm64::lr.code(), MachineRepresentation::kWord64);
    SetUpHandleFrame(sp, fp, lr);
    CallOriginalFun();
    TearDownHandleFrame(sp, fp, lr);
}

void Arm64FunctionInstructionSelector::SetUpHandleFrame(RegisterOperand *sp, RegisterOperand *fp, RegisterOperand *lr) {
    auto stack_size = static_cast<int>(RoundUp(ReturningValSizeInBytes(fun_->prototype()) + ParametersSizeInBytes(fun_),
                                               kStackConf->stack_alignment_size()));

//    stack_total_size_ = ImmediateOperand::Word32(arena_, stack_size + 96);
//    // sub sp, sp, stack-total-size
//    current()->NewIO(Arm64Sub, sp, sp, stack_total_size_);
//    stack_sp_fp_location_ = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::sp.code(), -1, stack_size + 80);
//    // stp sp, lr, [sp, location]
//    current()->NewIO(Arm64Stp, stack_sp_fp_location_, fp, lr);
    stack_total_size_ = ImmediateOperand::Word32(arena_, stack_size + 96);
    stack_sp_fp_location_ = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::sp.code(), -1, stack_size + 80);
    current()->NewI2O(ArchFrameEnter, fp, stack_total_size_, stack_sp_fp_location_);

    int offset = stack_sp_fp_location_->k();
    for (int i = 19; i < 29; i += 2) {
        offset -= kPointerSize * 2;
        auto slot = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::sp.code(), -1, offset);
        auto p0 = new (arena_) RegisterOperand(i, MachineRepresentation::kWord64);
        auto p1 = new (arena_) RegisterOperand(i + 1, MachineRepresentation::kWord64);
        current()->NewIO(Arm64Stp, slot, p0, p1);
    }
    // add fp, sp, stack-used-size
    stack_used_size_ = ImmediateOperand::Word32(arena_, stack_size);
    current()->NewIO(Arm64Add, fp, sp, stack_used_size_);
}

void Arm64FunctionInstructionSelector::TearDownHandleFrame(RegisterOperand *sp, RegisterOperand *fp, RegisterOperand *lr) {
    auto x0 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[0], MachineRepresentation::kWord64);
    auto x1 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[1], MachineRepresentation::kWord64);
    auto x2 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[2], MachineRepresentation::kWord64);
    auto vals_size = static_cast<int>(RoundUp(ReturningValSizeInBytes(fun_->prototype()),
                                              kStackConf->stack_alignment_size()));
    auto returning_vals_size = ImmediateOperand::Word32(arena_, vals_size);
    
    if (returning_vals_size->word32() > 0) {
        current()->NewIO(Arm64Mov, x0, returning_vals_size);
        // bl reserve_handle_returning_vals -> x0: address
        current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_reserve_handle_returning_vals));
        current()->NewIO(Arm64Mov, x1, sp); // mov x1, sp
        current()->NewIO(Arm64Mov, x2, returning_vals_size); // mov x2, #returning_vals_size
        current()->NewI(ArchCall, bundle()->AddExternalSymbol(kLibc_memcpy)); // bl memcpy(x0, x1, x2)
    }
    
    int offset = stack_sp_fp_location_->k();
    for (int i = 19; i < 29; i += 2) {
        offset -= kPointerSize * 2;
        auto slot = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::sp.code(), -1, offset);
        auto p0 = new (arena_) RegisterOperand(i, MachineRepresentation::kWord64);
        auto p1 = new (arena_) RegisterOperand(i + 1, MachineRepresentation::kWord64);
        current()->NewIO2(Arm64Ldp, p0, p1, slot);
    }
//    current()->NewIO2(Arm64Ldp, fp, lr, stack_sp_fp_location_);
//    current()->NewIO(Arm64Add, sp, sp, stack_total_size_); // sub sp, sp, stack-total-size
//    current()->New(ArchRet);
    current()->NewI2O(ArchFrameExit, fp, stack_total_size_, stack_sp_fp_location_);
}

void Arm64FunctionInstructionSelector::CallOriginalFun() {
    std::vector<std::tuple<InstructionOperand *, InstructionOperand *, ir::Value *>> args;
    
    int general_index = 0, float_index = 0;
    for (auto param : fun_->paramaters()) {
        auto slot = operands_.AllocateStackSlot(param->type(), 0/*padding_size*/, StackSlotAllocator::kLinear);
        RegisterOperand *src = nullptr;
        auto rep = ToMachineRepresentation(param->type());
        if (param->type().IsFloating()) {
            src = new (arena_) RegisterOperand(kFloatArgumentsRegisters[float_index++], rep);
        } else {
            src = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[general_index++], rep);
        }
        Move(slot, src, param->type());
        args.push_back(std::make_tuple(slot, src, param));
    }
    auto rel = bundle()->AddExternalSymbol(kRt_current_root);
    current()->NewI(ArchCall, rel); // bl _current_root
    auto x0 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[0], MachineRepresentation::kWord64);
    auto root = new (arena_) RegisterOperand(kRootRegister, MachineRepresentation::kWord64);
    current()->NewIO(Arm64Mov, root, x0);
    
    for (auto [bak, origin, param] : args) {
        Move(origin, bak, param->type());
        bak->Grab();
        operands_.Free(bak);
    }

    auto sym = symbols_->Mangle(fun_->full_name());
    rel = new (arena_) ReloactionOperand(sym, nullptr);
    current()->NewI(ArchCall, rel); // bl [original_fun]
}

void Arm64FunctionInstructionSelector::BuildNativeStub() {
    bundle_ = new (arena_) InstructionFunction(arena_, symbols_->Mangle(fun_->full_name()));
    current_block_ = bundle()->NewBlock(labels_->NextLable());
    auto sp = operands_.registers()->stack_pointer();
    auto fp = operands_.registers()->frame_pointer();
    auto lr = new (arena_) RegisterOperand(arm64::lr.code(), MachineRepresentation::kWord64);
    SetUpStubFrame(sp, fp, lr);
    auto scope = CallNativeStub();
    TearDownStubFrame(sp, fp, lr, scope);
}

LocationOperand *Arm64FunctionInstructionSelector::CallNativeStub() {
    auto returning_vals_size = static_cast<int>(RoundUp(ReturningValSizeInBytes(fun_->prototype()),
                                                        kStackConf->stack_alignment_size()));
    //auto params_size = fun_->prototype()->params_size() * kPointerSize;
    
    LocationOperand *returning_vals_scope = nullptr;
    if (returning_vals_size > 0) {
        returning_vals_scope = operands_.AllocateStackSlot(OperandAllocator::kVal, sizeof(yalx_returning_vals), 0,
                                                           StackSlotAllocator::kLinear);
    }
    //printd("%s", fun_->full_name()->data());
    std::vector<std::tuple<LocationOperand *, InstructionOperand *, ir::Value *>> args;
    if (returning_vals_size > 0) {
        int general_index = 0, float_index = 0;
        for (auto param : fun_->paramaters()) {
            auto slot = operands_.AllocateStackSlot(param->type(), 0/*padding_size*/, StackSlotAllocator::kLinear);
            RegisterOperand *src = nullptr;
            auto rep = ToMachineRepresentation(param->type());
            DCHECK(rep != MachineRepresentation::kNone);
            DCHECK(rep != MachineRepresentation::kBit);
            if (param->type().IsFloating()) {
                src = new (arena_) RegisterOperand(kFloatArgumentsRegisters[float_index++], rep);
                DCHECK(float_index < kNumberOfFloatArgumentsRegisters);
            } else {
                src = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[general_index++], rep);
                DCHECK(general_index < kNumberOfGeneralArgumentsRegisters);
            }
            Move(slot, src, param->type());
            args.push_back(std::make_tuple(slot, src, param));
        }
        
        auto fp = operands_.registers()->frame_pointer();
        //    void associate_stub_returning_vals(struct yalx_returning_vals *state,
        //                                       address_t returning_addr,
        //                                       size_t reserved_size,
        //                                       address_t fun_addr);
        auto x0 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[0], MachineRepresentation::kWord64);
        auto x1 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[1], MachineRepresentation::kWord64);
        auto x2 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[2], MachineRepresentation::kWord64);
        auto x3 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[3], MachineRepresentation::kWord64);
        current()->NewIO(Arm64Sub, x0, fp, ImmediateOperand::Word32(arena_, -returning_vals_scope->k()));
        current()->NewIO(Arm64Add, x1, fp, ImmediateOperand::Word32(arena_, kPointerSize * 2));
        current()->NewIO(Arm64Mov, x2, ImmediateOperand::Word32(arena_, returning_vals_size));
        current()->NewIO(Arm64Adr, x3, bundle()->AddExternalSymbol(bundle()->symbol()));
        current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_associate_stub_returning_vals));
    } else {
        int general_index = 0, float_index = 0;
        for (auto param : fun_->paramaters()) {
            if (!param->type().IsReference()) {
                if (param->type().IsFloating()) {
                    float_index++;
                } else {
                    general_index++;
                }
                continue;
            }
            auto slot = operands_.AllocateStackSlot(param->type(), 0/*padding_size*/, StackSlotAllocator::kLinear);
            auto rep = ToMachineRepresentation(param->type());
            DCHECK(rep != MachineRepresentation::kNone);
            DCHECK(rep != MachineRepresentation::kBit);
            auto src = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[general_index++], rep);
            DCHECK(general_index <= kNumberOfGeneralArgumentsRegisters);
            Move(slot, src, param->type());
            args.push_back(std::make_tuple(slot, src, param));
        }
    }

    const String *native_stub_symbol = nullptr;
    if (fun_->native_stub_name()) {
        std::string buf("_");
        buf.append(fun_->native_stub_name()->ToString());
        native_stub_symbol = String::New(arena_, buf);
    } else {
        std::string buf;
        LinkageSymbols::BuildNativeStub(&buf, fun_->full_name()->ToSlice());
        native_stub_symbol = String::New(arena_, buf);
    }

    auto fp = operands_.registers()->frame_pointer();
    for (auto [bak, origin, param] : args) {
        if (param->type().IsReference()) {
            DCHECK(bak->register0_id() == fp->register_id());
            DCHECK(bak->k() < 0);
            current()->NewIO(Arm64Sub, origin, fp, ImmediateOperand::Word32(arena_, -bak->k()));
        } else {
            Move(origin, bak, param->type());
        }
    }
    current()->NewI(ArchCall, bundle()->AddExternalSymbol(native_stub_symbol));
    return returning_vals_scope;
}

void Arm64FunctionInstructionSelector::SetUpStubFrame(RegisterOperand *sp, RegisterOperand *fp, RegisterOperand *lr) {
    stack_total_size_ = ImmediateOperand::Word32(arena_, 0);
    stack_used_size_ = ImmediateOperand::Word32(arena_, 0);
    stack_sp_fp_location_ = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::sp.code(), -1, 0);
    current()->NewI2O(ArchFrameEnter, fp, stack_total_size_, stack_sp_fp_location_);
}

void Arm64FunctionInstructionSelector::TearDownStubFrame(RegisterOperand *sp, RegisterOperand *fp, RegisterOperand *lr,
                                                         LocationOperand *scope) {
    const auto returning_vals_size = RoundUp(ReturningValSizeInBytes(fun_->prototype()),
                                             kStackConf->stack_alignment_size());
    auto x0 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[0], MachineRepresentation::kWord64);
    auto root = new (arena_) RegisterOperand(kRootRegister, MachineRepresentation::kWord64);
    
    if (returning_vals_size > 0) {
        auto imm = ImmediateOperand::Word32(arena_, -scope->k());
        current()->NewIO(Arm64Sub, x0, fp, imm);
        current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_yalx_exit_returning_scope));
    }

    current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_current_root)); // x0 -> root
    current()->NewIO(Arm64Mov, root, x0);
    current()->NewI2O(ArchFrameExit, fp, stack_total_size_, stack_sp_fp_location_);
    
    const auto stack_size = RoundUp(operands_.slots()->max_stack_size(), kStackConf->stack_alignment_size());
    stack_total_size_->Set32(stack_size + kPointerSize * 2);
    stack_used_size_->Set32(stack_size);
    stack_sp_fp_location_->set_k(stack_size);
}

void Arm64FunctionInstructionSelector::Run() {
    auto blk = blocks_[fun_->entry()];
    //auto sp = operands_.registers()->stack_pointer();
    auto fp = operands_.registers()->frame_pointer();
//    stack_total_size_ = ImmediateOperand::Word32(arena_, 0);
//    blk->NewIO(Arm64Sub, sp, sp, stack_total_size_); // sub sp, sp, stack-total-size
//    //stack_sp_fp_offset_ = ImmediateOperand::word32(arena_, 0);
//    stack_sp_fp_location_ = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::sp.code(), -1, 0);
//    // stp sp, lr, [sp, location]
//    blk->NewIO(Arm64Stp, stack_sp_fp_location_, fp,
//               new (arena_) RegisterOperand(arm64::lr.code(), MachineRepresentation::kWord64));
//    // add fp, sp, stack-used-size
//    stack_used_size_ = ImmediateOperand::Word32(arena_, 0);
//    blk->NewIO(Arm64Add, fp, sp, stack_used_size_);
    
    stack_total_size_ = ImmediateOperand::Word32(arena_, 0);
    stack_sp_fp_location_ = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::sp.code(), -1, 0);
    stack_used_size_ = ImmediateOperand::Word32(arena_, 0);
    blk->NewI2O(ArchFrameEnter, fp, stack_total_size_, stack_sp_fp_location_);
    
    if (fun_->should_unwind_handle()) {
        current_block_ = blk;
        InstallUnwindHandler();
        current_block_ = nullptr;
    }
    
    AssociateParameters(blk);
    for (auto blk : fun_->blocks()) {
        SelectBasicBlock(blk);
    }
    
    const auto stack_size = RoundUp(operands_.slots()->max_stack_size(), kStackConf->stack_alignment_size());
    stack_total_size_->Set32(stack_size + kPointerSize * 2);
    stack_used_size_->Set32(stack_size);
    stack_sp_fp_location_->set_k(stack_size);

    for (auto adjust : calling_stack_adjust_) {
        DCHECK(stack_size >= adjust->word32());
        adjust->Set32(stack_size - adjust->word32());
    }
}

void Arm64FunctionInstructionSelector::HandleCatch(InstructionBlock *handler) {
    //auto opd = operands_.Allocate(ir::Types::Word64);
    auto exception = new (arena_) LocationOperand(Arm64Mode_MRI, kRootRegister, -1, ROOT_OFFSET_EXCEPTION);
    auto scratch = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
    Move(scratch, exception, ir::Types::Word64);
    auto xzr = new (arena_) RegisterOperand(arm64::xzr.code(), MachineRepresentation::kWord64);
    current()->NewII(Arm64Cmp, scratch, xzr);
    auto rel = new (arena_) ReloactionOperand(nullptr, handler);
    current()->NewO(Arm64B_ne, rel);
}

void Arm64FunctionInstructionSelector::AssociateParameters(InstructionBlock *block) {
    current_block_ = block;

    int number_of_float_args = kNumberOfFloatArgumentsRegisters;
    int number_of_general_args = kNumberOfGeneralArgumentsRegisters;
    for (auto param : fun_->paramaters()) {
        auto ty = param->type();
        if (ty.IsFloating()) {
            auto index = kNumberOfFloatArgumentsRegisters - number_of_float_args;
            auto arg = operands_.AllocateReigster(param, kFloatArgumentsRegisters[index]);
            DCHECK(arg != nullptr);
            number_of_float_args--;
        } else {
            auto index = kNumberOfGeneralArgumentsRegisters - number_of_general_args;
            auto arg = operands_.AllocateReigster(param, kGeneralArgumentsRegisters[index]);
            DCHECK(arg != nullptr);
            if (param->type().kind() == ir::Type::kValue &&
                !param->type().IsPointer() && !param->type().IsCompactEnum()) {
                auto opd = operands_.AllocateStackSlot(param->type(), 0/*padding_size*/, StackSlotAllocator::kLinear);
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
        // Allocate phi value in stack slots for keeping its' position.
        auto slot = operands_.Allocated(user.phi);
        if (!slot) {
            slot = operands_.AllocateStackSlot(user.phi, 0, StackSlotAllocator::kFit);
        }
        USE(slot);
        //printd("befor: %p <- %p", slot, user.phi);
    }
    
    for (auto instr : bb->instructions()) {
        operands_.ReleaseDeads(instruction_position_);
        tmps_.clear();
        if (instr->op()->IsTerminator()) {
            for (auto user : bb->phi_node_users()) {
                auto phi_slot = DCHECK_NOTNULL(operands_.Allocated(user.phi));
                //printd("after: %p <- %p", phi_slot, user.phi);
                auto dest = Allocate(user.dest, kAny);
                Move(phi_slot, dest, user.phi->type());
            }
        }
        Select(instr);
        for (auto tmp : tmps_) {
            tmp->Grab();
            operands_.Free(tmp);
        }
        for (const auto &borrow : borrowed_registers_) {
            
            if (borrow.original) {
                Move(borrow.old, borrow.bak, borrow.original->type());
                operands_.Associate(borrow.original, borrow.old);
            } else {
                operands_.Free(borrow.bak);
                operands_.Free(borrow.target);
            }
        }
        borrowed_registers_.clear();
        
        if (catch_handler_) {
            HandleCatch(catch_handler_);
            catch_handler_ = nullptr; // clear catch handler;
        }
        prev_instr_ = instr;
        instruction_position_++;
    }
    
    current_block_ = nullptr;
}

void Arm64FunctionInstructionSelector::Select(ir::Value *instr) {
    switch (instr->op()->value()) {
            
        case ir::Operator::kUnreachable:
            current()->New(ArchUnreachable);
            break;
            
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
            
        case ir::Operator::kStoreGlobal: {
            //printd("--%s", instr->InputValue(0)->name()->data());
            auto global_var = instr->InputValue(0);
            auto rval = Allocate(instr->InputValue(1), kAny);
            auto symbol = symbols_->Mangle(global_var->name());
            auto loc = bundle()->AddExternalSymbol(symbol);
            Move(loc, rval, global_var->type());
        } break;

        case ir::Operator::kLoadGlobal: {
            auto global_var = instr->InputValue(0);
            auto symbol = symbols_->Mangle(global_var->name());
            auto loc = bundle()->AddExternalSymbol(symbol);
            auto lval = Allocate(instr, kAny);
            Move(lval, loc, global_var->type());
        } break;
            
        case ir::Operator::kLazyLoad: {
            auto global_var = instr->InputValue(0);
            auto symbol = symbols_->Mangle(global_var->name());
            auto slot = bundle()->AddExternalSymbol(symbol, true/*fetch_address*/);
            DCHECK(global_var->type().IsReference());
            std::string buf;
            LinkageSymbols::Build(&buf, global_var->type().model()->full_name()->ToSlice());
            buf.append("$class");
            auto clazz = bundle()->AddExternalSymbol(buf, true/*fetch_address*/);
            
            RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
            saving_scope.SaveAll();
            auto arg0 = new (arena_) RegisterOperand(arm64::x0.code(), MachineRepresentation::kWord64);
            Move(arg0, slot, ir::Types::Word64);
            
            auto arg1 = new (arena_) RegisterOperand(arm64::x1.code(), MachineRepresentation::kWord64);
            Move(arg1, clazz, ir::Types::Word64);
            
            current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_lazy_load_object));
            auto ret0 = new (arena_) RegisterOperand(arm64::x0.code(), MachineRepresentation::kWord64);
            Move(Allocate(instr, kMoR), ret0, instr->type());
        } break;
            
        case ir::Operator::kLoadEffectField: {
            auto handle = ir::OperatorWith<const ir::Handle *>::Data(instr->op());
            DCHECK(handle->IsField());
            auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
            
            auto opd = Allocate(instr->InputValue(0), kReg);
            auto loc = new (arena_) LocationOperand(Arm64Mode_MRI, opd->AsRegister()->register_id(), -1,
                                                    static_cast<int32_t>(field->offset));
            auto value = Allocate(instr, kAny);
            Move(value, loc, instr->InputValue(0)->type());
        } break;
            
        case ir::Operator::kStoreEffectField:
            PutField(instr);
            break;
            
        case ir::Operator::kLoadEffectAddress: {
            UNREACHABLE();
        } break;
        
        case ir::Operator::kStoreInlineField: {
            auto handle = ir::OperatorWith<const ir::Handle *>::Data(instr->op());
            DCHECK(handle->IsField());
            auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
            //auto opd = Allocate(instr, kAny);
            
            auto opd = Allocate(instr->InputValue(0), kAny);
            LocationOperand *loc = nullptr;
            if (opd->IsRegister()) {
                auto self = opd->AsRegister();
                loc = new (arena_) LocationOperand(Arm64Mode_MRI, self->register_id(), -1,
                                                   static_cast<int>(field->offset));
            } else {
                DCHECK(opd->IsLocation());
                auto self = opd->AsLocation();
                //printd("%d", self->k());
                loc = new (arena_) LocationOperand(Arm64Mode_MRI, self->register0_id(), -1,
                                                   static_cast<int>(self->k() + field->offset));
            }
            auto value = Allocate(instr->InputValue(1), kAny);
            Move(loc, value, instr->InputValue(1)->type());
            operands_.Associate(instr, opd);
        } break;
            
        case ir::Operator::kStoreAccessField: {
            auto handle = ir::OperatorWith<const ir::Handle *>::Data(instr->op());
            DCHECK(handle->IsField());
            auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
            //auto opd = Allocate(instr, kAny);
            
            auto opd = Allocate(instr->InputValue(0), kReg);
            auto value = Allocate(instr->InputValue(1), kAny);
            // str value, [opd, #offset]
            auto loc = new (arena_) LocationOperand(Arm64Mode_MRI, opd->AsRegister()->register_id(), -1,
                                                    static_cast<int>(field->offset));
            Move(loc, value, instr->InputValue(1)->type());
            operands_.Associate(instr, opd);
        } break;
            
        case ir::Operator::kLoadInlineField: {
            auto handle = ir::OperatorWith<const ir::Handle *>::Data(instr->op());
            DCHECK(handle->IsField());
            auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
            
            auto opd = Allocate(instr->InputValue(0), kAny);
            LocationOperand *loc = nullptr;
            if (opd->IsRegister()) {
                auto self = opd->AsRegister();
                loc = new (arena_) LocationOperand(Arm64Mode_MRI, self->register_id(), -1,
                                                   static_cast<int>(field->offset));
            } else {
                DCHECK(opd->IsLocation());
                auto self = opd->AsLocation();
                loc = new (arena_) LocationOperand(Arm64Mode_MRI, self->register0_id(), -1,
                                                   static_cast<int>(self->k() + field->offset));
            }
            auto value = Allocate(instr, kAny);
            Move(value, loc, instr->type());
        } break;
            
        case ir::Operator::kLoadAccessField: {
            auto handle = ir::OperatorWith<const ir::Handle *>::Data(instr->op());
            DCHECK(handle->IsField());
            auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
            
            auto opd = Allocate(instr->InputValue(0), kReg);
            auto loc = new (arena_) LocationOperand(Arm64Mode_MRI, opd->AsRegister()->register_id(), -1,
                                                    static_cast<int>(field->offset));
            auto value = Allocate(instr, kAny);
            Move(value, loc, instr->type());
        } break;
            
        case ir::Operator::kIsInstanceOf: {
            auto model = ir::OperatorWith<const ir::Model *>::Data(instr->op());
            auto input = Allocate(instr->InputValue(0), kAny);
            
            RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
            saving_scope.SaveAll(); // TODO: SaveCallerOnly
            
            auto x0 = new (arena_) RegisterOperand(arm64::x0.code(), MachineRepresentation::kWord64);
            Move(x0, input, instr->InputValue(0)->type());
            
            std::string symbol;
            LinkageSymbols::Build(&symbol, model->full_name()->ToSlice());
            symbol.append("$class");
            auto rel = bundle()->AddExternalSymbol(symbol, true/*fetch_address*/);
            auto x1 = new (arena_) RegisterOperand(arm64::x1.code(), MachineRepresentation::kWord64);
            Move(x1, rel, ir::Types::Word64);
            rel = bundle()->AddExternalSymbol(kRt_is_instance_of);
            current()->NewI(ArchCall, rel);

            auto opd = Allocate(instr, kMoR);
            TypingNormalize(&opd, reinterpret_cast<InstructionOperand **>(&x0), instr->type());
            Move(opd, x0, instr->type());
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
            if (!IsSuccessorConditionBr(instr)) {
                BooleanValue(instr, Allocate(instr, kReg));
            }
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
            if (!IsSuccessorConditionBr(instr)) {
                BooleanValue(instr, Allocate(instr, kReg));
            }
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
            auto opd = Allocate(instr, kAny);
            auto rel = bundle()->AddExternalSymbol(symbol, true/*fetch_address*/);
            Move(opd, rel, instr->type());
        } break;
            
        case ir::Operator::kLoadAddress: {
            auto input = Allocate(instr->InputValue(0), kAny);
            if (input->IsRegister()) {
                operands_.Associate(instr, input);
                return;
            }
            auto opd = Allocate(instr, kMoR);
            if (auto loc = input->AsLocation()) {
                DCHECK(loc->mode() == Arm64Mode_MRI);
                auto bp = new (arena_) RegisterOperand(loc->register0_id(), MachineRepresentation::kWord64);
                if (auto dest = opd->AsRegister()) {
                    current()->NewIO(Arm64Add, dest, bp, ImmediateOperand::Word32(arena_, loc->k()));
                } else if (auto dest = opd->AsLocation()) {
                    auto scratch = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
                    current()->NewIO(Arm64Add, scratch, bp, ImmediateOperand::Word32(arena_, loc->k()));
                    current()->NewIO(Arm64Str, dest, scratch);
                } else {
                    UNREACHABLE();
                }
            }
        } break;
            
        case ir::Operator::kRefAssertedTo: {
            auto input = instr->InputValue(0);
            DCHECK(input->type().IsReference());
            DCHECK(instr->type().IsReference());
            auto from_ty = input->type().model();
            auto to_ty = instr->type().model();
            if (from_ty == to_ty || from_ty->IsBaseOf(to_ty)) {
                Move(Allocate(instr, kMoR), Allocate(input, kAny), instr->type());
            } else {
                RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
                saving_scope.SaveAll();
                auto x0 = new (arena_) RegisterOperand(arm64::x0.code(), MachineRepresentation::kWord64);
                Move(x0, Allocate(input, kAny), input->type());
                
                auto x1 = new (arena_) RegisterOperand(arm64::x1.code(), MachineRepresentation::kWord64);
                DCHECK(instr->type().IsReference());
                std::string symbol;
                LinkageSymbols::Build(&symbol, to_ty->full_name()->ToSlice());
                symbol.append("$class");
                auto rel = bundle()->AddExternalSymbol(symbol, true/*fetch_address*/);
                Move(x1, rel, ir::Types::Word64);
                
                rel = bundle()->AddExternalSymbol(kRt_ref_asserted_to);
                current()->NewI(ArchCall, rel);
                Move(Allocate(instr, kMoR), x0, instr->type());
            }
        } break;
            
        case ir::Operator::kStackAlloc:
            operands_.AllocateStackSlot(instr, 0, StackSlotAllocator::kFit);
            break;
            
        case ir::Operator::kHeapAlloc: {
            auto model = ir::OperatorWith<const ir::Model *>::Data(instr->op());
            
            RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
            saving_scope.SaveAll();
            
            std::string symbol;
            LinkageSymbols::Build(&symbol, model->full_name()->ToSlice());
            symbol.append("$class");
            auto rel = bundle()->AddExternalSymbol(symbol, true/*fetch_address*/);
            auto x0 = new (arena_) RegisterOperand(arm64::x0.code(), MachineRepresentation::kWord64);
            Move(x0, rel, ir::Types::Word64);
            rel = bundle()->AddExternalSymbol(kRt_heap_alloc);
            current()->NewI(ArchCall, rel);
            
            auto opd = Allocate(instr, kMoR);
            Move(opd, x0, instr->type());
        } break;
            
        case ir::Operator::kArrayAlloc: {
            auto array_ty = ir::OperatorWith<const ir::ArrayModel *>::Data(instr->op());
            std::vector<InstructionOperand *> capacities;
            capacities.push_back(ImmediateOperand::Word32(arena_, array_ty->dimension_count()));
            for (int i = 0; i < array_ty->dimension_count(); i++) {
                capacities.push_back(Allocate(instr->InputValue(i), kAny));
            }
            
            std::vector<InstructionOperand *> elements;
            for (int i = array_ty->dimension_count(); i < instr->op()->value_in(); i++) {
                elements.push_back(Allocate(instr->InputValue(i), kAny));
            }
            std::vector<LocationOperand *> slots;

            for (int i = instr->op()->value_in() - 1; i >= array_ty->dimension_count(); i--) {
                auto element = elements[i - array_ty->dimension_count()];
                auto slot = operands_.AllocateStackSlot(instr->InputValue(i)->type(), 0, StackSlotAllocator::kLinear);
                Move(slot, element, instr->InputValue(i)->type());
                slots.push_back(slot);
            }
            for (ssize_t i = capacities.size() - 1; i >= 0; i--) {
                auto capacity = capacities[i];
                auto slot = operands_.AllocateStackSlot(ir::Types::Word32, 0, StackSlotAllocator::kLinear);
                Move(slot, capacity, ir::Types::Word32);
                slots.push_back(slot);
            }
            auto top_line = operands_.slots()->stack_size();
            
            RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
            saving_scope.SaveAll();
            
            auto x0 = new (arena_) RegisterOperand(arm64::x0.code(), MachineRepresentation::kWord64);
            Move(x0, bundle()->AddArrayElementClassSymbol(array_ty, true/*fetch_address*/), ir::Types::Word64);
            auto x1 = new (arena_) RegisterOperand(arm64::x1.code(), MachineRepresentation::kWord64);
            
            auto fp = operands_.registers()->frame_pointer();
            current()->NewIO(Arm64Sub, x1, fp, ImmediateOperand::Word32(arena_, top_line));
            auto x2 = new (arena_) RegisterOperand(arm64::x2.code(), MachineRepresentation::kWord32);
            current()->NewIO(Arm64Mov32, x2, ImmediateOperand::Word32(arena_,
                                                                      instr->op()->value_in()
                                                                      - array_ty->dimension_count()));

            current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_array_alloc));

            for (auto slot : slots) {
                slot->Grab();
                operands_.Free(slot);
            }
            auto opd = Allocate(instr, kMoR);
            Move(opd, x0, instr->type());
        } break;
            
        case ir::Operator::kConcat:
            Concat(instr);
            break;
            
        case ir::Operator::kArrayFill:
            ArrayFill(instr);
            break;
            
        case ir::Operator::kArrayAt:
            ArrayAt(instr);
            break;
            
        case ir::Operator::kArraySet:
            ArraySet(instr);
            break;
            
        case ir::Operator::kClosure:
            Closure(instr);
            break;
            
        case ir::Operator::kBitCastTo: {
            auto opd = Allocate(instr, kMoR);
            auto input = Allocate(instr->InputValue(0), kAny);
            Move(opd, input, instr->type());
        } break;
            
        case ir::Operator::kZextTo: {
            auto opd = Allocate(instr, kReg);
            auto input = Allocate(instr->InputValue(0), kMoR);
            auto rep = ToMachineRepresentation(instr->InputValue(0)->type());
            switch (rep) {
                case MachineRepresentation::kWord8:
                    TypingNormalize(&opd, &input, ir::Types::Word32);
                    input->IsRegister()
                        ? current()->NewIO(Arm64Uxtb, opd, input)
                        : current()->NewIO(Arm64Ldrb, opd, input);
                    break;
                case MachineRepresentation::kWord16:
                    TypingNormalize(&opd, &input, ir::Types::Word32);
                    input->IsRegister()
                        ? current()->NewIO(Arm64Uxth, opd, input)
                        : current()->NewIO(Arm64Ldrh, opd, input);
                    break;
                case MachineRepresentation::kWord32:
                    TypingNormalize(&opd, &input, ir::Types::Word32);
                    Move(opd, input, ir::Types::Word32);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case ir::Operator::kSextTo: {
            auto opd = Allocate(instr, kReg);
            auto input = Allocate(instr->InputValue(0), kMoR);
            auto rep = ToMachineRepresentation(instr->InputValue(0)->type());
            switch (rep) {
                case MachineRepresentation::kWord8:
                    TypingNormalize(&opd, &input, ir::Types::Word32);
                    input->IsRegister()
                        ? current()->NewIO(Arm64Sxtb32, opd, input)
                        : current()->NewIO(Arm64Ldrsb, opd, input);
                    break;
                case MachineRepresentation::kWord16:
                    TypingNormalize(&opd, &input, ir::Types::Word32);
                    input->IsRegister()
                        ? current()->NewIO(Arm64Sxth32, opd, input)
                        : current()->NewIO(Arm64Ldrsh, opd, input);
                    break;
                case MachineRepresentation::kWord32:
                    TypingNormalize(&opd, &input, ir::Types::Word64);
                    input->IsRegister()
                        ? current()->NewIO(Arm64Sxtw32, opd, input)
                        : current()->NewIO(Arm64Ldrsw, opd, input);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case ir::Operator::kTruncTo: {
            auto opd = Allocate(instr, kReg);
            auto input = Allocate(instr->InputValue(0), kMoR);
            TypingNormalize(&opd, &input, instr->type());
            Move(opd, input, instr->type());
            auto rep = ToMachineRepresentation(instr->type());
            switch (rep) {
                case MachineRepresentation::kWord8:
                    current()->NewIO(Arm64And32, opd, opd, ImmediateOperand::Word32(arena_, 0xff));
                    break;
                case MachineRepresentation::kWord16:
                    current()->NewIO(Arm64And32, opd, opd, ImmediateOperand::Word32(arena_, 0xffff));
                    break;
                case MachineRepresentation::kWord32:
                    // Ignore
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case ir::Operator::kFPExtTo:
        case ir::Operator::kFPTruncTo:
        case ir::Operator::kUIToFP:
        case ir::Operator::kSIToFP:
        case ir::Operator::kFPToSI:
        case ir::Operator::kFPToUI:
            current()->New(ArchUnreachable); // TODO:
            break;
            
        case ir::Operator::kRefToIface: {
            DCHECK(instr->type().kind() == ir::Type::kValue);
            DCHECK(instr->type().model());
            DCHECK(instr->type().model()->declaration() == ir::Model::kInterface);
            auto iface = down_cast<ir::InterfaceModel>(instr->type().model());
            
            auto input = instr->InputValue(0);
            DCHECK(input->type().kind() == ir::Type::kReference);
            DCHECK(input->type().model());
            DCHECK(input->type().model()->declaration() == ir::Model::kClass);
            
            auto clazz = down_cast<ir::StructureModel>(input->type().model());
            auto index = clazz->ConceptOffsetOf(iface);
            DCHECK(index >= 0);
            
            auto from = Allocate(instr->InputValue(0), kAny);
            auto to = operands_.AllocateStackSlot(instr, 0, StackSlotAllocator::kFit);
            auto loc = new (arena_) LocationOperand(Arm64Mode_MRI, to->register0_id(), -1, to->k());
            Move(loc, from, instr->InputValue(0)->type());
            loc = new (arena_) LocationOperand(Arm64Mode_MRI, to->register0_id(), -1, to->k() + kPointerSize);
            
            RegisterOperand *orign = nullptr;
            if (auto rf = from->AsRegister()) {
                orign = rf;
            } else {
                auto scratch = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
                Move(scratch, from, ir::Types::Word64);
                orign = scratch;
            }
            
            auto klass = new (arena_) LocationOperand(Arm64Mode_MRI, orign->register_id(), -1, ANY_OFFSET_OF_KLASS);
            auto scratch0 = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
            current()->NewIO(Arm64Ldr, scratch0, klass);
            current()->NewIO(Arm64And, scratch0, scratch0, ImmediateOperand::Word64(arena_, ~1ull));
            auto itab = new (arena_) LocationOperand(Arm64Mode_MRI, scratch0->register_id(), -1, CLASS_OFFSET_OF_ITAB);
            current()->NewIO(Arm64Ldr, scratch0, itab);
            
            auto offset_of_index = static_cast<int>(index * kPointerSize);
            if (offset_of_index > 0) {
                current()->NewIO(Arm64Add, scratch0, scratch0, ImmediateOperand::Word32(arena_, offset_of_index));
            }
            current()->NewIO(Arm64Str, loc, scratch0);
        } break;
            
        case ir::Operator::kIfaceToRef: {
            auto input = instr->InputValue(0);
            DCHECK(input->type().kind() == ir::Type::kValue);
            DCHECK(input->type().model());
            DCHECK(input->type().model()->declaration() == ir::Model::kInterface);
            
            //auto iface = down_cast<ir::InterfaceModel>(input->type().model());
            auto from = Allocate(input, kAny);
            auto to = Allocate(instr, kAny);
            if (auto origin = from->AsLocation()) {
                auto owns = new (arena_) LocationOperand(Arm64Mode_MRI, origin->register0_id(), -1, 0);
                Move(to, owns, ir::Types::Word64);
            } else {
                auto rf = DCHECK_NOTNULL(from->AsRegister());
                auto owns = new (arena_) LocationOperand(Arm64Mode_MRI, rf->register_id(), -1, 0);
                Move(to, owns, ir::Types::Word64);
            }
        } break;
            
        case ir::Operator::kCallRuntime:
            CallRuntime(instr);
            break;
            
        case ir::Operator::kCallHandle:
        case ir::Operator::kCallVirtual:
        case ir::Operator::kCallAbstract:
        case ir::Operator::kCallDirectly:
        case ir::Operator::kCallIndirectly:
            Call(instr);
            break;
            
//        case ir::Operator::kCallVirtual:
//            CallVirtual(instr);
//            break;
            
        case ir::Operator::kCatch: {
            auto loc = new (arena_) LocationOperand(Arm64Mode_MRI, kRootRegister, -1, ROOT_OFFSET_EXCEPTION);
            auto opd = Allocate(instr, kAny);
            Move(opd, loc, instr->type());
        } break;
            
        case ir::Operator::kUnwind: {
            auto loc = new (arena_) LocationOperand(Arm64Mode_MRI, kRootRegister, -1, ROOT_OFFSET_EXCEPTION);
            auto xzr = new (arena_) RegisterOperand(arm64::xzr.code(), MachineRepresentation::kWord64);
            Move(loc, xzr, ir::Types::Word64);
        } break;
            
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
            //DCHECK(instr->op()->value_in() == fun_->prototype()->return_types_size());
            for (int i = instr->op()->value_in() - 1; i >= 0; i--) {
                auto ty = fun_->prototype()->return_type(i);
                if (ty.kind() == ir::Type::kVoid) {
                    continue;
                }
                auto ret = Allocate(instr->InputValue(i), kAny);
                auto opd = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::fp.code(), 0,
                                                        static_cast<int>(returning_val_offset));
                returning_val_offset += RoundUp(ty.ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
                Move(opd, ret, ty);
            }
            
            if (fun_->should_unwind_handle()) { UninstallUnwindHandler(); }
            auto fp = operands_.registers()->frame_pointer();
            current()->NewI2O(ArchFrameExit, fp, stack_total_size_, stack_sp_fp_location_);
        } break;
            
        debugging_info:
        default: {
        #ifndef NDEBUG
            ir::PrintingContext ctx(0);
            std::string buf;
            auto file = base::NewMemoryWritableFile(&buf);
            base::PrintingWriter printer(file, true);
            instr->PrintTo(&ctx, &printer);
            printd("%s", buf.c_str());
        #endif
            UNREACHABLE();
        } break;
    }
}

void Arm64FunctionInstructionSelector::PutField(ir::Value *instr) {
    auto handle = ir::OperatorWith<const ir::Handle *>::Data(instr->op());
    DCHECK(handle->IsField());
    auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
    
    const auto is_reference = (field->type.IsReference() || field->type.IsCompactEnum());
    const auto is_chunk = (field->type.kind() == ir::Type::kValue && field->type.model()->RefsMarkSize() > 0);
    
    Allocate(instr->InputValue(0), (is_reference || is_chunk) ? kAny : kReg);
    Allocate(instr->InputValue(1), kAny);
    
    std::unique_ptr<RegisterSavingScope>
    saving_scope((is_reference || is_chunk) ? new RegisterSavingScope(&operands_, instruction_position_, &moving_delegate_)
                 : nullptr);

    if (is_reference || is_chunk) {
        saving_scope->SaveAll();
    }
    auto opd = Allocate(instr->InputValue(0), (is_reference || is_chunk) ? kAny : kReg);
    auto value = Allocate(instr->InputValue(1), kAny);

    LocationOperand *addr = nullptr;
    if (auto base = opd->AsRegister()) {
        addr = new (arena_) LocationOperand(Arm64Mode_MRI, base->register_id(), -1, static_cast<int>(field->offset));
    } else {
        DCHECK(opd->IsLocation());
        auto mem = opd->AsLocation();
        auto bak = operands_.Allocate(ir::Types::Word64);
        if (auto rb = bak->AsRegister()) {
            Move(rb, mem, ir::Types::Word64);
            addr = new (arena_) LocationOperand(Arm64Mode_MRI, rb->register_id(), -1, static_cast<int>(field->offset));
            bak->Grab();
            operands_.Free(bak);
        } else {
            DCHECK(bak->IsLocation());
            auto brd = operands_.BorrowRegister(ir::Types::Word64, bak);
            Move(bak, brd.target, ir::Types::Word64);
            borrowed_registers_.push_back(brd);
            Move(brd.target, mem, ir::Types::Word64);
            addr = new (arena_) LocationOperand(Arm64Mode_MRI, brd.target->register_id(), -1,
                                               static_cast<int>(field->offset));
        }
    }

    if (is_reference) {
        auto arg0 = operands_.AllocateReigster(ir::Types::Word64, arm64::x0.code());
        auto base = new (arena_) RegisterOperand(addr->register0_id(), MachineRepresentation::kWord64);
        current()->NewIO(Arm64Add, arg0, base, ImmediateOperand::Word32(arena_, addr->k()));
        auto arg1 = operands_.AllocateReigster(ir::Types::Word64, arm64::x1.code());
        Move(arg1, value, field->type);

        current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_put_field));
        saving_scope.reset();
        
        arg1->Grab();
        arg0->Grab();
        operands_.Free(arg1);
        operands_.Free(arg0);
    } else if (is_chunk) {
        auto arg0 = operands_.AllocateReigster(ir::Types::Word64, arm64::x0.code());
        Move(arg0, opd, instr->InputValue(0)->type());
        
        auto arg1 = operands_.AllocateReigster(ir::Types::Word64, arm64::x1.code());
        auto base = new (arena_) RegisterOperand(addr->register0_id(), MachineRepresentation::kWord64);
        current()->NewIO(Arm64Add, arg1, base, ImmediateOperand::Word32(arena_, addr->k()));
        
        auto arg2 = operands_.AllocateReigster(ir::Types::Word64, arm64::x2.code());
        if (value->IsRegister()) {
            Move(arg2, value, ir::Types::Word64);
        } else {
            DCHECK(value->IsLocation());
            auto mem = value->AsLocation();
            DCHECK(mem->register0_id() == arm64::fp.code());
            current()->NewIO(Arm64Sub, arg2, operands_.registers()->frame_pointer(),
                             ImmediateOperand::Word32(arena_, std::abs(mem->k())));
        }
        
        
        current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_put_field_chunk));
        saving_scope.reset();
        
        arg2->Grab();
        arg1->Grab();
        arg0->Grab();
        operands_.Free(arg2);
        operands_.Free(arg1);
        operands_.Free(arg0);
    } else {
        Move(addr, value, field->type);
    }

    operands_.Associate(instr, opd);
}

void Arm64FunctionInstructionSelector::Concat(ir::Value *instr) {
    std::vector<InstructionOperand *> parts;
    for (int i = 0; i < instr->op()->value_in(); i++) {
        parts.push_back(Allocate(instr->InputValue(i), kAny));
    }
    
    std::vector<InstructionOperand *> slots;
    for (ssize_t i = parts.size() - 1; i >= 0; i--) {
        auto slot = operands_.AllocateStackSlot(ir::Types::String, 0, StackSlotAllocator::kLinear);
        Move(slot, parts[i], ir::Types::String);
        slots.push_back(slot);
    }
    const auto top_line = operands_.slots()->stack_size();
    
    RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
    saving_scope.SaveAll();
    
    auto a0 = operands_.AllocateReigster(ir::Types::Word64, kGeneralArgumentsRegisters[0]);
    auto fp = operands_.registers()->frame_pointer();
    current()->NewIO(Arm64Sub, a0, fp, ImmediateOperand::Word32(arena_, top_line));
    auto a1 = operands_.AllocateReigster(ir::Types::Word64, kGeneralArgumentsRegisters[1]);
    current()->NewIO(Arm64Mov, a1, ImmediateOperand::Word32(arena_, instr->op()->value_in()));
    current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_concat));
    Move(Allocate(instr, kAny), a0, instr->type());

    for (auto slot : slots) {
        slot->Grab();
        operands_.Free(slot);
    }
    a1->Grab();
    a0->Grab();
    operands_.Free(a1);
    operands_.Free(a0);
}

void Arm64FunctionInstructionSelector::ArrayFill(ir::Value *instr) {
    auto array_ty = ir::OperatorWith<const ir::ArrayModel *>::Data(instr->op());
    
    std::vector<InstructionOperand *> capacites;
    capacites.push_back(ImmediateOperand::Word32(arena_, array_ty->dimension_count()));
    for (int i = 0; i < array_ty->dimension_count(); i++) {
        capacites.push_back(Allocate(instr->InputValue(i), kAny));
    }

    std::vector<InstructionOperand *> slots;
    const auto filling_value_index = instr->op()->value_in() - 1;
    const auto filling_ty = instr->InputValue(filling_value_index)->type();
    const auto filling = Allocate(instr->InputValue(filling_value_index), kAny);
    auto slot = operands_.AllocateStackSlot(filling_ty, 0, StackSlotAllocator::kLinear);
    Move(slot, filling, filling_ty);
    slots.push_back(slot);
    
    for (ssize_t i = capacites.size() - 1; i >= 0; i--) {
        slot = operands_.AllocateStackSlot(ir::Types::Word32, 0, StackSlotAllocator::kLinear);
        Move(slot, capacites[i], ir::Types::Word32);
        slots.push_back(slot);
    }
    const auto top_line = operands_.slots()->stack_size();
                
    RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
    saving_scope.SaveAll();
    
    auto a0 = operands_.AllocateReigster(ir::Types::Word64, arm64::x0.code());
    auto a1 = operands_.AllocateReigster(ir::Types::Word64, arm64::x1.code());

    Move(a0, bundle()->AddArrayElementClassSymbol(array_ty, true/*fetch_address*/), ir::Types::Word64);
    auto fp = operands_.registers()->frame_pointer();
    current()->NewIO(Arm64Sub, a1, fp, ImmediateOperand::Word32(arena_, top_line));
    
    current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_array_fill));

    auto opd = Allocate(instr, kMoR);
    Move(opd, a0, instr->type());
    a0->Grab();
    a1->Grab();
    operands_.Free(a0);
    operands_.Free(a1);
    
    for (auto slot : slots) {
        slot->Grab();
        operands_.Free(slot);
    }
}

void Arm64FunctionInstructionSelector::ArrayAt(ir::Value *instr) {
    const auto number_of_indices = instr->op()->value_in() - 1;
    DCHECK(number_of_indices > 0);
    
    std::vector<InstructionOperand *> indices;
    if (number_of_indices > 3) {
        Allocate(instr->InputValue(0), kAny);
        for (int i = 1; i < instr->op()->value_in(); i++) {
            indices.push_back(Allocate(instr->InputValue(i), kAny));
        }
    } else {
        for (int i = 0; i < instr->op()->value_in(); i++) {
            Allocate(instr->InputValue(i), kAny);
        }
    }
    
    std::vector<LocationOperand *> slots;
    for (ssize_t i = indices.size() - 1; i >= 0; i--) {
        auto slot = operands_.AllocateStackSlot(ir::Types::Word32, 0, StackSlotAllocator::kLinear);
        Move(slot, indices[i], ir::Types::Word32);
        slots.push_back(slot);
    }
    const auto top_line = operands_.slots()->stack_size();
    
    RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
    saving_scope.SaveAll();
    
    std::vector<RegisterOperand *> args;
    args.push_back(operands_.AllocateReigster(ir::Types::Word64, arm64::x0.code()));
    
    auto array = Allocate(instr->InputValue(0), kAny);
    Move(args[0], array, instr->InputValue(0)->type());
    
    switch (number_of_indices) {
        case 1: {
            args.push_back(operands_.AllocateReigster(ir::Types::Word32, arm64::x1.code()));
            
            auto d0 = Allocate(instr->InputValue(1), kAny);
            Move(args[1], d0, instr->InputValue(1)->type());
            current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_array_location_at1));
        } break;
            
        case 2: {
            args.push_back(operands_.AllocateReigster(ir::Types::Word32, arm64::x1.code()));
            args.push_back(operands_.AllocateReigster(ir::Types::Word32, arm64::x2.code()));
            
            auto d0 = Allocate(instr->InputValue(1), kAny);
            Move(args[1], d0, instr->InputValue(1)->type());
            auto d1 = Allocate(instr->InputValue(2), kAny);
            Move(args[2], d1, instr->InputValue(2)->type());
            current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_array_location_at2));
        } break;
            
        case 3: {
            args.push_back(operands_.AllocateReigster(ir::Types::Word32, arm64::x1.code()));
            args.push_back(operands_.AllocateReigster(ir::Types::Word32, arm64::x2.code()));
            args.push_back(operands_.AllocateReigster(ir::Types::Word32, arm64::x3.code()));
            
            auto d0 = Allocate(instr->InputValue(1), kAny);
            Move(args[1], d0, instr->InputValue(1)->type());
            auto d1 = Allocate(instr->InputValue(2), kAny);
            Move(args[2], d1, instr->InputValue(2)->type());
            auto d2 = Allocate(instr->InputValue(3), kAny);
            Move(args[3], d2, instr->InputValue(3)->type());
            current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_array_location_at3));
        } break;
            
        default: {
            args.push_back(operands_.AllocateReigster(ir::Types::Word32, arm64::x1.code()));

            auto fp = operands_.registers()->frame_pointer();
            current()->NewIO(Arm64Sub, args[1], fp, ImmediateOperand::Word32(arena_, top_line));
            current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_array_location_at));
        } break;
    }
    
    auto opd = Allocate(instr, kMoR);
    if (instr->type().kind() == ir::Type::kValue) {
        Move(opd, args[0], instr->type());
    } else {
        auto loc = new (arena_) LocationOperand(Arm64Mode_MRI, args[0]->register_id(), -1, 0);
        Move(opd, loc, instr->type());
    }
    
    for (auto arg : args) {
        arg->Grab();
        operands_.Free(arg);
    }
    for (auto slot : slots) {
        slot->Grab();
        operands_.Free(slot);
    }
}

void Arm64FunctionInstructionSelector::ArraySet(ir::Value *instr) {
    const auto number_of_indices = instr->op()->value_in() - 2;
    DCHECK(number_of_indices > 0);
    const auto index_of_value = instr->op()->value_in() - 1;
    
    std::vector<InstructionOperand *> indices;
    if (number_of_indices > 3) {
        Allocate(instr->InputValue(0), kAny);
        for (int i = 1; i < number_of_indices; i++) {
            indices.push_back(Allocate(instr->InputValue(i), kAny));
        }
        Allocate(instr->InputValue(index_of_value), kAny);
    } else {
        for (int i = 0; i < instr->op()->value_in(); i++) {
            Allocate(instr->InputValue(i), kAny);
        }
    }
    
    std::vector<LocationOperand *> slots;
    for (ssize_t i = indices.size() - 1; i >= 0; i--) {
        auto slot = operands_.AllocateStackSlot(ir::Types::Word32, 0, StackSlotAllocator::kLinear);
        Move(slot, indices[i], ir::Types::Word32);
        slots.push_back(slot);
    }
    const auto top_line = operands_.slots()->stack_size();
    
    RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
    saving_scope.SaveAll();
    
    std::vector<RegisterOperand *> args;
    args.push_back(operands_.AllocateReigster(ir::Types::Word64, arm64::x0.code()));
    Move(args[0], Allocate(instr->InputValue(0), kAny), instr->InputValue(0)->type());
    
    auto ar = ir::OperatorWith<const ir::ArrayModel *>::Data(instr->op());
    if (ar->dimension_count() > 3) {
        args.push_back(operands_.AllocateReigster(ir::Types::Word64, arm64::x1.code()));
        auto fp = operands_.registers()->frame_pointer();
        current()->NewIO(Arm64Sub, args[1], fp, ImmediateOperand::Word32(arena_, top_line));
    }
    
#define RANK1(V, S) V(1) S(1, 2)
#define RANK2(V, S) V(1) V(2) S(2, 3)
#define RANK3(V, S) V(1) V(2) V(3) S(3, 4)
    
#define HANDLE_RANK_1TO3(V, S) \
    case 1: \
        RANK1(V, S); \
        break; \
    case 2: \
        RANK2(V, S); \
        break; \
    case 3: \
        RANK3(V, S); \
        break

    // indices

#define PASS_INDICES_ARGS(n) \
    args.push_back(operands_.AllocateReigster(ir::Types::Word32, arm64::x##n.code())); \
    Move(args[n], Allocate(instr->InputValue(n), kAny), ir::Types::Word32);
    
    if (ar->element_type().IsCompactEnum()) {
        goto refs_set_handle;
    }
    switch (ar->element_type().kind()) {
        refs_set_handle:
        case ir::Type::kString:
        case ir::Type::kReference: {
            
        #define PASS_VALUE_ARG(r, n) \
            args.push_back(operands_.AllocateReigster(ir::Types::Word64, arm64::x##n.code())); \
            Move(args[n], Allocate(instr->InputValue(n), kAny), ir::Types::Word64); \
            current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_array_set_ref##r))
            
            switch (ar->dimension_count()) {
                HANDLE_RANK_1TO3(PASS_INDICES_ARGS, PASS_VALUE_ARG);
                default:
                    DCHECK(ar->dimension_count() >= 3);
                    args.push_back(operands_.AllocateReigster(ir::Types::Word64, arm64::x2.code()));
                    Move(args[2], Allocate(instr->InputValue(index_of_value), kAny), ir::Types::Word64);
                    current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_array_set_ref));
                    break;
            }
            
        #undef PASS_VALUE_ARG
        } break;
            
        case ir::Type::kValue: {
        #define PASS_VALUE_ARG(r, n) \
            args.push_back(operands_.AllocateReigster(ir::Types::Word64, arm64::x##n.code())); \
            if (auto ptr = value->AsRegister()) { \
                Move(args[n], ptr, ir::Types::Word64); \
            } else if (auto mem = value->AsLocation()) { \
                current()->NewIO(Arm64Add, args[n], \
                                 new (arena_) RegisterOperand(mem->register0_id(), MachineRepresentation::kWord64), \
                                 ImmediateOperand::Word32(arena_, mem->k())); \
            } else if (auto rel = value->AsReloaction()) { \
                DCHECK(rel->fetch_address()); \
                Move(args[n], rel, ir::Types::Word64); \
            } else { \
                UNREACHABLE(); \
            } \
            current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_array_set_chunk##r));
            
            auto value = Allocate(instr->InputValue(index_of_value), kAny);
            switch (ar->dimension_count()) {
                HANDLE_RANK_1TO3(PASS_INDICES_ARGS, PASS_VALUE_ARG);
                default:
                    DCHECK(ar->dimension_count() >= 3);
                    args.push_back(operands_.AllocateReigster(ir::Types::Word64, arm64::x2.code()));
                    if (auto ptr = value->AsRegister()) {
                        Move(args[2], ptr, ir::Types::Word64);
                    } else if (auto mem = value->AsLocation()) {
                        current()->NewIO(Arm64Add, args[2],
                                         new (arena_) RegisterOperand(mem->register0_id(), MachineRepresentation::kWord64),
                                         ImmediateOperand::Word32(arena_, mem->k()));
                    } else if (auto rel = value->AsReloaction()) {
                        DCHECK(rel->fetch_address());
                        Move(args[2], rel, ir::Types::Word64);
                    } else {
                        UNREACHABLE();
                    }
                    current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_array_set_chunk));
                    break;
            }
        #undef PASS_VALUE_ARG
        } break;
            
        default: {
            DCHECK(ar->element_type().IsNumber() || ar->element_type().IsPointer());
            auto receiver = new (arena_) LocationOperand(Arm64Mode_MRI, arm64::x0.code(), -1, 0);
            auto value = Allocate(instr->InputValue(index_of_value), kAny);
            
        #define PASS_VALUE_ARG(r, n) \
            current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_array_location_at##r)); \
            Move(receiver, value, instr->InputValue(index_of_value)->type());

            switch (ar->dimension_count()) {
                HANDLE_RANK_1TO3(PASS_INDICES_ARGS, PASS_VALUE_ARG);
                default:
                    DCHECK(ar->dimension_count() >= 3);
                    current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_array_location_at));
                    Move(receiver, value, instr->InputValue(index_of_value)->type());
                    break;
            }
            
        #undef PASS_VALUE_ARG
        } break;
    }

#undef HANDLE_RANK_1TO3
#undef RANK3
#undef RANK2
#undef RANK1
#undef PASS_INDICES_ARGS
    operands_.Associate(instr, Allocate(instr->InputValue(0), kAny));
    for (auto arg : args) {
        arg->Grab();
        operands_.Free(arg);
    }
    for (auto slot : slots) {
        slot->Grab();
        operands_.Free(slot);
    }
}

void Arm64FunctionInstructionSelector::Closure(ir::Value *instr) {
    auto clazz = ir::OperatorWith<const ir::StructureModel *>::Data(instr);
    
    std::vector<std::tuple<InstructionOperand *, ir::Type>> captured_vars;
    for (int i = instr->op()->value_in() - 1; i >= 0; i--) {
        auto var = instr->InputValue(i);
        captured_vars.push_back(std::make_tuple(Allocate(var, kAny), var->type()));
    }

    const auto base_line = operands_.slots()->stack_size();
    std::vector<LocationOperand *> slots;
    for (auto [var, ty] : captured_vars) {
        auto slot = operands_.AllocateStackSlot(ty, 0, StackSlotAllocator::kLinear);
        Move(slot, var, ty);
        slots.push_back(slot);
    }
    const auto top_line = operands_.slots()->stack_size();

    RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
    saving_scope.SaveAll();

    std::string symbol;
    LinkageSymbols::Build(&symbol, clazz->full_name()->ToSlice());
    symbol.append("$class");
    auto rel = bundle()->AddExternalSymbol(symbol, true/*fetch_address*/);
    auto a0 = operands_.AllocateReigster(ir::Types::Word64, kGeneralArgumentsRegisters[0]);
    Move(a0, rel, ir::Types::Word64);
    
    auto a1 = operands_.AllocateReigster(ir::Types::Word64, kGeneralArgumentsRegisters[1]);
    auto fp = operands_.registers()->frame_pointer();
    current()->NewIO(Arm64Sub, a1, fp, ImmediateOperand::Word32(arena_, top_line));
    
    auto a2 = operands_.AllocateReigster(ir::Types::Word64, kGeneralArgumentsRegisters[2]);
    current()->NewIO(Arm64Mov, a2, ImmediateOperand::Word32(arena_, top_line - base_line));

    current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_closure));    
    Move(Allocate(instr, kAny), a0, instr->type());

    for (auto slot : slots) {
        slot->Grab();
        operands_.Free(slot);
    }
    a2->Grab();
    a1->Grab();
    a0->Grab();
    operands_.Free(a0);
    operands_.Free(a1);
    operands_.Free(a2);
}

void Arm64FunctionInstructionSelector::Call(ir::Value *instr) {
    ir::Function *callee = nullptr;
    ir::PrototypeModel *proto = nullptr;
    const ir::Model::Method *method = nullptr;
    if (instr->Is(ir::Operator::kCallDirectly)) {
        callee = ir::OperatorWith<ir::Function *>::Data(instr->op());
        proto = callee->prototype();
    } else if (instr->Is(ir::Operator::kCallIndirectly)) {
        proto = ir::OperatorWith<ir::PrototypeModel *>::Data(instr->op());
    } else {
        DCHECK(instr->Is(ir::Operator::kCallHandle) ||
               instr->Is(ir::Operator::kCallVirtual) ||
               instr->Is(ir::Operator::kCallAbstract));
        auto handle = ir::OperatorWith<const ir::Handle *>::Data(instr->op());
        //printd("%s", handle->name()->data());
        method = std::get<const ir::Model::Method *>(handle->owns()->GetMember(handle));
        callee = method->fun;
        proto = callee->prototype();
    }

    std::vector<ir::Value *> returning_vals;
    returning_vals.push_back(instr);
    if (instr->users().size() > 0) {
        for (auto edge : instr->users()) {
            if (edge.user->Is(ir::Operator::kReturningVal)) {
                returning_vals.push_back(edge.user);
            }
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
    for (int i = 0; i < instr->op()->value_in(); i++) {
        auto arg = instr->InputValue(i);
        const auto size = RoundUp(arg->type().ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
        if (arg->type().IsFloating()) {
            if (float_index < kNumberOfFloatArgumentsRegisters) {
                saving_scope.AddExclude(arg, kFloatArgumentsRegisters[float_index], instruction_position_);
                Allocate(arg, kAny);
            } else {
                // overflow
                overflow_args_size += size;
                overflow_args.push_back(arg);
            }
            float_index++;
        } else {
            if (general_index < kNumberOfGeneralArgumentsRegisters) {
                saving_scope.AddExclude(arg, kGeneralArgumentsRegisters[general_index], instruction_position_);
                Allocate(arg, kAny);
            } else {
                // overflow
                overflow_args_size += size;
                overflow_args.push_back(arg);
            }
            general_index++;
        }
    }
    saving_scope.SaveAll();

    PassArguments(nullptr, instr, &saving_scope);
    
    auto current_stack_size = operands_.slots()->stack_size();
    bool first = true;
    for (auto rv : returning_vals) {
        if (rv->type().kind() == ir::Type::kVoid) {
            continue;
        }
        size_t padding_size = 0;
        if (first) {
            padding_size = RoundUp(current_stack_size, kStackConf->stack_alignment_size()) - current_stack_size;
            first = false;
        }
        auto slot = operands_.AllocateStackSlot(rv, padding_size, StackSlotAllocator::kLinear);
        USE(slot);
    }
    
    if (overflow_args_size > 0) {
        for (auto arg : overflow_args) {
            auto opd = operands_.AllocateStackSlot(arg->type(), 0/*padding_size*/, StackSlotAllocator::kLinear);
            tmps_.push_back(opd);
            Move(opd, Allocate(arg, kAny), arg->type());
        }
    }

    const auto returning_vals_size_in_bytes = ReturningValSizeInBytes(proto);
    current_stack_size = RoundUp(operands_.slots()->stack_size(), kStackConf->stack_alignment_size());
    
    ImmediateOperand *adjust = nullptr;
    auto sp = operands_.registers()->stack_pointer();
    if (returning_vals_size_in_bytes > 0) {
        adjust = ImmediateOperand::Word32(arena_, static_cast<int>(current_stack_size));
        calling_stack_adjust_.push_back(adjust);
        current()->NewIO(Arm64Add, sp, sp, adjust);
    }
    
    if (instr->Is(ir::Operator::kCallDirectly) || instr->Is(ir::Operator::kCallHandle)) {
        auto rel = new (arena_) ReloactionOperand(symbols_->Mangle(callee->full_name()), nullptr);
        current()->NewI(ArchCall, rel);
    } else if (instr->Is(ir::Operator::kCallIndirectly)) {
        auto scratch0 = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
        auto callee = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[0], MachineRepresentation::kWord64);
        auto entry = new (arena_) LocationOperand(Arm64Mode_MRI, callee->register_id(), -1, CLOSURE_OFFSET_OF_ENTRY);
        current()->NewIO(Arm64Ldr, scratch0, entry);
        current()->NewI(ArchCall, scratch0);
    } else if (instr->Is(ir::Operator::kCallAbstract)) {
        auto scratch = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
        auto self = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[0], MachineRepresentation::kWord64);
        current()->NewIO(Arm64Mov, scratch, self);
        auto owns = new (arena_) LocationOperand(Arm64Mode_MRI, scratch->register_id(), -1, 0);
        auto itab = new (arena_) LocationOperand(Arm64Mode_MRI, scratch->register_id(), -1, kPointerSize);
        current()->NewIO(Arm64Ldr, self, owns);
        current()->NewIO(Arm64Ldr, scratch, itab);
        auto offset_of_index = static_cast<int>(method->id_vtab * kPointerSize);
        auto func = new (arena_) LocationOperand(Arm64Mode_MRI, scratch->register_id(), -1, offset_of_index);
        current()->NewIO(Arm64Ldr, scratch, func);
        current()->NewI(ArchCall, scratch);
    } else {
        DCHECK(instr->Is(ir::Operator::kCallVirtual));
        DCHECK(method->in_vtab);
        
        auto klass = new (arena_) LocationOperand(Arm64Mode_MRI,arm64::x0.code(), -1, ANY_OFFSET_OF_KLASS);
        auto scratch0 = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
        Move(scratch0, klass, ir::Types::Word64);
        current()->NewIO(Arm64And, scratch0, scratch0, ImmediateOperand::Word64(arena_, ~1ull));
        auto vtab = new (arena_) LocationOperand(Arm64Mode_MRI, scratch0->register_id(), -1, CLASS_OFFSET_OF_VTAB);
        Move(scratch0, vtab, ir::Types::Word64);
        auto offset_of_index = static_cast<int>(method->id_vtab * sizeof(Address));
        auto fun = new (arena_) LocationOperand(Arm64Mode_MRI, scratch0->register_id(), -1, offset_of_index);
        Move(scratch0, fun, ir::Types::Word64);
        current()->NewI(ArchCall, scratch0);
    }

    if (returning_vals_size_in_bytes > 0) {
        current()->NewIO(Arm64Sub, sp, sp, adjust);
    }
    
    if (instr->op()->control_out() > 0) {
        SetCatchHandler(instr->OutputControl(0));
    }
}

void Arm64FunctionInstructionSelector::CallRuntime(ir::Value *instr) {
    const auto runtime_id = ir::OperatorWith<ir::RuntimeId>::Data(instr->op());
    const String *symbol = nullptr;
    switch (runtime_id.value) {
        case ir::RuntimeId::kPkgInitOnce: {
            symbol = kRt_pkg_init_once;
            DCHECK(instr->op()->value_in() == 2);
        } break;
            
        case ir::RuntimeId::kRaise:
            symbol = kRt_raise;
            break;
            
        default:
            UNREACHABLE();
            break;
    }
    
    RegisterSavingScope saving_scope(&operands_, instruction_position_, &moving_delegate_);
    InstructionOperand *exclude = nullptr;
    int general_index = 0, float_index = 0;
    for (int i = 0; i < instr->op()->value_in(); i++) {
        auto arg = instr->InputValue(i);
        if (symbol == kRt_pkg_init_once && i == 1) {
            const auto pkg_name = ir::OperatorWith<const String *>::Data(arg->op());
            const auto kid = const_pool_->FindOrInsertString(pkg_name);
            const auto linked_name = String::New(arena_, base::Sprintf("Lkzs.%d", kid));
            exclude = new (arena_) ReloactionOperand(linked_name, nullptr, true/*fetch_address*/);
            general_index++;
            continue;
        }
        if (arg->type().IsFloating()) {
            if (float_index < kNumberOfFloatArgumentsRegisters) {
                saving_scope.AddExclude(arg, kFloatArgumentsRegisters[float_index], instruction_position_);
                Allocate(arg, kAny);
            } else {
                // overflow
                UNREACHABLE();
            }
            float_index++;
        } else {
            if (general_index < kNumberOfGeneralArgumentsRegisters) {
                saving_scope.AddExclude(arg, kGeneralArgumentsRegisters[general_index], instruction_position_);
                Allocate(arg, kAny);
            } else {
                // overflow
                UNREACHABLE();
            }
            general_index++;
        }
    }
    saving_scope.SaveAll(); // TODO: SaveCallerOnly
    
    PassArguments(exclude, instr, &saving_scope);
    
    auto rel = new (arena_) ReloactionOperand(symbol, nullptr);
    current()->NewI(ArchCall, rel);
    
    if (instr->op()->control_out() > 0) {
        SetCatchHandler(instr->OutputControl(0));
    }
}

void Arm64FunctionInstructionSelector::PassArguments(InstructionOperand *exclude, ir::Value *instr,
                                                     RegisterSavingScope *saving_scope) {
    int general_index = 0, float_index = 0;
    for (int i = 0; i < instr->op()->value_in(); i++) {
        auto arg = instr->InputValue(i);
        auto opd = (i == 1 && exclude) ? exclude : DCHECK_NOTNULL(operands_.Allocated(arg));
        if (instr->Is(ir::Operator::kCallAbstract)) {
            auto x0 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[0], MachineRepresentation::kWord64);
            auto loc = DCHECK_NOTNULL(opd->AsLocation());
            auto fp = operands_.registers()->frame_pointer();
            current()->NewIO(Arm64Sub, x0, fp, ImmediateOperand::Word32(arena_, std::abs(loc->k())));
            continue;
        }
        if (arg->type().IsFloating()) {
            if (float_index < kNumberOfFloatArgumentsRegisters) {
                const auto rid  = kFloatArgumentsRegisters[float_index];
                if (saving_scope->Include(rid, false/*general*/) && saving_scope->IsNotStill(arg, rid)) {
                    auto dest = new (arena_) RegisterOperand(rid, ToMachineRepresentation(arg->type()));
                    Move(dest, opd, arg->type());
                }
            }
            float_index++;
        } else {
            if (general_index < kNumberOfGeneralArgumentsRegisters) {
                const auto rid = kGeneralArgumentsRegisters[general_index];
                if (saving_scope->Include(rid, true/*general*/) && saving_scope->IsNotStill(arg, rid)) {
                    auto dest = new (arena_) RegisterOperand(rid, ToMachineRepresentation(arg->type()));
                    Move(dest, opd, arg->type());
                }
            }
            general_index++;
        }
    }
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
}

void Arm64FunctionInstructionSelector::BooleanValue(ir::Value *instr, InstructionOperand *opd) {
    auto false_val = new (arena_) RegisterOperand(arm64::xzr.code(), MachineRepresentation::kWord8);
    auto true_val = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord8);
    Move(true_val, ImmediateOperand::Word8(arena_, 1), instr->type());
    ConditionSelect(instr, opd, true_val, false_val);
}

void Arm64FunctionInstructionSelector::ConditionSelect(ir::Value *instr, InstructionOperand *opd,
                                                       InstructionOperand *true_val,
                                                       InstructionOperand *false_val) {
    DCHECK(instr->Is(ir::Operator::kICmp) || instr->Is(ir::Operator::kFCmp));
    if (instr->Is(ir::Operator::kICmp)) {
        switch (ir::OperatorWith<ir::IConditionId>::Data(instr->op()).value) {
            case ir::IConditionId::k_eq:
                current()->NewIO(Arm64Select_eq, opd, true_val, false_val);
                break;
            case ir::IConditionId::k_ne:
                current()->NewIO(Arm64Select_ne, opd, true_val, false_val);
                break;
            case ir::IConditionId::k_slt:
                current()->NewIO(Arm64Select_lt, opd, true_val, false_val);
                break;
            case ir::IConditionId::k_ult:
                current()->NewIO(Arm64Select_ls, opd, true_val, false_val);
                break;
            case ir::IConditionId::k_sle:
                current()->NewIO(Arm64Select_le, opd, true_val, false_val);
                break;
            case ir::IConditionId::k_ule:
                current()->NewIO(Arm64Select_hi, opd, false_val, true_val);
                break;
            case ir::IConditionId::k_sgt:
                current()->NewIO(Arm64Select_gt, opd, true_val, false_val);
                break;
            case ir::IConditionId::k_ugt:
                current()->NewIO(Arm64Select_hi, opd, true_val, false_val);
                break;
            case ir::IConditionId::k_sge:
                current()->NewIO(Arm64Select_ge, opd, true_val, false_val);
                break;
            case ir::IConditionId::k_uge:
                current()->NewIO(Arm64Select_ge, opd, true_val, false_val);
                break;
            default:
                UNREACHABLE();
                break;
        }
    } else {
        DCHECK(instr->Is(ir::Operator::kFCmp));
        auto unordered = operands_.registers()->GeneralScratch1(MachineRepresentation::kWord8);
        switch (ir::OperatorWith<ir::FConditionId>::Data(instr->op()).value) {
                // oeq: yields true if both operands are not a QNAN and op1 is equal to op2.
            case ir::FConditionId::k_oeq:
                current()->NewIO(Arm64Select_eq, opd, true_val, false_val);
                break;
            case ir::FConditionId::k_one:
                current()->NewIO(Arm64Select_ne, opd, true_val, false_val);
                break;
            case ir::FConditionId::k_olt:
                current()->NewIO(Arm64Select_lt, opd, true_val, false_val);
                break;
            case ir::FConditionId::k_ole:
                current()->NewIO(Arm64Select_le, opd, true_val, false_val);
                break;
            case ir::FConditionId::k_ogt:
                current()->NewIO(Arm64Select_hi, opd, true_val, false_val);
                break;
            case ir::FConditionId::k_oge:
                current()->NewIO(Arm64Select_pl, opd, true_val, false_val);
                break;
                // ord: yields true if both operands are not a QNAN.
            case ir::FConditionId::k_ord:
                current()->NewIO(Arm64Select_vc, opd, true_val, false_val);
                break;
                // ueq: yields true if either operand is a QNAN or op1 is equal to op2.
            case ir::FConditionId::k_ueq:
                current()->NewIO(Arm64Select_vs, unordered, true_val, false_val);
                current()->NewIO(Arm64Select_eq, opd, true_val, unordered);
                break;
                // une: yields true if either operand is a QNAN or op1 is not equal to op2.
            case ir::FConditionId::k_une:
                current()->NewIO(Arm64Select_vs, unordered, true_val, false_val);
                current()->NewIO(Arm64Select_ne, opd, true_val, unordered);
                break;
            case ir::FConditionId::k_ult:
                current()->NewIO(Arm64Select_vs, unordered, true_val, false_val);
                current()->NewIO(Arm64Select_lt, opd, true_val, unordered);
                break;
            case ir::FConditionId::k_ule:
                current()->NewIO(Arm64Select_vs, unordered, true_val, false_val);
                current()->NewIO(Arm64Select_le, opd, true_val, unordered);
                break;
            case ir::FConditionId::k_ugt:
                current()->NewIO(Arm64Select_vs, unordered, true_val, false_val);
                current()->NewIO(Arm64Select_hi, opd, true_val, unordered);
                break;
            case ir::FConditionId::k_uge:
                current()->NewIO(Arm64Select_vs, unordered, true_val, false_val);
                current()->NewIO(Arm64Select_pl, opd, true_val, unordered);
                break;
                // uno: yields true if either operand is a QNAN.
            case ir::FConditionId::k_uno:
                current()->NewIO(Arm64Select_vs, opd, true_val, false_val);
                break;
            case ir::FConditionId::k_never:
                Move(opd, false_val, instr->type());
                break;
            case ir::FConditionId::k_always:
                Move(opd, true_val, instr->type());
                break;
            default:
                UNREACHABLE();
                break;
        }
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
                auto kop = Constant(value, 0);
                DCHECK(kop->IsImmediate() || kop->IsConstant());
                auto bak = operands_.Allocate(value->type());
                if (bak->IsRegister()) {
                    tmps_.push_back(bak);
                    Move(bak, kop, value->type());
                    return bak;
                }
                DCHECK(bak->IsLocation());
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
                    DCHECK(old == opd);
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
                DCHECK(opd->IsReloaction());
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

            DCHECK(opd->IsLocation());
            auto bak = operands_.Allocate(value->type());
            if (bak->IsRegister()) {
                Move(bak, opd, value->type());
                auto old = operands_.LinkTo(value, bak);
                DCHECK(old == opd);
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
                DCHECK(kop->IsImmediate() || kop->IsConstant());
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
    DCHECK(dest->IsRegister() || dest->IsLocation() || dest->IsReloaction());
    if (dest->Equals(src)) {
        return;
    }
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
            if (ty.IsPointer() || ty.IsCompactEnum()) {
                MoveMachineWords(Arm64Ldr, Arm64Str, dest, src, MachineRepresentation::kWord64,
                                 MachineRepresentation::kWord64);
                return;
            }
            MoveMemory(dest, src, static_cast<int>(ty.ReferenceSizeInBytes()));
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
            auto tmp0 = operands_.registers()->GeneralScratch0(scratch_rep);
            current()->NewIO(Arm64Mov, tmp0, src);
            current()->NewIO(store_op, dest, tmp0);
            //current()->NewIO(Arm64Mov, dest, src);
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

void Arm64FunctionInstructionSelector::CopyMemoryFast(int register0_id, int register1_id,
                                                      int adjust0,
                                                      int adjust1,
                                                      int32_t size) {
    const auto wide_count = size / 8;
    int32_t offset = 0;
    for (auto i = 0; i < wide_count; i++) {
        offset = i * 8;
        auto dest_loc = new (arena_) LocationOperand(Arm64Mode_MRI, register0_id, -1, adjust0 + offset);
        auto src_loc = new (arena_) LocationOperand(Arm64Mode_MRI, register1_id, -1, adjust1 + offset);
        Move(dest_loc, src_loc, ir::Types::Word64);
    }
    const auto remain = size % 8;
    if (remain > 0) {
        auto dest_loc = new (arena_) LocationOperand(Arm64Mode_MRI, register0_id, -1, adjust0 + offset);
        auto src_loc = new (arena_) LocationOperand(Arm64Mode_MRI, register1_id, -1, adjust1 + offset);
        switch (remain) {
            case 1:
                Move(dest_loc, src_loc, ir::Types::Word8);
                break;
            case 2:
                Move(dest_loc, src_loc, ir::Types::Word16);
                break;
            case 4:
                Move(dest_loc, src_loc, ir::Types::Word32);
                break;
            default:
                UNREACHABLE();
                break;
        }
        
    }
}

void Arm64FunctionInstructionSelector::MoveMemory(InstructionOperand *dest, InstructionOperand *src, int32_t size) {
    if (dest->IsRegister()) {
        if (auto origin = src->AsRegister()) {
            CopyMemoryFast(dest->AsRegister()->register_id(), origin->register_id(), 0, 0, size);
        } else if (auto origin = src->AsLocation()) {
            DCHECK(origin->mode() == Arm64Mode_MRI);
            CopyMemoryFast(dest->AsRegister()->register_id(), origin->register0_id(), 0, origin->k(), size);
        } else if (auto origin = src->AsReloaction()) {
            auto address = operands_.registers()->GeneralScratch1(MachineRepresentation::kWord64);
            current()->NewIO(Arm64Adrp, address, origin);
            current()->NewIO(Arm64AddOff, address, address, origin);
            CopyMemoryFast(dest->AsRegister()->register_id(), address->register_id(), 0, 0, size);
        } else {
            UNREACHABLE();
        }
    } else if (auto target = dest->AsLocation()) {
        DCHECK(target->mode() == Arm64Mode_MRI);
        if (auto origin = src->AsRegister()) {
            CopyMemoryFast(target->register0_id(), origin->register_id(), target->k(), 0, size);
        } else if (auto origin = src->AsLocation()) {
            DCHECK(origin->mode() == Arm64Mode_MRI);
            CopyMemoryFast(target->register0_id(), origin->register0_id(), target->k(), origin->k(), size);
        } else if (auto origin = src->AsReloaction()) {
            auto address = operands_.registers()->GeneralScratch1(MachineRepresentation::kWord64);
            current()->NewIO(Arm64Adrp, address, origin);
            current()->NewIO(Arm64AddOff, address, address, origin);
            CopyMemoryFast(target->register0_id(), address->register_id(), target->k(), 0, size);
        }
    } else if (auto target = dest->AsReloaction()) {
        auto address0 = operands_.registers()->GeneralScratch1(MachineRepresentation::kWord64);
        current()->NewIO(Arm64Adrp, address0, dest);
        current()->NewIO(Arm64AddOff, address0, address0, dest);
        if (auto origin = src->AsRegister()) {
            CopyMemoryFast(address0->register_id(), origin->register_id(), 0, 0, size);
        } else if (auto origin = src->AsLocation()) {
            DCHECK(origin->mode() == Arm64Mode_MRI);
            CopyMemoryFast(address0->register_id(), origin->register0_id(), 0, origin->k(), size);
        } else if (auto origin = src->AsReloaction()) {
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
        size_in_bytes += RoundUp(ty.ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
    }
    return size_in_bytes;
}

size_t Arm64FunctionInstructionSelector::ParametersSizeInBytes(const ir::Function *fun) {
    size_t size_in_bytes = 0;
    for (auto param : fun->paramaters()) {
        auto size = RoundUp(param->type().ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
        size_in_bytes += size;
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
    switch (fun->decoration()) {
        case ir::Function::kDefault:
        case ir::Function::kOverride:
            selector.Prepare();
            selector.Run();
            funs_[fun->full_name()->ToSlice()] = selector.bundle();
            break;
        case ir::Function::kNative:
            selector.BuildNativeStub();
            funs_[fun->full_name()->ToSlice()] = selector.bundle();
            break;
        case ir::Function::kAbstract:
        default:
            break;
    }
    
    
    if (fun->is_native_handle()) {
        Arm64FunctionInstructionSelector builder(arena_, const_pool_, symbols_, lables_.get(), owns, fun,
                                                 optimizing_level_ > 0 /*use_registers_allocation*/);
        builder.BuildNativeHandle();
        selector.bundle()->set_native_handle(builder.bundle());
    }
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
