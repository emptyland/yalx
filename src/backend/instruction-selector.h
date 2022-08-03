#pragma once
#ifndef YALX_BACKEND_INSTRUCTION_SELECTOR_H_
#define YALX_BACKEND_INSTRUCTION_SELECTOR_H_

#include "backend/instruction-code.h"
#include "backend/instruction.h"
#include "base/arena-utils.h"
#include "base/checking.h"
#include "base/base.h"

namespace yalx {
namespace ir {
class Value;
class Type;
class Function;
} // namespace ir
namespace backend {

class RegistersConfiguration;
class InstructionOperand;
class Instruction;
class Frame;

class InstructionSelector {
public:
    InstructionSelector(const RegistersConfiguration *regconf, base::Arena *arena);
    
    void VisitParameters(ir::Function *fun);
    void VisitCall(ir::Value *value);

    Instruction *Emit(InstructionCode opcode, InstructionOperand output,
                      int temps_count = 0, InstructionOperand *temps = nullptr);
    Instruction *Emit(InstructionCode opcode, InstructionOperand output,
                      InstructionOperand input0,
                      int temps_count = 0, InstructionOperand *temps = nullptr);
    Instruction *Emit(InstructionCode opcode, InstructionOperand output,
                      InstructionOperand input0, InstructionOperand input1,
                      int temps_count = 0, InstructionOperand *temps = nullptr);
    Instruction *Emit(InstructionCode opcode, InstructionOperand output,
                      InstructionOperand input0, InstructionOperand input1, InstructionOperand input2,
                      int temps_count = 0, InstructionOperand *temps = nullptr);
    Instruction *Emit(InstructionCode opcode, InstructionOperand output,
                      InstructionOperand input0, InstructionOperand input1, InstructionOperand input2,
                      InstructionOperand input3, int temps_count = 0, InstructionOperand *temps = nullptr);
    Instruction *Emit(InstructionCode opcode, InstructionOperand output,
                      InstructionOperand input0, InstructionOperand input1, InstructionOperand input2,
                      InstructionOperand input3, InstructionOperand input4,
                      int temps_count = 0, InstructionOperand *temps = nullptr);
    
    Instruction *Emit(InstructionCode opcode,
                      int outputs_count, InstructionOperand *outputs,
                      int inputs_count, InstructionOperand *inputs,
                      int temps_count, InstructionOperand *temps);
    
    Instruction *Emit(Instruction *instr) {
        instructions_.push_back(instr);
        return instr;
    }
    
    UnallocatedOperand DefineFixedRegister(ir::Value *value, int index);
    UnallocatedOperand DefineFixedFPRegister(ir::Value *value, int index);
    UnallocatedOperand DefineFixedSlot(ir::Value *value, int index);
    
    static InstructionOperand Invalid() { return InstructionOperand(); }
    
//    UnallocatedOperand Define(ir::Value *value, UnallocatedOperand operand);
//    UnallocatedOperand Use(ir::Value *value, UnallocatedOperand operand);
    
    int GetVirtualRegister(ir::Value *value);
private:
    base::Arena *const arena_;
    const RegistersConfiguration *const regconf_;
    base::ArenaVector<Instruction *> instructions_;
    Frame *frame_ = nullptr;
}; // class InstructionSelector

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_INSTRUCTION_SELECTOR_H_
