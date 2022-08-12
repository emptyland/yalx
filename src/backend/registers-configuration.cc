#include "backend/registers-configuration.h"
#include "base/lazy-instance.h"
#include "arm64/asm-arm64.h"
#include "x64/asm-x64.h"

namespace yalx {

namespace backend {

RegistersConfiguration::RegistersConfiguration(int number_of_argument_gp_registers,
                                               const int *argument_gp_registers,
                                               int number_of_argument_fp_registers,
                                               const int *argument_fp_registers,
                                               int number_of_allocatable_gp_registers,
                                               const int *allocatable_gp_registers,
                                               int number_of_allocatable_fp_registers,
                                               const int *allocatable_fp_registers,
                                               int number_of_callee_save_gp_registers,
                                               const int *callee_save_gp_registers,
                                               int number_of_callee_save_fp_registers,
                                               const int *callee_save_fp_registers,
                                               int max_gp_register,
                                               int max_fp_register,
                                               int scratch0,
                                               int scratch1,
                                               int returning0_register,
                                               int fp,
                                               int sp,
                                               int root)
: number_of_argument_gp_registers_(number_of_argument_gp_registers)
, argument_gp_registers_(argument_gp_registers)
, number_of_argument_fp_registers_(number_of_argument_fp_registers)
, argument_fp_registers_(argument_fp_registers)
, number_of_allocatable_gp_registers_(number_of_allocatable_gp_registers)
, allocatable_gp_registers_(allocatable_gp_registers)
, number_of_allocatable_fp_registers_(number_of_allocatable_fp_registers)
, allocatable_fp_registers_(allocatable_fp_registers)
, number_of_callee_save_gp_registers_(number_of_callee_save_gp_registers)
, callee_save_gp_registers_(callee_save_gp_registers)
, number_of_callee_save_fp_registers_(number_of_callee_save_fp_registers)
, callee_save_fp_registers_(callee_save_fp_registers)
, max_gp_register_(max_gp_register)
, max_fp_register_(max_fp_register)
, scratch0_(scratch0)
, scratch1_(scratch1)
, returning0_register_(returning0_register)
, fp_(fp)
, sp_(sp)
, root_(root) {
}

const RegistersConfiguration *RegistersConfiguration::of_arm64() {
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

    static const int kAllocatableGPRegisters[] = {
    #define DEFINE_CODE(name) arm64::name.code(),
        ALWAYS_ALLOCATABLE_GENERAL_REGISTERS(DEFINE_CODE)
    #undef  DEFINE_CODE
    };

    static const int kAllocatableFPRegisters[] = {
    #define DEFINE_CODE(name) arm64::name.code(),
        ALLOCATABLE_DOUBLE_REGISTERS(DEFINE_CODE)
    #undef  DEFINE_CODE
    };

    static const int kArgumentGPRegisters[] = {
        arm64::x0.code(),
        arm64::x1.code(),
        arm64::x2.code(),
        arm64::x3.code(),
        arm64::x4.code(),
        arm64::x5.code(),
        arm64::x6.code(),
        arm64::x7.code(),
    };

    static const int kArgumentFPRegisters[] = {
        arm64::s0.code(),
        arm64::s1.code(),
        arm64::s2.code(),
        arm64::s3.code(),
        arm64::s4.code(),
        arm64::s5.code(),
        arm64::s6.code(),
        arm64::s7.code(),
    };
    
    static const int kCalleeSaveGPRegisters[] = {
        arm64::x19.code(),
        arm64::x20.code(),
        arm64::x21.code(),
        arm64::x22.code(),
        arm64::x23.code(),
        arm64::x24.code(),
        arm64::x25.code(),
        arm64::x26.code(),
        arm64::x27.code(),
        arm64::x28.code(),
    };
    
    struct Initializer {
        static RegistersConfiguration *New(void *chunk) {
            return new (chunk) RegistersConfiguration(arraysize(kArgumentGPRegisters), // number_of_argument_gp_registers,
                                                      kArgumentGPRegisters, // argument_gp_registers,
                                                      arraysize(kArgumentFPRegisters), // number_of_argument_fp_registers,
                                                      kArgumentFPRegisters, // argument_fp_registers,
                                                      arraysize(kAllocatableGPRegisters), // number_of_allocatable_gp_registers,
                                                      kAllocatableGPRegisters, // allocatable_gp_registers,
                                                      arraysize(kAllocatableFPRegisters), // number_of_allocatable_fp_registers,
                                                      kAllocatableFPRegisters, // allocatable_fp_registers,
                                                      arraysize(kCalleeSaveGPRegisters), // number_of_callee_save_gp_registers,
                                                      kCalleeSaveGPRegisters, // callee_save_gp_registers,
                                                      0, // number_of_callee_save_fp_registers
                                                      nullptr, // callee_save_fp_registers
                                                      32, // max_gp_register,
                                                      32, // max_fp_register,
                                                      arm64::x19.code(), // scratch0,
                                                      arm64::x20.code(), // scratch1,
                                                      arm64::x0.code(), // returning0_register
                                                      arm64::fp.code(), // fp
                                                      arm64::sp.code(), // sp,
                                                      arm64::kRootRegister.code() /*root*/);
        }
        
        static void Delete(void *) {}
    };
    
    static base::LazyInstance<RegistersConfiguration, Initializer> inst;
    return inst.Get();
}

const RegistersConfiguration *RegistersConfiguration::of_x64() {
    //RAX, RCX,
    //RDX, RSI,
    //RDI,
    //R8-R11,
    //ST(0)-ST(7) K0-K7,
    //XMM0-XMM15, YMM0-YMM15 ZMM0-ZMM31
//    static const int kRootRegister = kR14;
//    static const int kScratchGeneralRegister = kR13;

    static const int kAllocatableGPRegisters[] = {
        x64::kRAX,
        x64::kRCX,
        x64::kRDX,
        x64::kRSI,
        x64::kRDI,
        x64::kR8,
        x64::kR9,  // 9
        x64::kR10, // 10
        x64::kR11,
        x64::kR12,
        //kR13, // r13 = scratch
        //kR14, // r14 = root
        x64::kR15,
    };

    static const int kCalleeSaveGPRegisters[] = {
        x64::kRBX,
        x64::kRBP,
        x64::kR12,
        x64::kR13,
        x64::kR14,
        x64::kR15,
    };

    static const int kAllocatableFPRegisters[] = {
        x64::xmm0.code(),
        x64::xmm1.code(),
        x64::xmm2.code(),
        x64::xmm3.code(),
        x64::xmm4.code(),
        x64::xmm5.code(),
        x64::xmm6.code(),
        x64::xmm7.code(),
        x64::xmm8.code(),
        x64::xmm9.code(),
        x64::xmm10.code(),
        x64::xmm11.code(),
        x64::xmm12.code(),
        x64::xmm13.code(),
        x64::xmm14.code(),
        x64::xmm15.code(),
    };

    static const int kArgumentGPRegisters[] = {
        x64::Argv_0.code(),
        x64::Argv_1.code(),
        x64::Argv_2.code(),
        x64::Argv_3.code(),
        x64::Argv_4.code(),
        x64::Argv_5.code(),
        x64::Argv_6.code(),
        x64::Argv_7.code(),
    };

    static const int kArgumentFPRegisters[] = {
        x64::xmm0.code(),
        x64::xmm1.code(),
        x64::xmm2.code(),
        x64::xmm3.code(),
        x64::xmm4.code(),
        x64::xmm5.code(),
        x64::xmm6.code(),
        x64::xmm7.code(),
    };
    
    struct Initializer {
        static RegistersConfiguration *New(void *chunk) {
            return new (chunk) RegistersConfiguration(arraysize(kArgumentGPRegisters), // number_of_argument_gp_registers,
                                                      kArgumentGPRegisters, // argument_gp_registers,
                                                      arraysize(kArgumentFPRegisters), // number_of_argument_fp_registers,
                                                      kArgumentFPRegisters, // argument_fp_registers,
                                                      arraysize(kAllocatableGPRegisters), // number_of_allocatable_gp_registers,
                                                      kAllocatableGPRegisters, // allocatable_gp_registers,
                                                      arraysize(kAllocatableFPRegisters), // number_of_allocatable_fp_registers,
                                                      kAllocatableFPRegisters, // allocatable_fp_registers,
                                                      arraysize(kCalleeSaveGPRegisters), // number_of_callee_save_gp_registers,
                                                      kCalleeSaveGPRegisters, // callee_save_gp_registers,
                                                      0, // number_of_callee_save_fp_registers
                                                      nullptr, // callee_save_fp_registers
                                                      16, // number_of_gp_registers,
                                                      16, // number_of_fp_registers,
                                                      x64::r13.code(), // scratch0,
                                                      -1, // scratch1,
                                                      x64::rax.code(), // returning0_register
                                                      x64::rbp.code(), // fp
                                                      x64::rsp.code(), // sp,
                                                      x64::r14.code() /*root*/);
        }
        
        static void Delete(void *) {}
    };
    
    static base::LazyInstance<RegistersConfiguration, Initializer> inst;
    return inst.Get();
}

} // namespace backend

} // namespace yalx
