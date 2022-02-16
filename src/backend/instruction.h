#pragma once
#ifndef YALX_BACKEND_INSTRUCTION_H_
#define YALX_BACKEND_INSTRUCTION_H_

#include "backend/x64/instruction-codes-x64.h"
#include "backend/arm64/instruction-codes-arm64.h"
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
    ArchJmp,
    ArchCall,
    ArchUnreachable,
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

class UnallocatedOperand;
class ConstantOperand;
class ImmediateOperand;
class ReloactionOperand;
class LocationOperand;
class RegisterOperand;

class InstructionBlock;

#define DECLARE_INSTRUCTION_OPERANDS_KINDS(V) \
    V(Constant) \
    V(Immediate) \
    V(Reloaction) \
    V(Location) \
    V(Register)

class InstructionOperand : public base::ArenaObject {
public:
    enum Kind {
        kInvalid,
#define DEFINE_ENUM(name) k##name,
    DECLARE_INSTRUCTION_OPERANDS_KINDS(DEFINE_ENUM)
#undef  DEFINE_ENUM
    };
    
    DEF_VAL_GETTER(Kind, kind);
    
    void Kill() { kind_ = kInvalid; }

#define DEFINE_TESTING(name) bool Is##name() const { return kind() == k##name; }
    DECLARE_INSTRUCTION_OPERANDS_KINDS(DEFINE_TESTING)
#undef  DEFINE_TESTING
    bool IsInvalid() const { return kind() == kInvalid; }
    
    bool Equals(const InstructionOperand *other) const;
    
#define DEFINE_CASTING(name) \
    inline name##Operand *As##name(); \
    inline const name##Operand *As##name() const;
    DECLARE_INSTRUCTION_OPERANDS_KINDS(DEFINE_CASTING)
#undef  DEFINE_CASTING
protected:
    explicit InstructionOperand(Kind kind): kind_(kind) {}

    Kind kind_;
}; // class InstructionOperand

class ImmediateOperand final : public InstructionOperand {
public:
    DEF_VAL_GETTER(MachineRepresentation, rep);
    DEF_VAL_GETTER(int8_t, word8);
    DEF_VAL_GETTER(int16_t, word16);
    DEF_VAL_GETTER(int32_t, word32);
    DEF_VAL_GETTER(int64_t, word64);
    
    static ImmediateOperand *Word8(base::Arena *arena, int8_t val) {
        auto imm = new (arena) ImmediateOperand(MachineRepresentation::kWord8);
        imm->word8_ = val;
        return imm;
    }
    
    static ImmediateOperand *Word16(base::Arena *arena, int16_t val) {
        auto imm = new (arena) ImmediateOperand(MachineRepresentation::kWord16);
        imm->word16_ = val;
        return imm;
    }
    
    static ImmediateOperand *Word32(base::Arena *arena, int32_t val) {
        auto imm = new (arena) ImmediateOperand(MachineRepresentation::kWord32);
        imm->word32_ = val;
        return imm;
    }
    
    static ImmediateOperand *Word64(base::Arena *arena, int64_t val) {
        auto imm = new (arena) ImmediateOperand(MachineRepresentation::kWord64);
        imm->word64_ = val;
        return imm;
    }
    
    void Set8(int8_t val) {
        assert(rep_ == MachineRepresentation::kWord8);
        word8_ = val;
    }
    
    void Set16(int16_t val) {
        assert(rep_ == MachineRepresentation::kWord16);
        word16_ = val;
    }
    
    void Set32(int32_t val) {
        assert(rep_ == MachineRepresentation::kWord32);
        word32_ = val;
    }
    
    void Set64(int64_t val) {
        assert(rep_ == MachineRepresentation::kWord64);
        word64_ = val;
    }

private:
    ImmediateOperand(MachineRepresentation rep)
    : InstructionOperand(kImmediate)
    , rep_(rep) {}

    const MachineRepresentation rep_;
    union {
        int8_t word8_;
        int16_t word16_;
        int32_t word32_;
        int64_t word64_;
    };
}; // class ImmediateOperand

class ConstantOperand final : public InstructionOperand {
public:
    enum Type {
        kString,
        kNumber,
    };
    
    ConstantOperand(Type type, int symbol_id): InstructionOperand(kConstant), type_(type), symbol_id_(symbol_id) {}
    
    DEF_VAL_GETTER(Type, type);
    DEF_VAL_GETTER(int, symbol_id);

private:
    Type type_;
    int symbol_id_;
}; // class ConstantOperand

class ReloactionOperand final : public InstructionOperand {
public:
    ReloactionOperand(const String *symbol_name, const InstructionBlock *label)
    : InstructionOperand(kReloaction)
    , label_(label)
    , symbol_name_(symbol_name) {}
    
    DEF_PTR_GETTER(const String, symbol_name);
    DEF_PTR_GETTER(const InstructionBlock, label);
private:
    const InstructionBlock *label_;
    const String *symbol_name_;
}; // class ReloactionOperand

class LocationOperand final : public InstructionOperand {
public:
    LocationOperand(AddressingMode mode, int register0_id, int register1_id, int k)
    : InstructionOperand(kLocation)
    , mode_(mode)
    , register0_id_(register0_id)
    , register1_id_(register1_id)
    , k_(k) {
    }
    
    DEF_VAL_GETTER(AddressingMode, mode);
    DEF_VAL_GETTER(int, register0_id);
    DEF_VAL_GETTER(int, register1_id);
    DEF_VAL_GETTER(int, k);

private:
    AddressingMode mode_;
    int register0_id_;
    int register1_id_;
    int k_;
}; // class LocationOperand

class RegisterOperand final : public InstructionOperand {
public:
    RegisterOperand(int register_id, MachineRepresentation rep)
    : InstructionOperand(kRegister)
    , register_id_(register_id)
    , rep_(rep) {}
    
    DEF_VAL_GETTER(int, register_id);
    DEF_VAL_GETTER(MachineRepresentation, rep);
    
    bool IsGeneralRegister() const;
    bool IsFloatRegister() const { return rep() == MachineRepresentation::kFloat32; }
    bool IsDoubleRegister() const { return rep() == MachineRepresentation::kFloat64; }
    
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
    DEF_VAL_GETTER(int, inputs_count);
    DEF_VAL_GETTER(int, outputs_count);
    DEF_VAL_GETTER(int, temps_count);
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
    
    constexpr int input_offset() const { return 0; }
    int output_offset() const { return input_offset() + inputs_count_; }
    int temp_offset() const { return output_offset() + outputs_count_; }
    
    static Instruction *New(base::Arena *arena, Code op, Operand *operands[] = nullptr,
                            size_t inputs_count = 0,
                            size_t outputs_count = 0,
                            size_t temps_count = 0);
    
    static void *AllocatePlacementMemory(base::Arena *arena,
                                         size_t inputs_count = 0,
                                         size_t outputs_count = 0,
                                         size_t temps_count = 0);
    
    Code op_;
    uint8_t inputs_count_;
    uint8_t outputs_count_;
    uint8_t temps_count_;
    uint8_t is_call_;
    Operand *operands_[1];
}; // class Instruction


class InstructionFunction final : public base::ArenaObject {
public:
    using SymbolMap = base::ArenaUnorderedMap<std::string_view, const String *>;
    
    InstructionFunction(base::Arena *arena, const String *symbol);
    
    DEF_PTR_GETTER(const String, symbol);
    DEF_VAL_GETTER(SymbolMap, external_symbols);
    DEF_ARENA_VECTOR_GETTER(InstructionBlock *, block);
    
    inline InstructionBlock *NewBlock(int label);
    
    void AddExternalSymbol(std::string_view name, const String *symbol) {
        external_symbols_[name] = symbol;
    }
private:
    const String *const symbol_;
    base::Arena *const arena_;
    SymbolMap external_symbols_;
    base::ArenaVector<InstructionBlock *> blocks_;
}; // class InstructionFunction

class InstructionBlock final : public base::ArenaObject {
public:
    explicit InstructionBlock(base::Arena *arena, InstructionFunction *owns, int label);

    Instruction *New(Instruction::Code op);
    Instruction *NewI(Instruction::Code op, Instruction::Operand *input);
    Instruction *NewO(Instruction::Code op, Instruction::Operand *output);
    Instruction *NewIO(Instruction::Code op, Instruction::Operand *io, Instruction::Operand *input);
    Instruction *NewIO(Instruction::Code op, Instruction::Operand *output, Instruction::Operand *in1,
                       Instruction::Operand *in2);

    DEF_PTR_GETTER(InstructionFunction, owns);
    int label() const { return label_; }
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
    InstructionFunction *owns_;
    base::ArenaVector<InstructionBlock *> successors_;
    base::ArenaVector<InstructionBlock *> predecessors_;
    base::ArenaVector<Instruction *> instructions_;
    int label_;
}; // class InstructionBlock

class InstructionBlockLabelGenerator final {
public:
    InstructionBlockLabelGenerator() = default;
    
    int NextLable() { return next_label_id_++; }
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(InstructionBlockLabelGenerator);
private:
    int next_label_id_ = 0;
}; // class InstructionBlockLabelGenerator

#define DEFINE_CASTING(name) \
inline name##Operand *InstructionOperand::As##name() { \
    return !Is##name() ? nullptr : static_cast<name##Operand *>(this); \
} \
inline const name##Operand *InstructionOperand::As##name() const { \
    return !Is##name() ? nullptr : static_cast<const name##Operand *>(this); \
}
DECLARE_INSTRUCTION_OPERANDS_KINDS(DEFINE_CASTING)
#undef  DEFINE_CASTING

inline InstructionBlock *InstructionFunction::NewBlock(int label) {
    auto block = new (arena_) InstructionBlock(arena_, this, label);
    blocks_.push_back(block);
    return block;
}




} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_INSTRUCTION_H_
