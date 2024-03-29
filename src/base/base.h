#pragma once
#ifndef YALX_BASE_BASE_H_
#define YALX_BASE_BASE_H_

#include "runtime/macros.h"
#include <type_traits>
#include <utility>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

namespace yalx {

#define DISALLOW_IMPLICIT_CONSTRUCTORS(clazz_name) \
    clazz_name (const clazz_name &) = delete;      \
    clazz_name (clazz_name &&) = delete;           \
    void operator = (const clazz_name &) = delete;

#define DISALLOW_ALL_CONSTRUCTORS(clazz_name)      \
    clazz_name () = delete;                        \
    DISALLOW_IMPLICIT_CONSTRUCTORS(clazz_name)     \

/**
 * disable copy constructor, assign operator function.
 *
 */
class DisallowImplicitConstructors {
public:
    DisallowImplicitConstructors() = default;

    DISALLOW_IMPLICIT_CONSTRUCTORS(DisallowImplicitConstructors)
};

/**
 * Get size of array.
 */
template <class T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];

#ifndef _MSC_VER
template <class T, size_t N>
char (&ArraySizeHelper(const T (&array)[N]))[N];
#endif

#ifndef arraysize
#define arraysize(array) (sizeof(::yalx::ArraySizeHelper(array)))
#endif

/**
 * Safaty down cast
 */
template<class T, class F>
inline T *down_cast(F *from) {
#if defined(DEBUG) || defined(_DEBUG)
    assert(!from || dynamic_cast<T *>(from) && "Can not cast to.");
#endif
    return static_cast<T *>(from);
}

template<class T, class S>
inline T bit_cast(S from) {
    return *reinterpret_cast<const T *>(&from);
}

template<class T, class S>
inline T uninterpret_cast(S from) {
    static_assert(sizeof(T) >= sizeof(S), "incorrect cast");
    union {
        S s;
        T t;
    };
    s = from;
    return t;
}

#define IS_POWER_OF_TWO(x) (((x) & ((x) - 1)) == 0)

// Returns true iff x is a power of 2 (or zero). Cannot be used with the
// maximally negative value of the type T (the -1 overflows).
template <typename T>
constexpr inline bool IsPowerOf2(T x) {
    return IS_POWER_OF_TWO(x);
}

// Compute the 0-relative offset of some absolute value x of type T.
// This allows conversion of Addresses and integral types into
// 0-relative int offsets.
template <typename T>
constexpr inline intptr_t OffsetFrom(T x) {
    return x - static_cast<T>(0);
}


// Compute the absolute value of type T for some 0-relative offset x.
// This allows conversion of 0-relative int offsets into Addresses and
// integral types.
template <typename T>
constexpr inline T AddressFrom(intptr_t x) {
    return static_cast<T>(static_cast<T>(0) + x);
}


// Return the largest multiple of m which is <= x.
template <typename T>
constexpr inline T RoundDown(T x, intptr_t m) {
    assert(IsPowerOf2(m));
    return AddressFrom<T>(OffsetFrom(x) & -m);
}


// Return the smallest multiple of m which is >= x.
template <typename T>
constexpr inline T RoundUp(T x, intptr_t m) {
    return RoundDown<T>(static_cast<T>(x + m - 1), m);
}

template <class T>
constexpr inline T AlignDownBounds(T bounds, size_t value) {
    return (value + bounds - 1) & (~(bounds - 1));
}

template <typename T, typename U>
constexpr inline bool IsAligned(T value, U alignment) {
    return (value & (alignment - 1)) == 0;
}

template<class T>
inline int ComputeValueShift(T value) {
    int shift;
    for (shift = 0; (1 << shift) < value; shift++) {
    }
    return shift;
}

// Array view template
template<class T>
struct View {
    T const     *z;
    size_t const n;
};

template<class T>
struct MutView {
    T     *z;
    size_t n;
};

template<class T>
inline View<T> MakeView(T const *z, size_t n) { return View<T>{z, n}; }

template<class T>
inline MutView<T> MakeMutView(T *z, size_t n) { return MutView<T>{z, n}; }


static constexpr uint32_t kInitZag = 0xcccccccc;
static constexpr uint32_t kFreeZag = 0xfeedfeed;

// Round bytes filling
// For int16, 32, 64 filling:
void *Round16BytesFill(uint16_t zag, void *chunk, size_t n);
void *Round32BytesFill(uint32_t zag, void *chunk, size_t n);
void *Round64BytesFill(uint64_t zag, void *chunk, size_t n);

#if defined(NDEBUG)

template<class T> inline T *DbgInitZag(T *chunk, size_t) { return chunk; }
template<class T> inline T *DbgFreeZag(T *chunk, size_t) { return chunk; }

#else // !defined(NDEBUG)

template<class T>
inline T *DbgInitZag(T *chunk, size_t n) { return static_cast<T *>(Round32BytesFill(kInitZag, chunk, n)); }

template<class T>
inline T *DbgFreeZag(T *chunk, size_t n) { return static_cast<T *>(Round32BytesFill(kFreeZag, chunk, n)); }

#endif // #if defined(NDEBUG)

/**
 * define getter/mutable_getter/setter
 */
#define DEF_VAL_PROP_RMW(type, name) \
    DEF_VAL_GETTER(type, name) \
    DEF_VAL_MUTABLE_GETTER(type, name) \
    DEF_VAL_SETTER(type, name)

#define DEF_VAL_PROP_RM(type, name) \
    DEF_VAL_GETTER(type, name) \
    DEF_VAL_MUTABLE_GETTER(type, name)

#define DEF_VAL_PROP_RW(type, name) \
    DEF_VAL_GETTER(type, name) \
    DEF_VAL_SETTER(type, name)

#define DEF_PTR_PROP_RW(type, name) \
    DEF_PTR_GETTER(type, name) \
    DEF_PTR_SETTER(type, name)

#define DEF_PTR_PROP_RW_NOTNULL1(type, name) \
    DEF_PTR_GETTER(type, name) \
    DEF_PTR_SETTER_NOTNULL(type, name)

#define DEF_PTR_PROP_RW_NOTNULL2(type, name) \
    DEF_PTR_GETTER_NOTNULL(type, name) \
    DEF_PTR_SETTER_NOTNULL(type, name)

#define DEF_VAL_GETTER(type, name) \
    [[nodiscard]] inline const type &name() const { return name##_; }

#define DEF_VAL_MUTABLE_GETTER(type, name) \
    [[nodiscard]] inline type *mutable_##name() { return &name##_; }

#define DEF_VAL_SETTER(type, name) \
    inline void set_##name(const type &value) { name##_ = value; }

#define DEF_PTR_GETTER(type, name) \
    [[nodiscard]] inline type *name() const { return name##_; }

#define DEF_PTR_SETTER(type, name) \
    inline void set_##name(type *value) { name##_ = value; }

#define DEF_PTR_GETTER_NOTNULL(type, name) \
    [[nodiscard]] inline type *name() const { return DCHECK_NOTNULL(name##_); }

#define DEF_PTR_SETTER_NOTNULL(type, name) \
    inline void set_##name(type *value) { name##_ = DCHECK_NOTNULL(value); }

// Unittest Uilts
#define FRIEND_UNITTEST_CASE(test_name, test_case) \
    friend class test_name##_##test_case##_Test


enum Initializer {
    LAZY_INSTANCE_INITIALIZER,
    ON_EXIT_SCOPE_INITIALIZER
};

using Byte = uint8_t;
using Address = Byte *;

static constexpr int kPointerSize = sizeof(void *);
static constexpr int kPointerShift = 3;

namespace base {
static constexpr int kKB = 1024;
static constexpr int kMB = 1024 * kKB;
static constexpr int kGB = 1024 * kMB;

} // namespace base

} // namespace yalx

#endif // YALX_BASE_BASE_H_
