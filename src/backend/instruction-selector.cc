#include "backend/instruction-selector.h"
#include "backend/instruction.h"

namespace yalx {

namespace backend {

InstructionSelector::InstructionSelector(base::Arena *arena)
: arena_(arena)
, instructions_(arena) {
    
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, InstructionOperand output,
                                       int temps_count, InstructionOperand *temps) {
    int outputs_count = output.IsInvalid() ? 0 : 1;
    return Emit(opcode, outputs_count, &output, 0, nullptr, temps_count, temps);
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, InstructionOperand output,
                                       InstructionOperand input0, int temps_count, InstructionOperand *temps) {
    int outputs_count = output.IsInvalid() ? 0 : 1;
    InstructionOperand inputs[] = {input0};
    int inputs_count = arraysize(inputs);
    return Emit(opcode, outputs_count, &output, inputs_count, inputs, temps_count, temps);
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, InstructionOperand output, InstructionOperand input0,
                                       InstructionOperand input1, int temps_count, InstructionOperand *temps) {
    int outputs_count = output.IsInvalid() ? 0 : 1;
    InstructionOperand inputs[] = {input0, input1};
    int inputs_count = arraysize(inputs);
    return Emit(opcode, outputs_count, &output, inputs_count, inputs, temps_count, temps);
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, InstructionOperand output, InstructionOperand input0,
                                       InstructionOperand input1, InstructionOperand input2, int temps_count,
                                       InstructionOperand *temps) {
    int outputs_count = output.IsInvalid() ? 0 : 1;
    InstructionOperand inputs[] = {input0, input1, input2};
    int inputs_count = arraysize(inputs);
    return Emit(opcode, outputs_count, &output, inputs_count, inputs, temps_count, temps);
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, InstructionOperand output, InstructionOperand input0,
                                       InstructionOperand input1, InstructionOperand input2, InstructionOperand input3,
                                       int temps_count, InstructionOperand *temps) {
    int outputs_count = output.IsInvalid() ? 0 : 1;
    InstructionOperand inputs[] = {input0, input1, input2, input3};
    int inputs_count = arraysize(inputs);
    return Emit(opcode, outputs_count, &output, inputs_count, inputs, temps_count, temps);
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, InstructionOperand output, InstructionOperand input0,
                                       InstructionOperand input1, InstructionOperand input2, InstructionOperand input3,
                                       InstructionOperand input4, int temps_count, InstructionOperand *temps) {
    int outputs_count = output.IsInvalid() ? 0 : 1;
    InstructionOperand inputs[] = {input0, input1, input2, input3, input4};
    int inputs_count = arraysize(inputs);
    return Emit(opcode, outputs_count, &output, inputs_count, inputs, temps_count, temps);
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, int outputs_count, InstructionOperand *outputs,
                                       int inputs_count, InstructionOperand *inputs, int temps_count,
                                       InstructionOperand *temps) {
    auto instr = Instruction::New(arena_, opcode, inputs_count, inputs, outputs_count, outputs, temps_count, temps);
    return Emit(instr);
}


} // namespace backend

} // namespace yalx
