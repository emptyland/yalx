#include "backend/instruction.h"
#include "base/checking.h"

namespace yalx {
namespace backend {


bool RegisterOperand::IsGeneralRegister() const {
    switch (rep()) {
        case MachineRepresentation::kWord8:
        case MachineRepresentation::kWord16:
        case MachineRepresentation::kWord32:
        case MachineRepresentation::kWord64:
            return true;
            
        default:
            return false;
    }
}

bool InstructionOperand::Equals(const InstructionOperand *other) const {
    if (kind() != other->kind()) {
        return false;
    }
    switch (kind()) {
        case kInvalid:
            return other->IsInvalid();
        case kConstant: {
            auto lhs = AsConstant();
            auto rhs = other->AsConstant();
            return lhs->type() == rhs->type() && lhs->symbol_id() == rhs->symbol_id();
        } break;
        case kLocation: {
            auto lhs = AsLocation();
            auto rhs = other->AsLocation();
            return lhs->mode() == rhs->mode() &&
                lhs->register0_id() == rhs->register0_id() &&
                lhs->register1_id() == rhs->register1_id() &&
                lhs->k() == rhs->k();
        } break;
        case kRegister: {
            auto lhs = AsRegister();
            auto rhs = other->AsRegister();
            return lhs->rep() == rhs->rep() && lhs->register_id() == rhs->register_id();
        } break;
        case kImmediate: {
            auto lhs = AsImmediate();
            auto rhs = other->AsImmediate();
            if (lhs->rep() != rhs->rep()) {
                return false;
            }
            switch (lhs->rep()) {
                case MachineRepresentation::kWord8:
                    return lhs->word8() == rhs->word8();
                case MachineRepresentation::kWord16:
                    return lhs->word16() == rhs->word16();
                case MachineRepresentation::kWord32:
                    return lhs->word32() == rhs->word32();
                case MachineRepresentation::kWord64:
                    return lhs->word64() == rhs->word64();
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
        case kReloaction: {
            auto lhs = AsReloaction();
            auto rhs = other->AsReloaction();
            return lhs->label() == rhs->label() && lhs->symbol_name() == rhs->symbol_name() &&
                lhs->fetch_address() == rhs->fetch_address();
        } break;
        default:
            UNREACHABLE();
            break;
    }
}

ReloactionOperand *ReloactionOperand::OffsetOf(base::Arena *arena, int offset) const {
    ReloactionOperand *copied = new (arena) ReloactionOperand(symbol_name(), label(), fetch_address());
    copied->offset_ = offset;
    return copied;
}

Instruction::Instruction(Code op, size_t inputs_count, size_t outputs_count, size_t temps_count, Operand *operands[])
: op_(op)
, inputs_count_(inputs_count)
, outputs_count_(outputs_count)
, temps_count_(temps_count) {
    ::memcpy(operands_, operands, operands_size() * sizeof(Operand *));
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

InstructionFunction::InstructionFunction(base::Arena *arena, const String *symbol)
: arena_(arena)
, symbol_(symbol)
, external_symbols_(arena)
, blocks_(arena) {
    
}

InstructionBlock::InstructionBlock(base::Arena *arena, InstructionFunction *owns, int label)
: arena_(arena)
, owns_(owns)
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

Instruction *InstructionBlock::NewO(Instruction::Code op, Instruction::Operand *output) {
    auto instr = Instruction::New(arena_, op, &output, 0/*inputs_count*/, 1/*outputs_count*/);
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

Instruction *InstructionBlock::NewIO2(Instruction::Code op, Instruction::Operand *out1, Instruction::Operand *out2,
                                      Instruction::Operand *input) {
    InstructionOperand *operands[] = {input, out1, out2};
    auto instr = Instruction::New(arena_, op, operands, 1/*inputs_count*/, 2/*outputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

Instruction *InstructionBlock::NewI2O(Instruction::Code op, Instruction::Operand *io, Instruction::Operand *in1,
                                      Instruction::Operand *in2) {
    InstructionOperand *operands[] = {in1, in2, io};
    auto instr = Instruction::New(arena_, op, operands, 2/*inputs_count*/, 1/*outputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

Instruction *InstructionBlock::NewII(Instruction::Code op, Instruction::Operand *in1, Instruction::Operand *in2) {
    InstructionOperand *operands[] = {in1, in2};
    auto instr = Instruction::New(arena_, op, operands, 2/*inputs_count*/, 0/*outputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

} // namespace backend
} // namespace yalx
