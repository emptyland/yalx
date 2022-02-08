#include "backend/instruction.h"

namespace yalx {

namespace backend {

Instruction::Instruction(Code op, size_t inputs_count, size_t outputs_count, size_t temps_count, Operand *operands[])
: op_(op)
, inputs_count_(inputs_count)
, outputs_count_(outputs_count)
, temps_count_(temps_count) {
    ::memcpy(operands_, operands, operands_size() * sizeof(operands_[0]));
}

Instruction *Instruction::New(base::Arena *arena, Code op, Operand *operands[], size_t inputs_count,
                              size_t outputs_count, size_t temps_count) {
    auto chunk = AllocatePlacementMemory(arena, inputs_count, outputs_count, temps_count);
    return new (chunk) Instruction(op, inputs_count, outputs_count, temps_count, operands);
}

void *Instruction::AllocatePlacementMemory(base::Arena *arena, size_t inputs_count, size_t outputs_count,
                                           size_t temps_count) {
    size_t in_memory_bytes = sizeof(Instruction) + (inputs_count + outputs_count + temps_count) * sizeof(Operand *);
    return arena->Allocate(in_memory_bytes);
}

InstructionBlock::InstructionBlock(base::Arena *arena, int label)
: arena_(arena)
, successors_(arena)
, predecessors_(arena)
, instructions_(arena)
, label_(label) {
    
}

Instruction *InstructionBlock::New(Instruction::Code op) {
    auto instr = Instruction::New(arena_, op);
    instructions_.push_back(instr);
    return instr;
}

Instruction *InstructionBlock::NewI(Instruction::Code op, Instruction::Operand *input) {
    auto instr = Instruction::New(arena_, op, &input, 1/*inputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

Instruction *InstructionBlock::NewIO(Instruction::Code op, Instruction::Operand *io, Instruction::Operand *input) {
    InstructionOperand *operands[2] = {input, io};
    auto instr = Instruction::New(arena_, op, operands, 1/*inputs_count*/, 1/*outputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

Instruction *InstructionBlock::NewIO(Instruction::Code op, Instruction::Operand *output, Instruction::Operand *in1,
                                     Instruction::Operand *in2) {
    InstructionOperand *operands[] = {in1, in2, output};
    auto instr = Instruction::New(arena_, op, operands, 2/*inputs_count*/, 1/*outputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

} // namespace backend

} // namespace yalx
