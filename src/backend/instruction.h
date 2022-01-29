#pragma once
#ifndef YALX_BACKEND_INSTRUCTION_H_
#define YALX_BACKEND_INSTRUCTION_H_

#include "backend/x64/instruction-codes-x64.h"
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
    ArchDebugBreak,
    ArchRet,
    ArchUnreachable,
#define DEFINE_ENUM(name) name,
    X64_ARCH_OPCODE_LIST(DEFINE_ENUM)
#undef DEFINE_ENUM
};

class UnallocatedOperand;
class ConstantOperand;
class ImmediateOperand;
class ReloactionOperand;
class LocationOperand;
class RegisterOperand;

class InstructionOperand : public base::ArenaObject {
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
}; // class InstructionOperand


class RegisterOperand final : public InstructionOperand {
public:
    RegisterOperand(int register_id, MachineRepresentation rep)
    : InstructionOperand(kRegister)
    , register_id_(register_id)
    , rep_(rep) {}
    
    DEF_VAL_GETTER(int, register_id);
    DEF_VAL_GETTER(MachineRepresentation, rep);
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(RegisterOperand);
private:
    int register_id_;
    MachineRepresentation rep_;
}; // class RegisterOperand

class Instruction final {
public:
    using Code = InstructionCode;
    using Operand = InstructionOperand;
    
    DEF_VAL_GETTER(Code, op);
    DEF_VAL_GETTER(size_t, inputs_count);
    DEF_VAL_GETTER(size_t, outputs_count);
    DEF_VAL_GETTER(size_t, temps_count);
    size_t operands_size() const { return inputs_count() + outputs_count() + temps_count(); }
    
    Operand *InputAt(size_t i) const {
        assert(i < inputs_count());
        return operands_[input_offset() + i];
    }
    
    Operand *OutputAt(size_t i) const {
        assert(i < outputs_count());
        return operands_[output_offset() + i];
    }
    
    friend class InstructionBlock;
    DISALLOW_IMPLICIT_CONSTRUCTORS(Instruction);
private:
    Instruction(Code op, size_t inputs_count, size_t outputs_count, size_t temps_count, Operand *operands[]);
    
    constexpr size_t input_offset() const { return 0; }
    size_t output_offset() const { input_offset() + inputs_count(); }
    size_t temp_offset() const { output_offset() + outputs_count(); }
    
    static Instruction *New(base::Arena *arena, Code op, Operand *operands[] = nullptr,
                            size_t inputs_count = 0,
                            size_t outputs_count = 0,
                            size_t temps_count = 0);
    
    static void *AllocatePlacementMemory(base::Arena *arena,
                                         size_t inputs_count = 0,
                                         size_t outputs_count = 0,
                                         size_t temps_count = 0);
    
    Code op_;
    uint32_t inputs_count_  : 4;
    uint32_t outputs_count_ : 4;
    uint32_t temps_count_   : 4;
    uint32_t is_call_       : 1;
    Operand *operands_[1];
}; // class Instruction


class InstructionBlock final : public base::ArenaObject {
public:
    explicit InstructionBlock(base::Arena *arena);

    Instruction *New(Instruction::Code op);
    Instruction *NewI(Instruction::Code op, Instruction::Operand *input);
    Instruction *NewIO(Instruction::Code op, Instruction::Operand *io, Instruction::Operand *input);
    Instruction *NewIO(Instruction::Code op, Instruction::Operand *output, Instruction::Operand *in1,
                       Instruction::Operand *in2);

    DEF_ARENA_VECTOR_GETTER(InstructionBlock *, successor);
    DEF_ARENA_VECTOR_GETTER(InstructionBlock *, predecessor);
    DEF_ARENA_VECTOR_GETTER(Instruction *, instruction);

    void AddSuccessor(InstructionBlock *successor) { AddLinkedNode(&successors_, successor); }
    void AddPredecessors(InstructionBlock *predecessor) { AddLinkedNode(&predecessors_, predecessor); }

    DISALLOW_IMPLICIT_CONSTRUCTORS(InstructionBlock);
private:
    void AddLinkedNode(base::ArenaVector<InstructionBlock *> *nodes, InstructionBlock *node) {
        if (auto iter = std::find(nodes->begin(), nodes->end(), node); iter == nodes->end()) {
            nodes->push_back(node);
        }
    }

    base::Arena *const arena_;
    base::ArenaVector<InstructionBlock *> successors_;
    base::ArenaVector<InstructionBlock *> predecessors_;
    base::ArenaVector<Instruction *> instructions_;
}; // class InstructionBlock

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_INSTRUCTION_H_
