#pragma once
#ifndef YALX_BACKEND_INSTRUCTION_H_
#define YALX_BACKEND_INSTRUCTION_H_

#include "backend/instruction-code.h"
#include "backend/machine-type.h"
#include "base/arena-utils.h"
#include "base/checking.h"
#include "base/base.h"

namespace yalx {
namespace ir {
class ArrayModel;
} // namespace ir
namespace base {
class ArenaString;
} // namespace base
namespace backend {

using String = base::ArenaString;

class UnallocatedOperand;
class AllocatedOperand;
class ConstantOperand;
class ImmediateOperand;
class ReloactionOperand;

class InstructionSelector;
class InstructionBlock;

#define DECLARE_INSTRUCTION_OPERANDS_KINDS(V) \
    V(Unallocated) \
    V(Constant)    \
    V(Immediate)   \
    V(Reloaction)  \
    V(Allocated)

class InstructionOperand : public base::ArenaObject {
public:
    enum Kind {
        kInvalid,
#define DEFINE_ENUM(name) k##name,
    DECLARE_INSTRUCTION_OPERANDS_KINDS(DEFINE_ENUM)
#undef  DEFINE_ENUM
    };
    
    InstructionOperand(): InstructionOperand(kInvalid) {}
    
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
    
    //DISALLOW_IMPLICIT_CONSTRUCTORS(InstructionOperand);
protected:
    struct UnallocatedBundle {
        int16_t policy;
        int16_t life_time;
        int value;
        int vid; // virtual register id
    };
    struct ReloactionBundle {
        union {
            const InstructionBlock *label;
            const String *symbol_name;
        };
        int32_t fetch_address: 1;
        int32_t label_or_symbol: 1;
        int32_t offset: 30;
    };
    struct ImmediateBundle {
        MachineRepresentation rep;
        union {
            int8_t  word8_value;
            int16_t word16_value;
            int32_t word32_value;
            int64_t word64_value;
            float   float32_value;
            double  float64_value;
        };
    };
    struct ConstantBundle {
        int type;
        int symbol_id;
    };
    struct AllocatedBundle {
        int location_kind;
        int rep;
        int index;
    };
    
    explicit InstructionOperand(Kind kind): kind_(kind) {}

    Kind kind_;
    union {
        UnallocatedBundle unallocated_values_;
        ReloactionBundle  reloaction_values_;
        ImmediateBundle   immediate_values_;
        ConstantBundle    constant_values_;
        AllocatedBundle   allocated_values_;
    };

}; // class InstructionOperand

class UnallocatedOperand final : public InstructionOperand {
public:
    enum Policy {
        kNone,
        kRegisterOrSlot,
        kRegisterOrSlotOrConstant,
        kFixedRegister,
        kFixedFPRegister,
        kFixedSlot,
        kMustHaveRegister,
        kMustHaveSlot,
        kSameAsInput,
    };
    enum LifeTime {
        kUsedAtStart,
        kUsedAtEnd,
    };
    
    static constexpr struct FixedSlotTag{} FIXED_SLOT;
    
    UnallocatedOperand(Policy policy, int virtual_register)
    : UnallocatedOperand(virtual_register) {
        mutable_bundle()->policy = policy;
        mutable_bundle()->life_time = kUsedAtEnd;
        mutable_bundle()->value = 0;
    }
    
    // Same as input
    UnallocatedOperand(int input_index, int virtual_register)
    : UnallocatedOperand(virtual_register) {
        mutable_bundle()->policy = kSameAsInput;
        mutable_bundle()->life_time = kUsedAtEnd;
        mutable_bundle()->value = input_index;
    }
    
    UnallocatedOperand(FixedSlotTag, int offset, int virtual_register)
    : UnallocatedOperand(virtual_register) {
        mutable_bundle()->policy = kFixedSlot;
        mutable_bundle()->life_time = kUsedAtEnd;
        mutable_bundle()->value = offset;
    }
    
    UnallocatedOperand(Policy policy, int index, int virtual_register)
    : UnallocatedOperand(virtual_register) {
        DCHECK(policy == kFixedRegister || policy == kFixedFPRegister);
        mutable_bundle()->policy = policy;
        mutable_bundle()->life_time = kUsedAtEnd;
        mutable_bundle()->value = index;
    }
    
    UnallocatedOperand(Policy policy, LifeTime life_time, int virtual_register)
    : UnallocatedOperand(virtual_register) {
        mutable_bundle()->policy = policy;
        mutable_bundle()->life_time = life_time;
        mutable_bundle()->value = 0;
    }
    
    UnallocatedOperand(const UnallocatedOperand &other, int virtual_register)
    : UnallocatedOperand(virtual_register) {
        mutable_bundle()->policy = other.bundle()->policy;
        mutable_bundle()->life_time = other.bundle()->life_time;
        mutable_bundle()->value = other.bundle()->value;
    }
    
    Policy policy() const { return static_cast<Policy>(bundle()->policy); }
    LifeTime life_time() const { return static_cast<LifeTime>(bundle()->life_time); }
    int virtual_register() const { return bundle()->vid; }

    bool has_reigster_or_slot_policy() const { return policy() == kRegisterOrSlot; }
    bool has_reigster_or_slot_or_constant_policy() const { return policy() == kRegisterOrSlotOrConstant; }
    bool has_fixed_reigster_policy() const { return policy() == kFixedRegister; }
    bool has_fixed_fp_reigster_policy() const { return policy() == kFixedFPRegister; }
    bool has_fixed_slot_policy() const { return policy() == kFixedSlot; }
    bool has_must_have_reigster_policy() const { return policy() == kMustHaveRegister; }
    bool has_must_have_slot_policy() const { return policy() == kMustHaveSlot; }
    bool has_same_as_input_policy() const { return policy() == kSameAsInput; }
    
    int fixed_slot_offset() const {
        DCHECK(has_fixed_slot_policy());
        return bundle()->value;
    }
    
    int fixed_register_id() const {
        DCHECK(has_fixed_reigster_policy());
        return bundle()->value;
    }
    
    int fixed_fp_register_id() const {
        DCHECK(has_fixed_fp_reigster_policy());
        return bundle()->value;
    }
    
    int input_index() const {
        DCHECK(has_same_as_input_policy());
        return bundle()->value;
    }
    
    bool is_used_at_start() const { return life_time() == kUsedAtStart; }
    
    bool is_used_at_end() const {
        DCHECK(!has_fixed_slot_policy());
        return life_time() == kUsedAtEnd;
    }
    
    //DISALLOW_IMPLICIT_CONSTRUCTORS(UnallocatedOperand);
private:
    UnallocatedOperand(int virtual_register)
    : InstructionOperand(kUnallocated) {
        mutable_bundle()->vid = virtual_register;
    }
    
    UnallocatedBundle *mutable_bundle() { return &unallocated_values_; }
    const UnallocatedBundle *bundle() const { return &unallocated_values_; }
}; // class UnallocatedOperand

static_assert(sizeof(UnallocatedOperand) == sizeof(InstructionOperand), "");

class ImmediateOperand final : public InstructionOperand {
public:
    ImmediateOperand(int8_t value): ImmediateOperand(MachineRepresentation::kWord8) {
        mutable_bundle()->word8_value = value;
    }
    
    ImmediateOperand(int16_t value): ImmediateOperand(MachineRepresentation::kWord16) {
        mutable_bundle()->word16_value = value;
    }
    
    ImmediateOperand(int32_t value): ImmediateOperand(MachineRepresentation::kWord32) {
        mutable_bundle()->word32_value = value;
    }
    
    ImmediateOperand(int64_t value): ImmediateOperand(MachineRepresentation::kWord32) {
        mutable_bundle()->word64_value = value;
    }
    
    MachineRepresentation machine_representation() const { return bundle()->rep; }
    int8_t word8_value() const { return bundle()->word8_value; }
    int16_t word16_value() const { return bundle()->word16_value; }
    int32_t word32_value() const { return bundle()->word32_value; }
    int64_t word64_value() const { return bundle()->word64_value; }
    float float32_value() const { return bundle()->float32_value; }
    double float64_value() const { return bundle()->float64_value; }

private:
    ImmediateOperand(MachineRepresentation rep)
    : InstructionOperand(kImmediate) {
        mutable_bundle()->rep = rep;
    }

    ImmediateBundle *mutable_bundle() { return &immediate_values_; }
    const ImmediateBundle *bundle() const { return &immediate_values_; }
}; // class ImmediateOperand

static_assert(sizeof(ImmediateOperand) == sizeof(InstructionOperand), "");

class ConstantOperand final : public InstructionOperand {
public:
    enum Type {
        kString,
        kNumber,
    };
    
    ConstantOperand(Type type, int symbol_id)
    : InstructionOperand(kConstant) {
        mutable_bundle()->type = static_cast<int>(type);
        mutable_bundle()->symbol_id = symbol_id;
    }
    
    Type type() const { return static_cast<Type>(bundle()->type); }
    int symbol_id() const { return bundle()->symbol_id; }
    
private:
    ConstantBundle *mutable_bundle() { return &constant_values_; }
    const ConstantBundle *bundle() const { return &constant_values_; }
}; // class ConstantOperand

static_assert(sizeof(ConstantOperand) == sizeof(InstructionOperand), "");

class ReloactionOperand final : public InstructionOperand {
public:
    ReloactionOperand(const String *symbol_name, int offset = 0, bool fetch_address = false)
    : ReloactionOperand(offset, fetch_address) {
        mutable_bundle()->label_or_symbol = false;
        mutable_bundle()->symbol_name = symbol_name;
    }
    
    ReloactionOperand(const InstructionBlock *label)
    : ReloactionOperand(0, false) {
        mutable_bundle()->label_or_symbol = true;
        mutable_bundle()->label = label;
    }
    
    ReloactionOperand(const ReloactionOperand &other, int offset, bool fetch_address = false)
    : ReloactionOperand(offset, fetch_address) {
        if (other.is_label()) {
            mutable_bundle()->label_or_symbol = true;
            mutable_bundle()->label = other.label();
        } else {
            mutable_bundle()->label_or_symbol = false;
            mutable_bundle()->symbol_name = other.symbol_name();
        }
    }

    bool is_symbol() const { return !is_label(); }
    bool is_label() const { return bundle()->label_or_symbol; }
    
    const InstructionBlock *label() const {
        DCHECK(is_label());
        return bundle()->label;
    }

    const String *symbol_name() const {
        DCHECK(is_symbol());
        return bundle()->symbol_name;
    }
    
    int offset() const {
        DCHECK(is_symbol());
        return bundle()->offset;
    }
    
    bool should_fetch_address() const { return bundle()->fetch_address; }
private:
    ReloactionOperand(int offset, bool fetch_address)
    : InstructionOperand(kReloaction) {
        mutable_bundle()->fetch_address = fetch_address;
        mutable_bundle()->offset = offset;
    }
    
    ReloactionBundle *mutable_bundle() { return &reloaction_values_; }
    const ReloactionBundle *bundle() const { return &reloaction_values_; }
}; // class ReloactionOperand

static_assert(sizeof(ReloactionOperand) == sizeof(InstructionOperand), "");

class AllocatedOperand final : public InstructionOperand {
public:
    enum LocationKind {
        kRegister,
        kSlot,
    };
    
    AllocatedOperand(LocationKind kind, MachineRepresentation rep, int index)
    : InstructionOperand(kAllocated) {
        mutable_bundle()->location_kind = static_cast<int>(kind);
        mutable_bundle()->rep = static_cast<int>(rep);
        mutable_bundle()->index = index;
    }
    
    LocationKind location_kind() const { return static_cast<LocationKind>(bundle()->location_kind); }
    MachineRepresentation machine_representation() const { return static_cast<MachineRepresentation>(bundle()->rep); }
    int index() const { return bundle()->index; }
    
    bool IsRegisterLocation() const { return location_kind() == kRegister; }
    bool IsSlotLocation() const { return location_kind() == kSlot; }

private:
    AllocatedBundle *mutable_bundle() { return &allocated_values_; }
    const AllocatedBundle *bundle() const { return &allocated_values_; }
}; // class LocationOperand

static_assert(sizeof(AllocatedOperand) == sizeof(InstructionOperand), "");


class Instruction final {
public:
    using Code = InstructionCode;
    using Operand = InstructionOperand;
    
    DEF_VAL_GETTER(Code, op);
    int inputs_count() const { return inputs_count_; }
    int outputs_count() const { return outputs_count_; }
    int temps_count() const { return temps_count_; }
    size_t operands_size() const { return inputs_count() + outputs_count() + temps_count(); }
    
    Operand *InputAt(size_t i) {
        DCHECK(i < inputs_count());
        return &operands_[input_offset() + i];
    }
    
    Operand *OutputAt(size_t i) {
        DCHECK(i < outputs_count());
        return &operands_[output_offset() + i];
    }
    
    static Instruction *New(base::Arena *arena, Code op,
                            size_t inputs_count,
                            Operand inputs[],
                            size_t outputs_count,
                            Operand outputs[],
                            size_t temps_count,
                            Operand temps[]);
    
    friend class InstructionBlock;
    friend class InstructionSelector;
    DISALLOW_IMPLICIT_CONSTRUCTORS(Instruction);
private:
    Instruction(Code op,
                size_t inputs_count,
                Operand inputs[],
                size_t outputs_count,
                Operand outputs[],
                size_t temps_count,
                Operand temps[]);
    
    constexpr int input_offset() const { return 0; }
    int output_offset() const { return input_offset() + inputs_count_; }
    int temp_offset() const { return output_offset() + outputs_count_; }
    
    static void *AllocatePlacementMemory(base::Arena *arena, size_t inputs_count, size_t outputs_count,
                                         size_t temps_count);
    
    Code op_;
    uint8_t inputs_count_;
    uint8_t outputs_count_;
    uint8_t temps_count_;
    uint8_t is_call_;
    Operand operands_[1];
}; // class Instruction


class InstructionFunction final : public base::ArenaObject {
public:
    using SymbolMap = base::ArenaUnorderedMap<std::string_view, ReloactionOperand *>;
    
    InstructionFunction(base::Arena *arena, const String *symbol);
    
    DEF_PTR_GETTER(const String, symbol);
    DEF_PTR_PROP_RW(InstructionFunction, native_handle);
    DEF_VAL_GETTER(SymbolMap, external_symbols);
    DEF_ARENA_VECTOR_GETTER(InstructionBlock *, block);
    
    inline InstructionBlock *NewBlock(int label);
    
//    ReloactionOperand *AddExternalSymbol(const String *symbol, bool fetch_address = false) {
//        if (auto iter = external_symbols_.find(symbol->ToSlice()); iter != external_symbols_.end()) {
//            return iter->second;
//        }
//        auto rel = new (arena_) ReloactionOperand(symbol, nullptr, fetch_address);
//        external_symbols_[symbol->ToSlice()] = rel;
//        return rel;
//    }
//
//    ReloactionOperand *AddExternalSymbol(const std::string_view symbol, bool fetch_address = false) {
//        if (auto iter = external_symbols_.find(symbol); iter != external_symbols_.end()) {
//            return iter->second;
//        }
//        auto ass = String::New(arena_, symbol);
//        auto rel = new (arena_) ReloactionOperand(ass, nullptr, fetch_address);
//        external_symbols_[ass->ToSlice()] = rel;
//        return rel;
//    }
//
//    ReloactionOperand *AddArrayElementClassSymbol(const ir::ArrayModel *ar, bool fetch_address = false);
//
//    ReloactionOperand *AddClassSymbol(const ir::Type &ty, bool fetch_address = false);
private:
//    ReloactionOperand *FindExternalSymbolOrNull(const std::string_view symbol) const {
//        if (auto iter = external_symbols_.find(symbol); iter != external_symbols_.end()) {
//            return iter->second;
//        }
//        return nullptr;
//    }
//
//    ReloactionOperand *InsertExternalSymbol(const std::string_view symbol, ReloactionOperand *rel) {
//        external_symbols_[symbol] = rel;
//        return rel;
//    }
    
    const String *const symbol_;
    base::Arena *const arena_;
    InstructionFunction *native_handle_ = nullptr;
    SymbolMap external_symbols_;
    base::ArenaVector<InstructionBlock *> blocks_;
}; // class InstructionFunction

class InstructionBlock final : public base::ArenaObject {
public:
    explicit InstructionBlock(base::Arena *arena, InstructionFunction *owns, int label);

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
