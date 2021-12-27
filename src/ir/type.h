#pragma once
#ifndef YALX_IR_TYPE_H_
#define YALX_IR_TYPE_H_

#include "base/checking.h"
#include "base/base.h"
#include <string>

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
    V(String, kPointerSize * 8, Type::kReferenceBit)

class Model;

class Type {
public:
    enum Kind {
#define DEFINE_KINDS(name, ...) k##name,
        DECLARE_HIR_TYPES(DEFINE_KINDS)
        kReference,
        kValue,
#undef DEFINE_KINDS
    };
    
    static constexpr uint32_t kSignedBit = 1u;
    static constexpr uint32_t kNumberBit = 1u << 1;
    static constexpr uint32_t kFloatBit = 1u << 2;
    static constexpr uint32_t kReferenceBit = 1u << 3;
    static constexpr uint32_t kNullableBit = 1u << 4;
    
    constexpr Type(): Type(kVoid, 0, 0) {}
    constexpr Type(Kind kind, int bits, uint32_t flags)
        : Type(kind, bits, flags, nullptr) {}
    
    DEF_VAL_GETTER(Kind, kind);
    DEF_VAL_GETTER(int, bits);
    DEF_PTR_GETTER(Model, model);
    
    int bytes() const { return bits_ / 8; }
    
    bool is_signed() const { return flags_ & kSignedBit; }
    bool is_unsigned() const { return !is_signed(); }
    bool is_float() const { return flags_ & kFloatBit; }
    bool is_number() const { return flags_ & kNumberBit; }
    bool is_reference() const { return flags_ & kReferenceBit; }
    bool is_none_nullable() const { return !is_nullable(); }
    bool is_nullable() const { return flags_ & kNullableBit; }
    
    std::string_view ToString() const;

    static Type Ref(Model *model, bool _nullable = false);
    static Type Val(Model *model);
private:
    constexpr Type(Kind kind, int bits, uint32_t flags, Model *model)
        : kind_(kind)
        , bits_(bits)
        , flags_(flags)
        , model_(model) {}
    
    const Kind kind_;
    const int bits_;
    const uint32_t flags_;
    Model *const model_;
}; // class Type

struct Types {
#define DEFINE_TYPES(name, bits, flags) \
static constexpr Type name = Type(Type::k##name, bits, flags);
    DECLARE_HIR_TYPES(DEFINE_TYPES)
#undef DEFINE_TYPES
}; // struct Types

enum Access {
    kExport,
    kPublic,
    kProtected,
    kPrivate,
};

} // namespace ir

} // namespace yalx

#endif // YALX_IR_TYPE_H_
