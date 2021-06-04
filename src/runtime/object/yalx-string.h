#pragma once
#ifndef YALX_RUNTIME_OBJECT_STRING_H_
#define YALX_RUNTIME_OBJECT_STRING_H_

#include "runtime/object/any.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IN_POOL_STR_LEN 64

struct heap;

struct yalx_value_str {
    YALX_VALUE_HEADER;
    u32_t hash_code;
    u32_t len;
    char  bytes[0];
}; // struct yalx_value_str

// New string object: short string should be in pool
struct yalx_value_str *yalx_new_string(struct heap *heap, const char *z, size_t n);

// Directly new string object
struct yalx_value_str *yalx_new_string_direct(struct heap *heap, const char *z, size_t n);

static inline size_t yalx_reserve_string_size(const char *z, size_t n) {
    if (!z || !n) {
        return sizeof(struct yalx_value_str);
    }
    return sizeof(struct yalx_value_str) + n + 1; // 1 for term zero
}

static inline u32_t yalx_str_hash(const char *z, size_t n) {
    u32_t hash = 1315423911;
    for (const char *s = z; s < z + n; s++) {
        hash ^= ((hash << 5) + (*s) + (hash >> 2));
    }
    return hash;
}


#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_OBJECT_STRING_H_
