#include "backend/x64/instruction-generating-x64.h"
#include "backend/register-allocator.h"
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

static const int kArgumentsRegisters[] = {
    Argv_0.code(),
    Argv_1.code(),
    Argv_2.code(),
    Argv_3.code(),
    Argv_4.code(),
    Argv_5.code(),
    Argv_6.code(),
    Argv_7.code(),
};

constexpr static const size_t kNumberOfArgumentsRegisters = arraysize(kArgumentsRegisters);

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

static base::LazyInstance<RegisterConfiguration, X64RegisterConfigurationInitializer> kRegConf;

X64InstructionGenerator::X64InstructionGenerator(base::Arena *arena, ir::Module *module, ConstantsPool *const_pool)
: arena_(arena)
, module_(module)
, const_pool_(const_pool) {
    kRegConf.Get();
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
    base::ArenaMap<ir::BasicBlock *, InstructionBlock *> blocks(arena_);
    for (auto blk : fun->blocks()) {
        blocks[blk] = new (arena_) InstructionBlock(arena_);
    }
    
    RegisterAllocator registers(kRegConf.Get(), arena_);
    for (size_t i = 0; i < std::min(kNumberOfArgumentsRegisters, fun->paramaters_size()); i++) {
        //registers.Alive(fun->paramater(i), 0);
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
