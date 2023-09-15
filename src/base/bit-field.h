#pragma once
#ifndef YALX_BASE_BIT_FIELD_H_
#define YALX_BASE_BIT_FIELD_H_

#include "base/checking.h"
#include <stdint.h>
#include <type_traits>

namespace yalx::base {

template<class T, int shift, int size, class U = uint32_t>
struct BitField final {
    static_assert(std::is_unsigned<U>::value);
    static_assert(shift < 8 * sizeof(U));  // Otherwise shifts by {shift} are UB.
    static_assert(size < 8 * sizeof(U));   // Otherwise shifts by {size} are UB.
    static_assert(shift + size <= 8 * sizeof(U));
    static_assert(size > 0);
    
    static constexpr int kShift = shift;
    static constexpr int kSize  = size;
    static constexpr U kBits = (U{1} << (kSize + 1)) - 1;
    static constexpr U kMask = ((U{1} << kShift) << kSize) - (U{1} << kShift);
    static constexpr U kValuesCount = U{1} << kSize;
    static constexpr T kMax = static_cast<T>(kValuesCount - 1);
    
    template <class T2, int size2>
    using Next = BitField<T2, kShift + kSize, size2, U>;
    
    
    // Tells whether the provided value fits into the bit field.
    static constexpr bool IsValid(T value) {
        return (static_cast<U>(value) & ~static_cast<U>(kMax)) == 0;
    }

    // Returns a type U with the bit field value encoded.
    static constexpr U Encode(T value) {
        DCHECK(IsValid(value));
        return static_cast<U>(value) << kShift;
    }

    // Returns a type U with the bit field value updated.
    static constexpr U Update(U previous, T value) {
      return (previous & ~kMask) | Encode(value);
    }

    // Extracts the bit field from the value.
    static constexpr T Decode(U value) {
        return static_cast<T>((value & kMask) >> kShift);
    }
};

} // namespace yalx

#endif // YALX_BASE_BIT_FIELD_H_
