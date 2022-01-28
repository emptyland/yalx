#pragma once
#ifndef YALX_BACKEND_CONSTANTS_POOL_H_
#define YALX_BACKEND_CONSTANTS_POOL_H_

#include "base/arena-utils.h"
#include "base/hash.h"
#include "base/base.h"

namespace yalx {

namespace backend {

using String = base::ArenaString;

class ConstantsPool final {
public:
    enum Kind {
        kWord8,
        kWord16,
        kWord32,
        kWord64,
        kFloat32,
        kFloat64,
        kString,
    }; // enum Kind

    struct Slot {
        Kind    kind;
        uint8_t data[8];
        
        template<class T> inline T *location() { return reinterpret_cast<T *>(data); }
        template<class T> inline T value() { return *location<T>(); }
    }; // struct Slot
    
    struct SlotHash : public std::unary_function<Slot, size_t> {
        size_t operator () (Slot value) const {
            return base::Hash::Js(reinterpret_cast<const char *>(&value), sizeof(value));
        }
    }; // struct SlotHash
    
    struct SlotEqualTo : public std::binary_function<Slot, Slot, bool> {
        bool operator () (Slot lhs, Slot rhs) const {
            return lhs.kind == rhs.kind && ::memcmp(lhs.data, rhs.data, 8) == 0;
        }
    }; // struct SlotEqualTo
    
    using NumbersMap = base::ArenaUnorderedMap<Slot, int, SlotHash, SlotEqualTo>;
    using StringMap = base::ArenaMap<std::string_view, int>;
    using StringPool = base::ArenaVector<const String *>;
    
    ConstantsPool(base::Arena *arena);
    
    DEF_VAL_GETTER(NumbersMap, numbers);
    DEF_VAL_GETTER(StringPool, string_pool);
    
    int FindOrInsertWord8(uint8_t value) {
        return FindOrInsertSlot(kWord8, &value, sizeof(value));
    }
    
    int FindOrInsertWord16(uint16_t value) {
        return FindOrInsertSlot(kWord16, &value, sizeof(value));
    }
    
    int FindOrInsertWord32(uint32_t value) {
        return FindOrInsertSlot(kWord32, &value, sizeof(value));
    }
    
    int FindOrInsertWord64(uint64_t value) {
        return FindOrInsertSlot(kWord64, &value, sizeof(value));
    }
    
    int FindOrInsertFloat32(float value) {
        return FindOrInsertSlot(kFloat32, &value, sizeof(value));
    }
    
    int FindOrInsertFloat64(double value) {
        return FindOrInsertSlot(kFloat64, &value, sizeof(value));
    }
    
    int FindOrInsertString(const String *value);
private:
    int FindOrInsertSlot(Kind kind, const void *data, size_t size);
    
    base::Arena *const arena_;
    NumbersMap numbers_;
    StringMap  strings_;
    StringPool string_pool_;
    int next_unique_id_ = 0;
}; // class ConstantsPool

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_CONSTANTS_POOL_H_
