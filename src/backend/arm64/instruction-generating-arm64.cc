#include "backend/arm64/instruction-generating-arm64.h"
#include "backend/register-allocator.h"
#include "backend/stackslot-allocator.h"
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
static const int kAllocatableGeneralRegisters[] = {
#define DEFINE_CODE(name) arm64::name.code(),
    ALWAYS_ALLOCATABLE_GENERAL_REGISTERS(DEFINE_CODE)
#undef  DEFINE_CODE
};

static const int kAllocatableFloatRegisters[] = {
#define DEFINE_CODE(name) arm64::name.code(),
    FLOAT_REGISTERS(DEFINE_CODE)
#undef  DEFINE_CODE
};

static const int kAllocatableDoubleRegisters[] = {
#define DEFINE_CODE(name) arm64::name.code(),
    ALLOCATABLE_DOUBLE_REGISTERS(DEFINE_CODE)
#undef  DEFINE_CODE
};

static const int kArgumentsRegisters[] = {
    arm64::x0.code(),
    arm64::x1.code(),
    arm64::x3.code(),
    arm64::x4.code(),
    arm64::x5.code(),
    arm64::x6.code(),
    arm64::x7.code(),
};

constexpr static const size_t kNumberOfArgumentsRegisters = arraysize(kArgumentsRegisters);

struct Arm64RegisterConfigurationInitializer {
    static RegisterConfiguration *New(void *chunk) {
        return new (chunk) RegisterConfiguration(arm64::fp.code()/*fp*/,
                                                 arm64::sp.code()/*sp*/,
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

Arm64InstructionGenerator::Arm64InstructionGenerator(base::Arena *arena, ir::Module *module, ConstantsPool *const_pool)
: arena_(arena)
, module_(module)
, const_pool_(const_pool) {
    kRegConf.Get();
    kStackConf.Get();
}

const StackConfiguration *Arm64StackConf() { return kStackConf.Get(); }

} // namespace backend
} // namespace yalx
