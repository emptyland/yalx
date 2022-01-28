#pragma once
#ifndef YALX_IR_TYPE_H_
#define YALX_IR_TYPE_H_

#include "base/checking.h"
#include "base/base.h"
#include <string>

namespace yalx {
namespace base {
class ArenaString;
class PrintingWriter;
} // namespace base
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
    V(Float32, 32, Type::kSignedBit|Type::kNumberBit|Type::kFloatBit) \
    V(Float64, 64, Type::kSignedBit|Type::kNumberBit|Type::kFloatBit) \
    V(String,  kPointerSize * 8, Type::kReferenceBit)

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
    static constexpr uint32_t kPointerBit = 1u << 4;
    static constexpr uint32_t kNullableBit = 1u << 5;
    
    constexpr Type(): Type(kVoid, 0, 0) {}
    constexpr Type(Kind kind, int bits, uint32_t flags)
        : Type(kind, bits, flags, nullptr) {}
    
    DEF_VAL_GETTER(Kind, kind);
    DEF_VAL_GETTER(int, bits);
    DEF_PTR_GETTER(Model, model);
    
    int bytes() const { return bits_ / 8; }
    
    bool IsSigned() const { return flags_ & kSignedBit; }
    bool IsUnsigned() const { return !IsSigned(); }
    bool IsFloating() const { return flags_ & kFloatBit; }
    bool IsIntegral() const { return !IsFloating(); }
    bool IsNumber() const { return flags_ & kNumberBit; }
    bool IsReference() const { return flags_ & kReferenceBit; }
    bool IsNotPointer() const { return !IsPointer(); }
    bool IsPointer() const { return flags_ & kPointerBit; }
    bool IsNoneNullable() const { return !IsNullable(); }
    bool IsNullable() const { return flags_ & kNullableBit; }
    
    bool Equals(const Type &other) const { return kind() == other.kind() && model() == other.model(); }
    
    std::string_view ToString() const;
    void PrintTo(base::PrintingWriter *printer) const;

    static Type Ref(Model *model, bool _nullable = false);
    static Type Val(Model *model, bool pointer = false);
    //static Type String(Model *model, bool _nullable = false);
private:
    constexpr Type(Kind kind, int bits, uint32_t flags, Model *model)
        : kind_(kind)
        , bits_(bits)
        , flags_(flags)
        , model_(model) {}
    
    Kind kind_;
    int bits_;
    uint32_t flags_;
    Model *model_;
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
