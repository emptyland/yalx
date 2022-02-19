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
static const int kScratchGeneralRegister = arm64::x19.code();
static const int kScratchFloatRegister   = arm64::s19.code();
static const int kScratchDoubleRegister  = arm64::d19.code();

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
                                                 kScratchGeneralRegister,
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
    
    void Run() {
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
//        ProcessParameters(blk);
//        for (auto blk : fun_->blocks()) {
//            ProcessBasicBlock(blk);
//        }
        
        const auto stack_size = RoundUp(operands_.slots()->max_stack_size(), kStackConf->stack_alignment_size());
        stack_total_size_->Set32(stack_size + kPointerSize * 2);
        stack_used_size_->Set32(stack_size);
        stack_sp_fp_location_->set_k(stack_size);

        for (auto adjust : calling_stack_adjust_) {
            assert(stack_size >= adjust->word32());
            adjust->Set32(stack_size - adjust->word32());
        }
    }
    
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Arm64FunctionInstructionSelector);
private:
    InstructionBlock *current() { return DCHECK_NOTNULL(current_block_); }
    
    void AssociateParameters(InstructionBlock *block);
    
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
    std::vector<ImmediateOperand *> calling_stack_adjust_;
}; // class Arm64FunctionInstructionSelector

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
                operands_.Move(param, opd);
            }
            
            number_of_general_args--;
        }
    }
    
    current_block_ = nullptr;
}

void Arm64FunctionInstructionSelector::Move(InstructionOperand *dest, InstructionOperand *src, ir::Type ty) {
    assert(dest->IsRegister() || dest->IsLocation());
    switch (ty.kind()) {
        case ir::Type::kInt8:
        case ir::Type::kUInt8:
            if (dest->IsRegister()) {
                if (src->IsRegister()) {
                    current()->NewIO(Arm64Mov, dest, src); // TODO:
                } else if (src->IsLocation() || src->IsConstant() || src->IsReloaction()) {
                    current()->NewIO(ty.IsSigned() ? Arm64Ldrsb : Arm64Ldrb, dest, src);
                } else if (src->IsImmediate()) {
                    current()->NewIO(Arm64Mov, dest, src); // TODO:
                } else {
                    UNREACHABLE();
                }
            } else if (dest->IsLocation() || dest->IsReloaction()) {
                if (src->IsRegister()) {
                    current()->NewIO(Arm64Strb, dest, src);
                } else if (src->IsLocation() || src->IsReloaction() || src->IsConstant()) {
                    auto tmp = operands_.registers()->GeneralScratch(MachineRepresentation::kWord32);
                    current()->NewIO(ty.IsSigned() ? Arm64Ldrsb : Arm64Ldrb, tmp, src);
                    current()->NewIO(Arm64Strb, dest, tmp);
                } else if (src->IsImmediate()) {
                    current()->NewIO(Arm64Mov, dest, src);
                } else {
                    UNREACHABLE();
                }
            } else {
                UNREACHABLE();
            }
            break;
            
        case ir::Type::kInt16:
        case ir::Type::kUInt16:
            if (dest->IsRegister()) {
                if (src->IsRegister()) {
                    current()->NewIO(Arm64Mov, dest, src); // TODO:
                } else if (src->IsLocation() || src->IsConstant() || src->IsReloaction()) {
                    current()->NewIO(ty.IsSigned() ? Arm64Ldrsh : Arm64Ldrh, dest, src);
                } else if (src->IsImmediate()) {
                    current()->NewIO(Arm64Mov, dest, src); // TODO:
                } else {
                    UNREACHABLE();
                }
            } else if (dest->IsLocation() || dest->IsReloaction()) {
                if (src->IsRegister()) {
                    current()->NewIO(Arm64Strh, dest, src);
                } else if (src->IsLocation() || src->IsReloaction() || src->IsConstant()) {
                    auto tmp = operands_.registers()->GeneralScratch(MachineRepresentation::kWord32);
                    current()->NewIO(ty.IsSigned() ? Arm64Ldrsh : Arm64Ldrh, tmp, src);
                    current()->NewIO(Arm64Strh, dest, tmp);
                } else if (src->IsImmediate()) {
                    current()->NewIO(Arm64Mov, dest, src);
                } else {
                    UNREACHABLE();
                }
            } else {
                UNREACHABLE();
            }
            break;
            
        case ir::Type::kInt32:
        case ir::Type::kUInt32:
            if (dest->IsRegister()) {
                if (src->IsRegister()) {
                    current()->NewIO(Arm64Mov, dest, src); // TODO:
                } else if (src->IsLocation() || src->IsConstant() || src->IsReloaction()) {
                    current()->NewIO(ty.IsSigned() ? Arm64Ldrsw : Arm64LdrW, dest, src);
                } else if (src->IsImmediate()) {
                    current()->NewIO(Arm64Mov, dest, src); // TODO:
                } else {
                    UNREACHABLE();
                }
            } else if (dest->IsLocation() || dest->IsReloaction()) {
                if (src->IsRegister()) {
                    current()->NewIO(Arm64StrW, dest, src);
                } else if (src->IsLocation() || src->IsReloaction() || src->IsConstant()) {
                    auto tmp = operands_.registers()->GeneralScratch(MachineRepresentation::kWord32);
                    current()->NewIO(ty.IsSigned() ? Arm64Ldrsw : Arm64LdrW, tmp, src);
                    current()->NewIO(Arm64StrW, dest, tmp);
                } else if (src->IsImmediate()) {
                    current()->NewIO(Arm64Mov, dest, src);
                } else {
                    UNREACHABLE();
                }
            } else {
                UNREACHABLE();
            }
            break;
            
        case ir::Type::kInt64:
        case ir::Type::kUInt64:
        case ir::Type::kReference:
        case ir::Type::kString:
            if (dest->IsRegister()) {
                if (src->IsRegister()) {
                    current()->NewIO(Arm64Mov, dest, src); // TODO:
                } else if (src->IsLocation() || src->IsConstant() || src->IsReloaction()) {
                    current()->NewIO(Arm64Ldr, dest, src);
                } else if (src->IsImmediate()) {
                    current()->NewIO(Arm64Mov, dest, src); // TODO:
                } else {
                    UNREACHABLE();
                }
            } else if (dest->IsLocation() || dest->IsReloaction()) {
                if (src->IsRegister()) {
                    current()->NewIO(Arm64Str, dest, src);
                } else if (src->IsLocation() || src->IsReloaction() || src->IsConstant()) {
                    auto tmp = operands_.registers()->GeneralScratch(MachineRepresentation::kWord64);
                    current()->NewIO(Arm64Ldr, tmp, src);
                    current()->NewIO(Arm64Str, dest, tmp);
                } else if (src->IsImmediate()) {
                    current()->NewIO(Arm64Mov, dest, src);
                } else {
                    UNREACHABLE();
                }
            } else {
                UNREACHABLE();
            }
            break;
            
        case ir::Type::kFloat32:
            if (dest->IsRegister()) {
                if (src->IsRegister()) {
                    current()->NewIO(Arm64FMov, dest, src); // TODO:
                } else if (src->IsLocation() || src->IsConstant() || src->IsReloaction()) {
                    current()->NewIO(Arm64LdrS, dest, src);
                } else {
                    UNREACHABLE();
                }
            } else if (dest->IsLocation() || dest->IsReloaction()) {
                if (src->IsRegister()) {
                    current()->NewIO(Arm64StrS, dest, src);
                } else if (src->IsLocation() || src->IsReloaction() || src->IsConstant()) {
                    auto tmp = operands_.registers()->float_scratch();
                    current()->NewIO(Arm64LdrS, tmp, src);
                    current()->NewIO(Arm64StrS, dest, tmp);
                } else {
                    UNREACHABLE();
                }
            } else {
                UNREACHABLE();
            }
            break;
            
        case ir::Type::kFloat64:
            if (dest->IsRegister()) {
                if (src->IsRegister()) {
                    current()->NewIO(Arm64FMov, dest, src); // TODO:
                } else if (src->IsLocation() || src->IsConstant() || src->IsReloaction()) {
                    current()->NewIO(Arm64LdrD, dest, src);
                } else {
                    UNREACHABLE();
                }
            } else if (dest->IsLocation() || dest->IsReloaction()) {
                if (src->IsRegister()) {
                    current()->NewIO(Arm64StrD, dest, src);
                } else if (src->IsLocation() || src->IsReloaction() || src->IsConstant()) {
                    auto tmp = operands_.registers()->double_scratch();
                    current()->NewIO(Arm64LdrD, tmp, src);
                    current()->NewIO(Arm64StrD, dest, tmp);
                } else {
                    UNREACHABLE();
                }
            } else {
                UNREACHABLE();
            }
            break;
            
        case ir::Type::kValue: {
            if (ty.IsPointer()) {
                // TODO:
                UNREACHABLE();
            }
            
            // TODO:
            UNREACHABLE();
        } break;
            
            
        default:
            UNREACHABLE();
            break;
    }
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
