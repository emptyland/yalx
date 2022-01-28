#pragma once
#ifndef YALX_BACKEND_INSTRUCTION_H_
#define YALX_BACKEND_INSTRUCTION_H_

#include "backend/machine-type.h"
#include "base/arena-utils.h"
#include "base/base.h"

namespace yalx {
namespace base {
class ArenaString;
} // namespace base
namespace backend {

using String = base::ArenaString;

enum InstructionCode {
    ArchNop,
    ArchNoreachable,
    ArchBreak,
    // TODO:
};

class UnallocatedOperand;
class ConstantOperand;
class ImmediateOperand;
class ReloactionOperand;
class LocationOperand;
class RegisterOperand;

class InstructionOperand final {
public:
    enum Kind {
        kInvalid,
        kUnallocated,
        kConstant,
        kImmediate,
        kReloaction,
        kLocation,
        kRegister,
    };
    
    DEF_VAL_GETTER(Kind, kind);
    
protected:
    explicit InstructionOperand(Kind kind): kind_(kind) {}

    Kind kind_;
}; // // class InstructionOperand


class Instruction final {
public:
    using Code = InstructionCode;
    using Operand = InstructionOperand;
    
    DEF_VAL_GETTER(Code, op);
    DEF_VAL_GETTER(size_t, inputs_count);
    DEF_VAL_GETTER(size_t, outputs_count);
    DEF_VAL_GETTER(size_t, temps_count);
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Instruction);
private:
    Instruction(Code op,
                Operand *inputs[], size_t inputs_count,
                Operand *outputs[], size_t outputs_count,
                Operand *temps[], size_t temps_count);
    
    constexpr size_t input_offset() const { return 0; }
    size_t output_offset() const { input_offset() + inputs_count(); }
    size_t temp_offset() const { output_offset() + outputs_count(); }
    
    Code op_;
    uint32_t inputs_count_  : 4;
    uint32_t outputs_count_ : 4;
    uint32_t temps_count_   : 4;
    uint32_t is_call_       : 1;
    Operand operands_[1];
}; // class Instruction


} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_INSTRUCTION_H_
