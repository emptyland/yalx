#pragma once
#ifndef YALX_RUNTIME_UTILS_H
#define YALX_RUNTIME_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "runtime/macros.h"
#include <stddef.h>
#include <stdint.h>

double yalx_current_mills_in_precision();

static inline int yalx_log2(uintptr_t n) {
    int x = 1, i = 0;
    for (; x < n; x <<= 1) {
        i++;
    }
    return i;
}

static inline int yalx_u32_is_power_of_2(uint32_t x) {
    return (x > 0) && ((x & (x - 1)) == 0);
}

static inline int yalx_u64_is_power_of_2(uint64_t x) {
    return (x > 0) && ((x & (x - 1)) == 0);
}

int yalx_count_leading_zeros_32(uint32_t x);

static inline int yalx_count_leading_zeros_64(uint64_t x) {
    if ((x & 0xFFFFFFFF00000000ULL) == 0) {
        return 32 + yalx_count_leading_zeros_32((uint32_t) x);
    } else {
        x = (x & 0xFFFFFFFF00000000ULL) >> 32;
        return yalx_count_leading_zeros_32((uint32_t) x);
    }
}

size_t yalx_round_up_power_of_2(size_t input);

static inline uint32_t yalx_hash_uint32_to_uint32(uint32_t key) {
    key = ~key + (key << 15);
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057;
    key = key ^ (key >> 16);
    return key;
}

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_UTILS_H
