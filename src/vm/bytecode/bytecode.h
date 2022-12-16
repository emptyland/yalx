#pragma once
#ifndef YALX_VM_BYTECODE_BYTECODE_H_
#define YALX_VM_BYTECODE_BYTECODE_H_

#include "base/checking.h"
#include "base/arena-utils.h"
#include "base/base.h"

namespace yalx {

namespace vm {

enum class OperandKind {
    kNone,
    kStackSlot,
    kConstPoolOffset,
    kFunTableOffset,
    kGlobalZoneOffset,
    kImm,
};

enum class AccumulatorAccessing {
    kUseless,
    kIn,
    kOut,
    kInOut
};

enum class Bitwise: uint8_t {
    kNone,   // Ignore
    kByte,   // 8 bits
    kWord,   // 16 bits
    kLong,   // 32 bits
    kQuad,   // 64 bits
    kFloat,  // 32 floating bits
    kDouble, // 64 floating bits
};

#define DECL_BYTECODES(V) \
    V(Wide16, Useless, 0) \
    V(Wide32, Useless, 0) \
    V(Wide64, Useless, 0) \
    V(Lda,    Out,     1, StackSlot) \
    V(Sta,    In,      1, StackSlot)

template<class T> class BytecodeNode;

class Bytecode : public base::ArenaObject {
public:
    enum Opcode: uint8_t {
#define DEFINE_OPCODE(name, ...) k##name,
        DECL_BYTECODES(DEFINE_OPCODE)
#undef DEFINE_OPCODE
    };
    
    constexpr static int kMaxOperands = 4;
    
    Bytecode(Opcode opcode, Bitwise bitwise)
    : Bytecode(opcode, opcode, bitwise) {
        DCHECK(is_wide_8bits());
    }
    
    Bytecode(Opcode prefix, Opcode opcode, Bitwise bitwise)
    : prefix_(prefix)
    , opcode_(opcode)
    , bitwise_(bitwise) {
    }
    
    bool is_wide_8bits() const {
        switch (prefix()) {
            case kWide16:
            case kWide32:
            case kWide64:
                return false;
                
            default:
                return true;
        }
    }
    bool is_wide_16bits() const { return prefix() == kWide16; }
    bool is_wide_32bits() const { return prefix() == kWide32; }
    bool is_wide_64bits() const { return prefix() == kWide64; }
    
    bool has_not_bitwise() const { return bitwise() == Bitwise::kNone; }
    bool has_bitwise() const { return !has_not_bitwise(); }
    
    DEF_VAL_GETTER(Opcode, prefix);
    DEF_VAL_GETTER(Opcode, opcode);
    DEF_VAL_GETTER(Bitwise, bitwise);
    
    inline static bool IsWidePrefix(Opcode opcode) {
        switch (opcode) {
            case kWide16:
            case kWide32:
            case kWide64:
                return true;
                
            default:
                return false;
        }
    }
    
    int OperandsCount() const;
    
    size_t Encode(uint8_t **buf) const;
    
    uint8_t prefix_byte() const { return static_cast<uint8_t>(prefix()); }
    uint8_t opcode_byte() const { return static_cast<uint8_t>(opcode()); }
    uint8_t bitwise_byte() const { return static_cast<uint8_t>(bitwise()); }
    
    template<class T>
    inline static constexpr BytecodeNode<T> *Cast(Bytecode *);
    
    template<class T>
    inline static constexpr const BytecodeNode<T> *Cast(const Bytecode *);
  
    DISALLOW_IMPLICIT_CONSTRUCTORS(Bytecode);
private:
    const Opcode prefix_;
    const Opcode opcode_;
    const Bitwise bitwise_;
}; // class Bytecode

template<class T>
struct BytecodeWideTraits {
    inline static constexpr Bytecode::Opcode Prefix(Bytecode::Opcode opcode) { return Bytecode::Opcode(-1); }
};

template<>
struct BytecodeWideTraits<int8_t> {
    inline static constexpr Bytecode::Opcode Prefix(Bytecode::Opcode opcode) { return opcode; }
};

template<>
struct BytecodeWideTraits<uint8_t> {
    inline static constexpr Bytecode::Opcode Prefix(Bytecode::Opcode opcode) { return opcode; }
};

template<>
struct BytecodeWideTraits<int16_t> {
    inline static constexpr Bytecode::Opcode Prefix(Bytecode::Opcode opcode) { return Bytecode::kWide16; }
};

template<>
struct BytecodeWideTraits<uint16_t> {
    inline static constexpr Bytecode::Opcode Prefix(Bytecode::Opcode opcode) { return Bytecode::kWide16; }
};

template<>
struct BytecodeWideTraits<int32_t> {
    inline static constexpr Bytecode::Opcode Prefix(Bytecode::Opcode opcode) { return Bytecode::kWide32; }
};

template<>
struct BytecodeWideTraits<uint32_t> {
    inline static constexpr Bytecode::Opcode Prefix(Bytecode::Opcode opcode) { return Bytecode::kWide32; }
};

template<>
struct BytecodeWideTraits<int64_t> {
    inline static constexpr Bytecode::Opcode Prefix(Bytecode::Opcode opcode) { return Bytecode::kWide64; }
};

template<>
struct BytecodeWideTraits<uint64_t> {
    inline static constexpr Bytecode::Opcode Prefix(Bytecode::Opcode opcode) { return Bytecode::kWide64; }
};

template<>
struct BytecodeWideTraits<float> {
    inline static constexpr Bytecode::Opcode Prefix(Bytecode::Opcode opcode) { return Bytecode::kWide32; }
};

template<>
struct BytecodeWideTraits<double> {
    inline static constexpr Bytecode::Opcode Prefix(Bytecode::Opcode opcode) { return Bytecode::kWide64; }
};

template<class T>
class BytecodeNode : public Bytecode {
public:
    BytecodeNode(Opcode opcode, Bitwise bitwise, T a, T b, T c, T d)
    : Bytecode(BytecodeWideTraits<T>::Prefix(opcode), opcode, bitwise) {
        operands_[0] = a;
        operands_[1] = b;
        operands_[2] = c;
        operands_[3] = d;
    }

    T operand(int index) const {
        DCHECK(index >= 0);
        DCHECK(index < kMaxOperands);
        return operands_[index];
    }
    
    size_t Encode(uint8_t **buf) const {
        const auto origin = *buf;
        if (!is_wide_8bits()) {
            *(*buf)++ = prefix_byte();
        }
        *(*buf)++ = opcode_byte();
        if (has_bitwise()) {
            *(*buf)++ = bitwise_byte();
        }
        const auto n = OperandsCount();
        for (auto i = 0; i < n; i++) {
            *reinterpret_cast<T *>(*buf) = operand(i);
            (*buf) += sizeof(T);
        }
        return static_cast<size_t>(*buf - origin);
    }
    
    size_t PackedSize() const {
        return OperandsCount() * sizeof(T) + (is_wide_8bits() ? 0 : 1) + (has_bitwise() ? 1 : 0);
    }
private:
    T operands_[kMaxOperands];
}; // class BytecodeNode


template<class T>
inline constexpr BytecodeNode<T> *Bytecode::Cast(Bytecode *bc) {
    DCHECK(bc || BytecodeWideTraits<T>::Prefix(bc->opcode()) == bc->prefix());
    return static_cast<BytecodeNode<T> *>(bc);
}

template<class T>
inline constexpr const BytecodeNode<T> *Bytecode::Cast(const Bytecode *bc) {
    DCHECK(bc || BytecodeWideTraits<T>::Prefix(bc->opcode()) == bc->prefix());
    return static_cast<const BytecodeNode<T> *>(bc);
}

} // namespace vm

} // namespace yalx

#endif // YALX_VM_BYTECODE_BYTECODE_H_
