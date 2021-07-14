#pragma once
#ifndef YALX_IR_TYPE_H_
#define YALX_IR_TYPE_H_

#include "base/checking.h"
#include "base/base.h"

namespace yalx {

namespace ir {

#define DECLARE_HIR_TYPES(V) \
    V(Void,    0,  0) \
    V(Word8,   8,  Type::kNumberBit) \
    V(Word16,  16, Type::kNumberBit) \
    V(Word32,  32, Type::kNumberBit) \
    V(Word64,  64, Type::kNumberBit) \
    V(Int8,    8,  Type::kSignedBit|Type::kNumberBit) \
    V(Int16,   16, Type::kSignedBit|Type::kNumberBit) \
    V(Int32,   32, Type::kSignedBit|Type::kNumberBit) \
    V(Int64,   64, Type::kSignedBit|Type::kNumberBit) \
    V(UInt8,   8,  Type::kNumberBit) \
    V(UInt16,  16, Type::kNumberBit) \
    V(UInt32,  32, Type::kNumberBit) \
    V(UInt64,  64, Type::kNumberBit) \
    V(Float32, 32, Type::kSignedBit|Type::kNumberBit) \
    V(Float64, 64, Type::kSignedBit|Type::kNumberBit|Type::kFloatBit) \
    V(Reference, kPointerSize * 8, Type::kReferenceBit)

class Type {
public:
    enum Kind {
#define DEFINE_KINDS(name, ...) k##name,
        DECLARE_HIR_TYPES(DEFINE_KINDS)
#undef DEFINE_KINDS
    };
    
    static constexpr uint32_t kSignedBit = 1u;
    static constexpr uint32_t kNumberBit = 1u << 1;
    static constexpr uint32_t kFloatBit = 1u << 2;
    static constexpr uint32_t kReferenceBit = 1u << 3;
    
    constexpr Type(Kind kind, int bits, uint32_t flags)
        : kind_(kind)
        , bits_(bits)
        , flags_(flags) {}
    
    DEF_VAL_GETTER(Kind, kind);
    DEF_VAL_GETTER(int, bits);
    int bytes() const { return bits_ / 8; }
    
    bool is_signed() const { return flags_ & kSignedBit; }
    bool is_unsigned() const { return !is_signed(); }
    bool is_float() const { return flags_ & kFloatBit; }
    bool is_number() const { return flags_ & kNumberBit; }
    bool is_reference() const { return flags_ & kReferenceBit; }
    
private:
    const Kind kind_;
    const int bits_;
    const uint32_t flags_;
}; // class Type

struct Types {
#define DEFINE_TYPES(name, bits, flags) \
static constexpr Type name = Type(Type::k##name, bits, flags);
    DECLARE_HIR_TYPES(DEFINE_TYPES)
#undef DEFINE_TYPES
}; // struct Types

} // namespace ir

} // namespace yalx

#endif // YALX_IR_TYPE_H_
