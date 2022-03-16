#include "backend/x64/instruction-generating-x64.h"
#include "backend/operand-allocator.h"
#include "backend/constants-pool.h"
#include "backend/linkage-symbols.h"
#include "backend/instruction.h"
#include "x64/asm-x64.h"
#include "ir/condition.h"
#include "ir/metadata.h"
#include "ir/operator.h"
#include "ir/runtime.h"
#include "ir/node.h"
#include "ir/type.h"
#include "runtime/runtime.h"
#include "base/lazy-instance.h"
#include "base/arena-utils.h"
#include "base/format.h"

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
    //kRBP,
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
                                              0, // saved size
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
        kRoI, // register or immediate
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
            SelectBasicBlock(blk);
        }
        
        const auto stack_size = RoundUp(operands_.slots()->max_stack_size(), kStackConf->stack_alignment_size());
        stack_size_->Set32(stack_size);
        for (auto adjust : calling_stack_adjust_) {
            assert(stack_size >= adjust->word32());
            adjust->Set32(stack_size - adjust->word32());
        }
    }
    
    void BuildNativeStub();
    void BuildNativeHandle();

    DISALLOW_IMPLICIT_CONSTRUCTORS(X64FunctionInstructionSelector);
private:
    InstructionBlock *current() { return DCHECK_NOTNULL(current_block_); }
    
    LocationOperand *CallNativeStub();
    void SetUpStubFrame(RegisterOperand *sp, RegisterOperand *fp);
    void TearDownStubFrame(RegisterOperand *sp, RegisterOperand *fp, LocationOperand *scope);
    void CallOriginalFun();
    std::vector<std::tuple<LocationOperand *, RegisterOperand *>>
    SetUpHandleFrame(RegisterOperand *sp, RegisterOperand *fp);
    void TearDownHandleFrame(const std::vector<std::tuple<LocationOperand *, RegisterOperand *>> &saved_registers,
                             RegisterOperand *sp, RegisterOperand *fp);
    
    void SelectBasicBlock(ir::BasicBlock *bb);
    void Select(ir::Value *instr);
    void CallDirectly(ir::Value *instr);
    void CallRuntime(ir::Value *instr);
    void ConditionBr(ir::Value *instr);
    void ProcessParameters(InstructionBlock *block);
    void BooleanValue(ir::Value *instr, InstructionOperand *opd);
    void ConditionSelect(ir::Value *instr, InstructionOperand *opd, InstructionOperand *true_val,
                         InstructionOperand *false_val);
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
    
    static Policy MatchPolicy(InstructionOperand *opd) {
        if (opd->kind() == InstructionOperand::kRegister) {
            return kAny;
        }
        if (opd->kind() == InstructionOperand::kLocation) {
            return kRoI;
        }
        UNREACHABLE();
        return kAny;
    }
    
    static size_t ReturningValSizeInBytes(const ir::PrototypeModel *proto);
    static size_t ParametersSizeInBytes(const ir::Function *fun);
    static size_t OverflowParametersSizeInBytes(const ir::Function *fun);
    static size_t OverflowArgumentsSizeInBytes(ir::Value *call, std::vector<ir::Value *> *overflow);
    
    bool IsSuccessorConditionBr(ir::Value *cond);
    
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
    ir::Value *prev_instr_ = nullptr;
    int instruction_position_ = 0;
    ImmediateOperand *stack_size_ = nullptr;
    std::map<ir::BasicBlock *, InstructionBlock *> blocks_;
    std::vector<InstructionOperand *> tmps_;
    std::vector<OperandAllocator::BorrowedRecord> borrowed_registers_;
    std::vector<ImmediateOperand *> calling_stack_adjust_;
}; // class X64FunctionInstructionSelector

void X64FunctionInstructionSelector::BuildNativeHandle() {
    std::string buf;
    LinkageSymbols::BuildNativeHandle(&buf, fun_->full_name()->ToSlice());
    bundle_ = new (arena_) InstructionFunction(arena_, String::New(arena_, buf));
    current_block_ = bundle()->NewBlock(labels_->NextLable());
    auto sp = operands_.registers()->stack_pointer();
    auto fp = operands_.registers()->frame_pointer();
    auto saved = SetUpHandleFrame(sp, fp);
    CallOriginalFun();
    TearDownHandleFrame(saved, sp, fp);
    
    stack_size_->Set32(RoundUp(operands_.slots()->max_stack_size(), kStackConf->stack_alignment_size()));
}

std::vector<std::tuple<LocationOperand *, RegisterOperand *>>
X64FunctionInstructionSelector::SetUpHandleFrame(RegisterOperand *sp, RegisterOperand *fp) {
    current()->NewI(X64Push, fp);
    current()->NewIO(X64Movq, fp, sp);
    stack_size_ = ImmediateOperand::Word32(arena_, 0/*placeholder*/);
    current()->NewIO(X64Sub, sp, stack_size_);
    
    std::vector<std::tuple<LocationOperand *, RegisterOperand *>> saved_registers;
    for (size_t i = 0; i < arraysize(kCalleeSaveRegisters); i++) {
        auto slot = operands_.AllocateStackSlot(OperandAllocator::kVal, kPointerSize, 0, StackSlotAllocator::kLinear);
        auto reg  = new (arena_) RegisterOperand(kCalleeSaveRegisters[i], MachineRepresentation::kWord64);
        current()->NewIO(X64Movq, slot, reg);
        saved_registers.push_back(std::make_tuple(slot, reg));
    }
    return std::move(saved_registers);
}

void X64FunctionInstructionSelector::TearDownHandleFrame(
        const std::vector<std::tuple<LocationOperand *, RegisterOperand *>> &saved_registers,
        RegisterOperand *sp, RegisterOperand *fp) {
    auto iacc = new (arena_) RegisterOperand(rax.code(), MachineRepresentation::kWord64);
    auto arg0 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[0], MachineRepresentation::kWord64);
    auto arg1 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[1], MachineRepresentation::kWord64);
    auto arg2 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[2], MachineRepresentation::kWord64);
    auto returning_vals_size = ImmediateOperand::Word32(arena_, RoundUp(ReturningValSizeInBytes(fun_->prototype()),
                                                                        kStackConf->stack_alignment_size()));
    
    if (returning_vals_size->word32() > 0) {
        current()->NewIO(X64Movq, arg0, returning_vals_size);
        current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_reserve_handle_returning_vals));
        current()->NewIO(X64Movq, arg0, iacc);
        current()->NewIO(X64Movq, arg1, sp);
        current()->NewIO(X64Movq, arg2, returning_vals_size);
        current()->NewI(ArchCall, bundle()->AddExternalSymbol(kLibc_memcpy)); // bl memcpy(x0, x1, x2)
    }

    for (auto [bak, origin] : saved_registers) {
        current()->NewIO(X64Movq, origin, bak);
        operands_.Free(bak);
    }

    current()->NewIO(X64Add, sp, stack_size_); // sub sp, sp, stack-total-size
    current()->NewO(X64Pop, fp);
    current()->New(ArchRet);
}

void X64FunctionInstructionSelector::CallOriginalFun() {
    std::vector<std::tuple<InstructionOperand *, InstructionOperand *, ir::Value *>> args;
    
    int general_index = 0, float_index = 0;
    for (auto param : fun_->paramaters()) {
        auto slot = operands_.AllocateStackSlot(param->type(), 0/*padding_size*/, StackSlotAllocator::kLinear);
        auto rep = ToMachineRepresentation(param->type());
        RegisterOperand *src = nullptr;
        if (param->type().IsFloating()) {
            src = new (arena_) RegisterOperand(kFloatArgumentsRegisters[float_index++], rep);
        } else {
            src = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[general_index++], rep);
        }
        Move(slot, src, param->type());
        args.push_back(std::make_tuple(slot, src, param));
    }
    current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_current_root));
    auto acc = new (arena_) RegisterOperand(rax.code(), MachineRepresentation::kWord64);
    auto root = new (arena_) RegisterOperand(kRootRegister, MachineRepresentation::kWord64);
    current()->NewIO(X64Movq, root, acc);
    
    for (auto [bak, origin, param] : args) {
        Move(origin, bak, param->type());
        operands_.Free(bak);
    }
    current()->NewI(ArchCall, bundle()->AddExternalSymbol(symbols_->Mangle(fun_->full_name())));
}

void X64FunctionInstructionSelector::BuildNativeStub() {
    bundle_ = new (arena_) InstructionFunction(arena_, symbols_->Mangle(fun_->full_name()));
    current_block_ = bundle()->NewBlock(labels_->NextLable());
    auto sp = operands_.registers()->stack_pointer();
    auto fp = operands_.registers()->frame_pointer();
    SetUpStubFrame(sp, fp);
    auto scope = CallNativeStub();
    TearDownStubFrame(sp, fp, scope);
    const auto stack_size = RoundUp(operands_.slots()->max_stack_size(), kStackConf->stack_alignment_size());
    stack_size_->Set32(stack_size);
}

LocationOperand *X64FunctionInstructionSelector::CallNativeStub() {
    auto returning_vals_size = RoundUp(ReturningValSizeInBytes(fun_->prototype()), kStackConf->stack_alignment_size());
    auto params_size = fun_->prototype()->params_size() * kPointerSize; // FIXME:
    
    LocationOperand *returning_vals_scope = nullptr;
    if (returning_vals_size > 0) {
        returning_vals_scope = operands_.AllocateStackSlot(OperandAllocator::kVal, sizeof(yalx_returning_vals), 0,
                                                           StackSlotAllocator::kLinear);
    }
    std::vector<std::tuple<InstructionOperand *, InstructionOperand *, ir::Value *>> args;
    if (returning_vals_size > 0) {
        int general_index = 0, float_index = 0;
        for (auto param : fun_->paramaters()) {
            auto slot = operands_.AllocateStackSlot(param->type(), 0/*padding_size*/, StackSlotAllocator::kLinear);
            RegisterOperand *src = nullptr;
            auto rep = ToMachineRepresentation(param->type());
            assert(rep != MachineRepresentation::kNone);
            assert(rep != MachineRepresentation::kBit);
            if (param->type().IsFloating()) {
                src = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[general_index++], rep);
                assert(general_index < kNumberOfGeneralArgumentsRegisters);
            } else {
                src = new (arena_) RegisterOperand(kFloatArgumentsRegisters[float_index++], rep);
                assert(float_index < kNumberOfFloatArgumentsRegisters);
            }
            Move(slot, src, param->type());
            args.push_back(std::make_tuple(slot, src, param));
        }
        
        auto fp = operands_.registers()->frame_pointer();
        //    void associate_stub_returning_vals(struct yalx_returning_vals *state,
        //                                       address_t returning_addr,
        //                                       size_t reserved_size,
        //                                       address_t fun_addr);
        auto arg0 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[0], MachineRepresentation::kWord64);
        auto arg1 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[1], MachineRepresentation::kWord64);
        auto arg2 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[2], MachineRepresentation::kWord64);
        auto arg3 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[3], MachineRepresentation::kWord64);
        current()->NewIO(X64Movq, arg0, fp);
        current()->NewIO(X64Sub,  arg0, ImmediateOperand::Word32(arena_, -returning_vals_scope->k()));
        current()->NewIO(X64Movq, arg1, fp);
        current()->NewIO(X64Add,  arg1, ImmediateOperand::Word32(arena_, kPointerSize * 2));
        current()->NewIO(X64Movq, arg2, ImmediateOperand::Word32(arena_, returning_vals_size));
        current()->NewIO(X64Lea,  arg3, bundle()->AddExternalSymbol(bundle()->symbol()));
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
            assert(rep != MachineRepresentation::kNone);
            assert(rep != MachineRepresentation::kBit);
            
            RegisterOperand *src = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[general_index++], rep);
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
    
    for (auto [bak, origin, param] : args) {
        //assert(returning_vals_size > 0);
        if (param->type().IsReference()) {
            assert(origin->IsRegister());
            assert(bak->IsLocation());
            current()->NewIO(X64Lea, origin, bak);
        } else {
            Move(origin, bak, param->type());
        }
    }
    current()->NewI(ArchCall, bundle()->AddExternalSymbol(native_stub_symbol));
    return returning_vals_scope;
}

void X64FunctionInstructionSelector::SetUpStubFrame(RegisterOperand *sp, RegisterOperand *fp) {
//    pushq %rbp
//    movq %rsp, %rbp
//    subq $32, %rsp
//    ....
//    addq $32, %rsp
//    popq %rbp
//    retq
    stack_size_ = ImmediateOperand::Word32(arena_, 0);
    current()->NewI(X64Push, fp);
    current()->NewIO(X64Movq, fp, sp);
    current()->NewIO(X64Sub, sp, stack_size_);
}

void X64FunctionInstructionSelector::TearDownStubFrame(RegisterOperand *sp, RegisterOperand *fp,
                                                       LocationOperand *scope) {
    const auto returning_vals_size = RoundUp(ReturningValSizeInBytes(fun_->prototype()),
                                             kStackConf->stack_alignment_size());
    auto arg0 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[0], MachineRepresentation::kWord64);
    auto arg1 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[1], MachineRepresentation::kWord64);
    auto arg2 = new (arena_) RegisterOperand(kGeneralArgumentsRegisters[2], MachineRepresentation::kWord64);
    auto acc  = new (arena_) RegisterOperand(rax.code(), MachineRepresentation::kWord64);
    auto root = new (arena_) RegisterOperand(kRootRegister, MachineRepresentation::kWord64);
    
    if (returning_vals_size > 0) {
        auto imm = ImmediateOperand::Word32(arena_, -scope->k());
        current()->NewIO(X64Movq, arg0, fp);
        current()->NewIO(X64Sub, arg0, imm);
        current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_yalx_exit_returning_scope));
    }
    
    current()->NewI(ArchCall, bundle()->AddExternalSymbol(kRt_current_root)); // x0 -> root
    current()->NewIO(X64Movq, root, acc);
    
    current()->NewIO(X64Add, sp, stack_size_); // sub sp, sp, stack-total-size
    current()->NewO(X64Pop, fp);
    current()->New(ArchRet);
}

void X64FunctionInstructionSelector::SelectBasicBlock(ir::BasicBlock *bb) {
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

void X64FunctionInstructionSelector::Select(ir::Value *instr) {
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
            auto handle = ir::OperatorWith<const ir::Handle *>::Data(instr->op());
            assert(handle->IsField());
            auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
            
            auto opd = Allocate(instr->InputValue(0), kMoR);
            auto value = Allocate(instr, kMoR);
            if (auto reg = opd->AsRegister()) {
                auto loc = new (arena_) LocationOperand(X64Mode_MRI, reg->register_id(), -1, field->offset);
                Move(value, loc, field->type);
            } else {
                assert(opd->IsLocation());
                auto mem = opd->AsLocation();
                auto bak = operands_.Allocate(ir::Types::Word64);
                if (auto base = bak->AsRegister()) {
                    current()->NewIO(X64Lea, base, mem);
                    auto loc = new (arena_) LocationOperand(X64Mode_MRI, base->register_id(), -1, field->offset);
                    Move(value, loc, field->type);
                    operands_.Free(bak);
                } else {
                    assert(bak->IsLocation());
                    auto brd = operands_.BorrowRegister(ir::Types::Word64, bak);
                    Move(bak, brd.target, ir::Types::Word64);
                    borrowed_registers_.push_back(brd);
                    current()->NewIO(X64Lea, brd.target, mem);
                    auto loc = new (arena_) LocationOperand(X64Mode_MRI, brd.target->register_id(), -1, field->offset);
                    Move(value, loc, field->type);
                }
            }
        } break;
            
        case ir::Operator::kStoreEffectField: {
            auto handle = ir::OperatorWith<const ir::Handle *>::Data(instr->op());
            assert(handle->IsField());
            auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
            auto opd = Allocate(instr->InputValue(0), kMoR);
            auto value = Allocate(instr->InputValue(1), kAny);
            
            if (auto reg = opd->AsRegister()) {
                auto loc = new (arena_) LocationOperand(X64Mode_MRI, reg->register_id(), -1, field->offset);
                Move(loc, value, instr->InputValue(1)->type());
            } else {
                assert(opd->IsLocation());
                auto mem = opd->AsLocation();
                auto bak = operands_.Allocate(ir::Types::Word64);
                if (auto base = bak->AsRegister()) {
                    current()->NewIO(X64Lea, base, mem);
                    auto loc = new (arena_) LocationOperand(X64Mode_MRI, base->register_id(), -1, field->offset);
                    Move(loc, value, field->type);
                    operands_.Free(bak);
                } else {
                    assert(bak->IsLocation());
                    auto brd = operands_.BorrowRegister(ir::Types::Word64, bak);
                    Move(bak, brd.target, ir::Types::Word64);
                    borrowed_registers_.push_back(brd);
                    current()->NewIO(X64Lea, brd.target, mem);
                    auto loc = new (arena_) LocationOperand(X64Mode_MRI, brd.target->register_id(), -1, field->offset);
                    Move(loc, value, field->type);
                }
            }
            operands_.Associate(instr, opd);
        } break;
            
        case ir::Operator::kStoreAccessField:
        case ir::Operator::kStoreInlineField: {
            auto handle = ir::OperatorWith<const ir::Handle *>::Data(instr->op());
            assert(handle->IsField());
            auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
            //auto opd = Allocate(instr, kAny);
            
            auto opd = Allocate(instr->InputValue(0), kMoR);
            auto value = Allocate(instr->InputValue(1), kAny);
            if (auto reg = opd->AsRegister()) {
                auto loc = new (arena_) LocationOperand(X64Mode_MRI, reg->register_id(), -1, field->offset);
                Move(loc, value, instr->InputValue(1)->type());
            } else if (auto mem = opd->AsLocation()) {
                assert(mem->mode() == X64Mode_MRI);
                auto loc = new (arena_) LocationOperand(X64Mode_MRI, mem->register0_id(), -1, field->offset);
                Move(loc, value, instr->InputValue(1)->type());
            }
            operands_.Associate(instr, opd);
        } break;
            
        case ir::Operator::kLoadAccessField:
        case ir::Operator::kLoadInlineField: {
            auto handle = ir::OperatorWith<const ir::Handle *>::Data(instr->op());
            assert(handle->IsField());
            auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
            
            auto opd = Allocate(instr->InputValue(0), kMoR);
            auto value = Allocate(instr, kAny);
            if (auto reg = opd->AsRegister()) {
                auto loc = new (arena_) LocationOperand(X64Mode_MRI, reg->register_id(), -1, field->offset);
                Move(value, loc, field->type);
            } else if (auto mem = opd->AsLocation()) {
                assert(mem->mode() == X64Mode_MRI);
                auto loc = new (arena_) LocationOperand(X64Mode_MRI, mem->register0_id(), -1, field->offset);
                Move(value, loc, field->type);
            }
        } break;
            
        case ir::Operator::kLoadAddress: {
            assert(instr->type().IsPointer());
            auto input = Allocate(instr->InputValue(0), kAny);
            if (input->IsRegister()) {
                auto opd = Allocate(instr, kAny);
                current()->NewIO(X64Movq, opd, input);
            } else if (input->IsLocation()) {
                auto opd = Allocate(instr, kAny);
                if (opd->IsRegister()) {
                    current()->NewIO(X64Lea, opd, input);
                } else{
                    auto scratch = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
                    current()->NewIO(X64Lea, scratch, input);
                    Move(opd, scratch, ir::Types::Word64);
                }
            } else {
                UNREACHABLE();
            }
        } break;
            
        case ir::Operator::kRefAssertedTo: {
            auto input = instr->InputValue(0);
            assert(input->type().IsReference());
            assert(instr->type().IsReference());
            auto opd = Allocate(instr, kMoR);
            auto from_ty = input->type().model();
            auto to_ty = instr->type().model();
            if (from_ty == to_ty || from_ty->IsBaseOf(to_ty)) {
                Move(opd, Allocate(input, kAny), instr->type());
            } else {
                UNREACHABLE(); // TODO: check it
            }
        } break;
            
        case ir::Operator::kICmp: {
            auto lhs = Allocate(instr->InputValue(0), kMoR);
            auto rhs = Allocate(instr->InputValue(1), MatchPolicy(lhs));
            switch (ToMachineRepresentation(instr->InputValue(0)->type())) {
                case MachineRepresentation::kWord8:
                    current()->NewII(X64Cmp8, lhs, rhs);
                case MachineRepresentation::kWord16:
                    current()->NewII(X64Cmp16, lhs, rhs);
                case MachineRepresentation::kWord32:
                    current()->NewII(X64Cmp32, lhs, rhs);
                    break;
                case MachineRepresentation::kWord64:
                    current()->NewII(X64Cmp, lhs, rhs);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            if (!IsSuccessorConditionBr(instr)) {
                BooleanValue(instr, Allocate(instr, kMoR));
            }
        } break;
            
        case ir::Operator::kFCmp: {
            auto lhs = Allocate(instr->InputValue(0), kMoR);
            auto rhs = Allocate(instr->InputValue(1), MatchPolicy(lhs));
            switch (ToMachineRepresentation(instr->InputValue(0)->type())) {
                case MachineRepresentation::kFloat32:
                    current()->NewII(SSEFloat32Cmp, lhs, rhs);
                    break;
                case MachineRepresentation::kFloat64:
                    current()->NewII(SSEFloat64Cmp, lhs, rhs);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            if (!IsSuccessorConditionBr(instr)) {
                BooleanValue(instr, Allocate(instr, kMoR));
            }
        } break;
            
        case ir::Operator::kAdd: {
            auto opd = Allocate(instr, kMoR);
            auto lhs = Allocate(instr->InputValue(0), kAny);
            auto rhs = Allocate(instr->InputValue(1), MatchPolicy(opd));
            Move(opd, lhs, instr->type());
            switch (ToMachineRepresentation(instr->type())) {
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
            auto opd = Allocate(instr, kMoR);
            auto lhs = Allocate(instr->InputValue(0), kAny);
            auto rhs = Allocate(instr->InputValue(1), MatchPolicy(opd));
            Move(opd, lhs, instr->type());
            switch (ToMachineRepresentation(instr->type())) {
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
            auto fun = ir::OperatorWith<const ir::Function *>::Data(instr->op());
            auto symbol = symbols_->Mangle(fun->full_name());
            auto rel = bundle()->AddExternalSymbol(symbol, true/*fetch_address*/);
            auto opd = Allocate(instr, kAny);
            Move(opd, rel, instr->type());
        } break;

        case ir::Operator::kStackAlloc:
            operands_.AllocateStackSlot(instr, 0, StackSlotAllocator::kFit);
            break;
            
        case ir::Operator::kHeapAlloc: {
            auto model = ir::OperatorWith<const ir::Model *>::Data(instr->op());

            RegisterOperand *opd = nullptr;
            OperandAllocator::BorrowedRecord borrowed = {nullptr,nullptr,nullptr,nullptr};
            if (operands_.IsGeneralRegisterAlive(kRAX)) {
                auto bak = Allocate(instr, kMoR);
                borrowed = operands_.BorrowRegister(instr, bak);
                Move(bak, borrowed.target, instr->type());
                opd = borrowed.target;
            } else {
                opd = operands_.AllocateReigster(instr, kRAX);
            }
            RegisterOperand *arg0 = nullptr;
            if (operands_.IsGeneralRegisterAlive(Argv_0.code())) {
                auto bak = operands_.AllocateStackSlot(ir::Types::Word64, 0, StackSlotAllocator::kFit);
                auto brd = operands_.BorrowRegister(instr, bak);
                Move(bak, brd.target, ir::Types::Word64);
                borrowed_registers_.push_back(brd);
                arg0 = brd.target;
            } else {
                arg0 = new (arena_) RegisterOperand(Argv_0.code(), MachineRepresentation::kWord64);
            }
            std::string symbol;
            LinkageSymbols::Build(&symbol, model->full_name()->ToSlice());
            symbol.append("$class");
            auto rel = bundle()->AddExternalSymbol(symbol, true/*fetch_address*/);
            current()->NewIO(X64Lea, arg0, rel);
            rel = bundle()->AddExternalSymbol(kRt_heap_alloc);
            current()->NewI(ArchCall, rel);
            if (borrowed.target) {
                assert(opd == borrowed.target);
                auto scratch = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
                Move(scratch, borrowed.bak, ir::Types::Word64);
                Move(borrowed.bak, opd, ir::Types::Word64);
                Move(opd, scratch, ir::Types::Word64);
            }
        } break;
            
        case ir::Operator::kClosure: {
            // TODO:
            UNREACHABLE();
        } break;;

        case ir::Operator::kCallRuntime:
            CallRuntime(instr);
            break;
        
        case ir::Operator::kCallHandle:
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
void X64FunctionInstructionSelector::CallDirectly(ir::Value *instr) {
    ir::Function *callee = nullptr;
    if (instr->Is(ir::Operator::kCallDirectly)) {
        callee = ir::OperatorWith<ir::Function *>::Data(instr->op());
    } else {
        assert(instr->Is(ir::Operator::kCallHandle));
        auto handle = ir::OperatorWith<const ir::Handle *>::Data(instr->op());
        auto method = std::get<const ir::Model::Method *>(handle->owns()->GetMember(handle));
        callee = method->fun;
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
    for (int i = 0; i < instr->op()->value_in(); i++) {
        auto arg = instr->InputValue(i);
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
        operands_.AllocateStackSlot(rv, padding_size, StackSlotAllocator::kLinear);
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

void X64FunctionInstructionSelector::CallRuntime(ir::Value *instr) {
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
            if (operands_.IsFloatRegisterAlive(rax.code())) {
                // TODO:
                UNREACHABLE();
            }
        } else {
            if (operands_.IsGeneralRegisterAlive(rax.code())) {
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

void X64FunctionInstructionSelector::BooleanValue(ir::Value *instr, InstructionOperand *opd) {
    assert(instr->Is(ir::Operator::kICmp) || instr->Is(ir::Operator::kFCmp));
    if (instr->Is(ir::Operator::kICmp)) {
        switch (ir::OperatorWith<ir::IConditionId>::Data(instr->op()).value) {
            case ir::IConditionId::k_eq:
                current()->NewO(X64Sete, opd);
                break;
            case ir::IConditionId::k_ne:
                current()->NewO(X64Setne, opd);
                break;
            case ir::IConditionId::k_slt:
                current()->NewO(X64Setl, opd);
                break;
            case ir::IConditionId::k_ult:
                current()->NewO(X64Setb, opd);
                break;
            case ir::IConditionId::k_sle:
                current()->NewO(X64Setle, opd);
                break;
            case ir::IConditionId::k_ule:
                current()->NewO(X64Setbe, opd);
                break;
            case ir::IConditionId::k_sgt:
                current()->NewO(X64Setg, opd);
                break;
            case ir::IConditionId::k_ugt:
                current()->NewO(X64Seta, opd);
                break;
            case ir::IConditionId::k_sge:
                current()->NewO(X64Setge, opd);
                break;
            case ir::IConditionId::k_uge:
                current()->NewO(X64Setae, opd);
                break;
            default:
                UNREACHABLE();
                break;
        }
    } else {
        assert(instr->Is(ir::Operator::kFCmp));
        auto unordered = operands_.registers()->GeneralScratch1(MachineRepresentation::kWord8);
        switch (ir::OperatorWith<ir::FConditionId>::Data(instr->op()).value) {
                // oeq: yields true if both operands are not a QNAN and op1 is equal to op2.
            case ir::FConditionId::k_oeq:
                current()->NewO(X64Sete, opd);
                break;
            case ir::FConditionId::k_one:
                current()->NewO(X64Setne, opd);
                break;
            case ir::FConditionId::k_olt:
                current()->NewO(X64Sete, opd);
                break;
            case ir::FConditionId::k_ole:
                current()->NewO(X64Sete, opd);
                break;
            case ir::FConditionId::k_ogt:
                current()->NewO(X64Sete, opd);
                break;
            case ir::FConditionId::k_oge:
                current()->NewO(X64Sete, opd);
                break;
                // ord: yields true if both operands are not a QNAN.
            case ir::FConditionId::k_ord:
                current()->NewO(X64Sete, opd);
                break;
                // ueq: yields true if either operand is a QNAN or op1 is equal to op2.
            case ir::FConditionId::k_ueq:
                current()->NewO(X64Sete, opd);
                break;
                // une: yields true if either operand is a QNAN or op1 is not equal to op2.
            case ir::FConditionId::k_une:
                current()->NewO(X64Sete, opd);
                break;
            case ir::FConditionId::k_ult:
                current()->NewO(X64Sete, opd);
                break;
            case ir::FConditionId::k_ule:
                current()->NewO(X64Sete, opd);
                break;
            case ir::FConditionId::k_ugt:
                current()->NewO(X64Sete, opd);
                break;
            case ir::FConditionId::k_uge:
                current()->NewO(X64Sete, opd);
                break;
                // uno: yields true if either operand is a QNAN.
            case ir::FConditionId::k_uno:
                current()->NewO(X64Sete, opd);
                break;
            case ir::FConditionId::k_never:
                if (opd->IsRegister()) {
                    current()->NewIO(X64Xor, opd, opd);
                } else {
                    current()->NewIO(X64Movb, opd, ImmediateOperand::Word8(arena_, 0));
                }
                break;
            case ir::FConditionId::k_always:
                current()->NewIO(X64Movb, opd, ImmediateOperand::Word8(arena_, 1));
                if (opd->IsRegister()) {
                    current()->NewIO(X64And, opd, ImmediateOperand::Word8(arena_, 0xff));
                }
                break;
            default:
                UNREACHABLE();
                break;
        }
    }
}

void X64FunctionInstructionSelector::ConditionBr(ir::Value *instr) {
    auto cond = instr->InputValue(0);
    auto if_true = blocks_[instr->OutputControl(0)];
    auto if_false = blocks_[instr->OutputControl(1)];
    auto label = new (arena_) ReloactionOperand(nullptr/*symbol_name*/, if_true);
    auto output = new (arena_) ReloactionOperand(nullptr, if_false);
    if (cond->Is(ir::Operator::kICmp)) {
        if (prev_instr_ != cond) {
            auto cond_val = Allocate(cond, kMoR);
            auto zero = ImmediateOperand::Word8(arena_, 0);
            current()->NewII(X64Cmp8, cond_val, zero);
            current()->NewO(X64Je, output);
        } else {
            switch (ir::OperatorWith<ir::IConditionId>::Data(cond->op()).value) {
                case ir::IConditionId::k_eq:
                    current()->NewO(X64Jne, output);
                    break;
                case ir::IConditionId::k_ne:
                    current()->NewO(X64Je, output);
                    break;
                case ir::IConditionId::k_slt:
                    current()->NewO(X64Jge, output);
                    break;
                case ir::IConditionId::k_ult:
                    current()->NewO(X64Jae, output); // Above use by unsigned
                    break;
                case ir::IConditionId::k_sle:
                    current()->NewO(X64Jg, output);
                    break;
                case ir::IConditionId::k_ule:
                    current()->NewO(X64Ja, output); // Above use by unsigned
                    break;
                case ir::IConditionId::k_sgt:
                    current()->NewO(X64Jle, output);
                    break;
                case ir::IConditionId::k_ugt:
                    current()->NewO(X64Jbe, output); // Blow use by unsigned
                    break;
                case ir::IConditionId::k_sge:
                    current()->NewO(X64Jl, output);
                    break;
                case ir::IConditionId::k_uge:
                    current()->NewO(X64Jb, output); // Blow use by unsigned
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        }
    } else if (cond->Is(ir::Operator::kFCmp)) {
        if (prev_instr_ != cond) {
            auto cond_val = Allocate(cond, kMoR);
            auto zero = ImmediateOperand::Word8(arena_, 0);
            current()->NewII(X64Cmp8, cond_val, zero);
            current()->NewO(X64Je, output);
        } else {
            switch (ir::OperatorWith<ir::FConditionId>::Data(cond->op()).value) {
                    // oeq: yields true if both operands are not a QNAN and op1 is equal to op2.
                case ir::FConditionId::k_oeq:
                    current()->NewO(X64Jne, output);
                    break;
                case ir::FConditionId::k_one:
                    current()->NewO(X64Je, output);
                    break;
                case ir::FConditionId::k_olt:
                    current()->NewO(X64Jge, output);
                    break;
                case ir::FConditionId::k_ole:
                    current()->NewO(X64Jg, output);
                    break;
                case ir::FConditionId::k_ogt:
                    current()->NewO(X64Jle, output);
                    break;
                case ir::FConditionId::k_oge:
                    current()->NewO(X64Jl, output);
                    break;
                    // ord: yields true if both operands are not a QNAN.
                case ir::FConditionId::k_ord:
                    current()->NewO(X64Jpe, output); // Parity odd is ordered
                    break;
                    // Unordered compare
                    // flags     ZF  PF  CF
                    // ----------------------
                    // unordered  1   1   1
                    // greater    0   0   0
                    // less       0   0   1
                    // equal      1   0   0
                    // ueq: yields true if either operand is a QNAN or op1 is equal to op2.
                case ir::FConditionId::k_ueq:
                    current()->NewO(X64Jne, output);
                    current()->NewO(X64Jp, output);
                    break;
                    // une: yields true if either operand is a QNAN or op1 is not equal to op2.
                case ir::FConditionId::k_une:
                    current()->NewO(X64Jne, label);
                    current()->NewO(X64Jp, label);
                    current()->NewO(ArchJmp, output);
                    break;
                case ir::FConditionId::k_ult:
                    current()->NewO(X64Jae, output);
                    break;
                case ir::FConditionId::k_ule:
                    current()->NewO(X64Ja, output);
                    break;
                case ir::FConditionId::k_ugt:
                    current()->NewO(X64Jbe, output);
                    break;
                case ir::FConditionId::k_uge:
                    current()->NewO(X64Jb, output);
                    break;
                    // uno: yields true if either operand is a QNAN.
                case ir::FConditionId::k_uno:
                    current()->NewO(X64Jpo, output); // Parity even is ordered
                    break;
                case ir::FConditionId::k_never:
                    current()->NewO(ArchJmp, output);
                    break;
                case ir::FConditionId::k_always:
                    current()->NewO(ArchJmp, label);
                    current()->New(ArchNop);
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        }
    } else {
        auto cond_val = Allocate(cond, kMoR);
        auto zero = ImmediateOperand::Word8(arena_, 0);
        current()->NewII(X64Cmp8, cond_val, zero);
        current()->NewO(X64Je, output);
    }
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
                auto opd = operands_.Allocate(value->type());
                auto imm = Constant(value);
                Move(opd, imm, value->type());
                tmps_.push_back(opd);
                return opd;
            }
        } break;
            
        case kRoI: {
            if (value->op()->IsConstant()) {
                auto kval = Constant(value);
                if (kval->IsImmediate()) {
                    return kval;
                }
                auto bak = operands_.Allocate(value->type());
                if (bak->IsRegister()) {
                    Move(bak, kval, value->type());
                    tmps_.push_back(bak);
                    return bak;
                }
                assert(bak->IsLocation());
                auto brd = operands_.BorrowRegister(value, bak);
                Move(bak, brd.target, value->type()); // Saving
                Move(brd.target, kval, value->type());
                borrowed_registers_.push_back(brd);
                return brd.target;
            }
            auto opd = operands_.Allocate(value->type());
            if (opd->IsRegister()) {
                operands_.Associate(value, opd);
                return opd;
            }
            assert(opd->IsLocation());
            auto brd = operands_.BorrowRegister(value, opd);
            Move(opd, brd.target, value->type()); // Saving
            borrowed_registers_.push_back(brd);
            return brd.target;
            
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
    if (dest->Equals(src)) {
        return;
    }
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
                if (auto rel = src->AsReloaction(); rel && rel->fetch_address()) {
                    current()->NewIO(X64Lea, dest, rel);
                } else {
                    current()->NewIO(X64Movq, dest, src);
                }
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
                auto tmp = operands_.registers()->GeneralScratch0(MachineRepresentation::kWord64);
                if (CanDirectlyMove(dest, src)) {
                    if (auto rel = src->AsReloaction(); rel != nullptr && rel->fetch_address()) {
                        current()->NewIO(X64Lea, tmp, rel);
                        current()->NewIO(X64Movq, dest, tmp);
                    } else {
                        current()->NewIO(X64Movq, dest, src);
                    }
                } else {
                    if (auto rel = src->AsReloaction(); rel != nullptr && rel->fetch_address()) {
                        current()->NewIO(X64Lea, tmp, rel);
                        current()->NewIO(X64Movq, dest, tmp);
                    } else {
                        current()->NewIO(X64Movq, tmp, src);
                        current()->NewIO(X64Movq, dest, tmp);
                    }
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

size_t X64FunctionInstructionSelector::ParametersSizeInBytes(const ir::Function *fun) {
    size_t size_in_bytes = 0;
    for (auto param : fun->paramaters()) {
        auto size = RoundUp(param->type().ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
        size_in_bytes += size;
    }
    return size_in_bytes;
}

size_t X64FunctionInstructionSelector::ReturningValSizeInBytes(const ir::PrototypeModel *proto) {
    size_t size_in_bytes = 0;
    for (auto ty : proto->return_types()) {
        if (ty.kind() == ir::Type::kVoid) {
            continue;
        }
        size_in_bytes += RoundUp(ty.ReferenceSizeInBytes(), kStackConf->slot_alignment_size());
    }
    return size_in_bytes;
}

size_t X64FunctionInstructionSelector::OverflowParametersSizeInBytes(const ir::Function *fun) {
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

size_t X64FunctionInstructionSelector::OverflowArgumentsSizeInBytes(ir::Value *call,
                                                                    std::vector<ir::Value *> *overflow) {
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

bool X64FunctionInstructionSelector::IsSuccessorConditionBr(ir::Value *cond) {
    assert(cond->Is(ir::Operator::kICmp) || cond->Is(ir::Operator::kFCmp));
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
        X64FunctionInstructionSelector builder(arena_, const_pool_, symbols_, lables_.get(), owns, fun,
                                               optimizing_level_ > 0 /*use_registers_allocation*/);
        builder.BuildNativeHandle();
        selector.bundle()->set_native_handle(builder.bundle());
    }
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
