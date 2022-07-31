#pragma once
#ifndef YALX_BACKEND_INSTRUCTION_SELECTOR_H_
#define YALX_BACKEND_INSTRUCTION_SELECTOR_H_

#include "backend/instruction-code.h"
#include "base/arena-utils.h"
#include "base/checking.h"
#include "base/base.h"

namespace yalx {

namespace backend {

class InstructionOperand;
class Instruction;

class InstructionSelector {
public:
    InstructionSelector(base::Arena *arena);
    

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
    
private:
    base::Arena *const arena_;
    base::ArenaVector<Instruction *> instructions_;
}; // class InstructionSelector

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_INSTRUCTION_SELECTOR_H_
