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

static const int kAllocatableGeneralRegisters[] = {
    kRAX,
    //kRCX,
    kRDX,
    //kRBX,
    kRSI,
    kRDI,
    kR8,
    kR9,  // 9
    kR10, // 10
    kR11,
    kR12,
    //kR13,
    kR14, // 14
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
    xmm13.code(),
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
    xmm13.code(),
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
    X64FunctionInstructionSelector(base::Arena *arena, ir::StructureModel *owns, ir::Function *fun,
                                   bool use_registers_allocation)
    : arena_(arena)
    , owns_(owns)
    , fun_(fun)
    , operands_(kStackConf.Get(), kRegConf.Get(),
                use_registers_allocation
                ? OperandAllocator::kRegisterFirst
                : OperandAllocator::kStackOnly, arena)
    , blocks_(arena) {}
    
    void Prepare() {
        int label = 0;
        for (auto blk : fun_->blocks()) {
            blocks_[blk] = new (arena_) InstructionBlock(arena_, label++);
        }
        operands_.Prepare(fun_);
    }
    
    void Run() {
        auto blk = blocks_[fun_->entry()];
        blk->NewI(X64Push, operands_.registers()->frame_pointer()); // push rbp
        stack_size_ = ImmediateOperand::Word32(arena_, 0);
        blk->NewIO(X64Sub, operands_.registers()->stack_pointer(), stack_size_);
        
        ProcessParameters(blk);
        for (auto blk : fun_->blocks()) {
            ProcessBasicBlock(blk);
        }
        
        stack_size_->Set32(static_cast<int32_t>(operands_.slots()->max_stack_size()));
    }
    
private:
    void ProcessBasicBlock(ir::BasicBlock *bb) {
        auto block = blocks_[bb];
        operands_.ReleaseDeads(instruction_position_++);
        for (auto instr : bb->instructions()) {
            Select(block, instr);
        }
    }
    
    
    void Select(InstructionBlock *block, ir::Value *val);
    void ProcessParameters(InstructionBlock *block);
    InstructionOperand *CopyArgumentValue(InstructionBlock *block, ir::Type ty, InstructionOperand *from);
    
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
    OperandAllocator operands_;
    base::ArenaMap<ir::BasicBlock *, InstructionBlock *> blocks_;
    int instruction_position_ = 0;
    ImmediateOperand *stack_size_ = nullptr;
}; // class X64FunctionInstructionSelector

void X64FunctionInstructionSelector::Select(InstructionBlock *block, ir::Value *val) {
    InstructionOperand *opd = nullptr;
    if (val->type().kind() != ir::Type::kVoid) {
        opd = operands_.Allocated(val);
        if (!opd) {
            opd = operands_.Allocate(val);
        }
    }
    switch (val->op()->value()) {
        case ir::Operator::kBr:
            break;
            
        case ir::Operator::kAdd: {
            auto lhs = operands_.Allocated(val->InputValue(0));
            auto rhs = operands_.Allocated(val->InputValue(1));
            block->NewIO(X64Movq, opd, lhs);
            block->NewIO(X64Add, opd, rhs);
            
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
    X64FunctionInstructionSelector selector(arena_, owns, fun, false/*use_registers_allocation*/);
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
