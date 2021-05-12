#pragma once
#ifndef YALX_BASE_UTILS_H_
#define YALX_BASE_UTILS_H_

#include "base/checking.h"
#include <stdint.h>

namespace yalx {

namespace base {

static constexpr int kBitsPerByte = 8;

// Bit field extraction.
inline uint32_t unsigned_bitextract_32(int msb, int lsb, uint32_t x) {
    return (x >> lsb) & ((1 << (1 + msb - lsb)) - 1);
}

inline uint64_t unsigned_bitextract_64(int msb, int lsb, uint64_t x) {
    return (x >> lsb) & ((static_cast<uint64_t>(1) << (1 + msb - lsb)) - 1);
}

inline int32_t signed_bitextract_32(int msb, int lsb, uint32_t x) {
    return static_cast<int32_t>(x << (31 - msb)) >> (lsb + 31 - msb);
}

// Check number width.
inline bool is_intn(int64_t x, unsigned n) {
    assert((0 < n) && (n < 64));
    int64_t limit = static_cast<int64_t>(1) << (n - 1);
    return (-limit <= x) && (x < limit);
}

inline bool is_uintn(int64_t x, unsigned n) {
    assert((0 < n) && (n < (sizeof(x) * kBitsPerByte)));
    return !(x >> n);
}

template <class T>
inline T truncate_to_intn(T x, unsigned n) {
    assert((0 < n) && (n < (sizeof(x) * kBitsPerByte)));
    return (x & ((static_cast<T>(1) << n) - 1));
}

// clang-format off
#define INT_1_TO_63_LIST(V)                                   \
  V(1) V(2) V(3) V(4) V(5) V(6) V(7) V(8) V(9) V(10)          \
  V(11) V(12) V(13) V(14) V(15) V(16) V(17) V(18) V(19) V(20) \
  V(21) V(22) V(23) V(24) V(25) V(26) V(27) V(28) V(29) V(30) \
  V(31) V(32) V(33) V(34) V(35) V(36) V(37) V(38) V(39) V(40) \
  V(41) V(42) V(43) V(44) V(45) V(46) V(47) V(48) V(49) V(50) \
  V(51) V(52) V(53) V(54) V(55) V(56) V(57) V(58) V(59) V(60) \
  V(61) V(62) V(63)
// clang-format on

#define DECLARE_IS_INT_N(N) \
  inline bool is_int##N(int64_t x) { return is_intn(x, N); }
#define DECLARE_IS_UINT_N(N)    \
  template <class T>            \
  inline bool is_uint##N(T x) { \
    return is_uintn(x, N);      \
  }
#define DECLARE_TRUNCATE_TO_INT_N(N) \
  template <class T>                 \
  inline T truncate_to_int##N(T x) { \
    return truncate_to_intn(x, N);   \
  }
INT_1_TO_63_LIST(DECLARE_IS_INT_N)
INT_1_TO_63_LIST(DECLARE_IS_UINT_N)
INT_1_TO_63_LIST(DECLARE_TRUNCATE_TO_INT_N)
#undef DECLARE_IS_INT_N
#undef DECLARE_IS_UINT_N
#undef DECLARE_TRUNCATE_TO_INT_N

} // namespace base

} // namespace yalx


#endif // YALX_BASE_UTILS_H_
