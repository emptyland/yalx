#pragma once
#ifndef YALX_BACKEND_INSTRUCTION_CODE_H_
#define YALX_BACKEND_INSTRUCTION_CODE_H_

#include "backend/x64/instruction-codes-x64.h"
#include "backend/arm64/instruction-codes-arm64.h"

namespace yalx {

namespace backend {

enum InstructionCode {
    ArchNop,
    ArchDebugBreak,
    ArchRet,
    ArchJmp,
    ArchCall,
    ArchSaveCallerRegisters,
    ArchRestoreCallerRegisters,
    ArchFrameEnter,
    ArchFrameExit,
    ArchUnreachable,
    ArchSafepoint,
    ArchStackAlloc,
    ArchLoadEffectAddress,
#define DEFINE_ENUM(name) name,
    X64_ARCH_OPCODE_LIST(DEFINE_ENUM)
    ARM64_ARCH_OPCODE_LIST(DEFINE_ENUM)
#undef DEFINE_ENUM
};

enum AddressingMode {
#define DEFINE_ENUM(name) X64Mode_##name,
    X64_ADDRESSING_MODE_LIST(DEFINE_ENUM)
#undef DEFINE_ENUM
    
#define DEFINE_ENUM(name) Arm64Mode_##name,
    ARM64_ADDRESSING_MODE_LIST(DEFINE_ENUM)
#undef DEFINE_ENUM
};

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_INSTRUCTION_CODE_H_
