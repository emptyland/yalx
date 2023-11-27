#pragma once
#ifndef YALX_BACKEND_INSTRUCTION_CODE_H_
#define YALX_BACKEND_INSTRUCTION_CODE_H_

#include "backend/x64/instruction-codes-x64.h"
#include "backend/arm64/instruction-codes-arm64.h"
#include "base/bit-field.h"

namespace yalx::backend {

#define ARCH_OPCODE_LIST(V) \
    V(ArchNop) \
    V(ArchDebugBreak) \
    V(ArchRet) \
    V(ArchJmp) \
    V(ArchCall) \
    V(ArchCallNative) \
    V(ArchAfterCall) \
    V(ArchBeforeCall) \
    V(ArchFrameEnter) \
    V(ArchFrameExit) \
    V(ArchUnreachable) \
    V(ArchSafepoint) \
    V(ArchStackAlloc) \
    V(ArchLoadRelocation) \
    V(ArchLoadEffectAddress)

enum InstructionCode {
#define DEFINE_ENUM(name) name,
    ARCH_OPCODE_LIST(DEFINE_ENUM)
    X64_ARCH_OPCODE_LIST(DEFINE_ENUM)
    ARM64_ARCH_OPCODE_LIST(DEFINE_ENUM)
#undef DEFINE_ENUM
    kMaxInstructionCodes,
};

extern const char *const kInstrCodeNames[];

inline InstructionCode AndBits(InstructionCode op, uint32_t bits) {
    return static_cast<InstructionCode>(static_cast<uint32_t>(op) | bits);
}

enum AddressingMode {
#define DEFINE_ENUM(name) X64Mode_##name,
    X64_ADDRESSING_MODE_LIST(DEFINE_ENUM)
#undef DEFINE_ENUM
    
#define DEFINE_ENUM(name) Arm64Mode_##name,
    ARM64_ADDRESSING_MODE_LIST(DEFINE_ENUM)
#undef DEFINE_ENUM
    kMaxAddressingMode,
};

enum CallDescriptor {
    kNotCall,
    kCallNative,
    kCallDirectly,
    kMaxCalling,
};

using InstructionCodeField = base::BitField<InstructionCode, 0, 16>;
using AddressingModeField  = InstructionCodeField::Next<AddressingMode, 8>;
using CallDescriptorField  = InstructionCodeField::Next<CallDescriptor, 4>;

} // namespace yalx

#endif // YALX_BACKEND_INSTRUCTION_CODE_H_
