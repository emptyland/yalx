#pragma once
#ifndef YALX_RUNTIME_RUNTIME_H_
#define YALX_RUNTIME_RUNTIME_H_

#include "runtime/macros.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define IS_POWER_OF_TWO(x) (((x) & ((x) - 1)) == 0)

#define ROUND_DOWN(x, m)   ((x) & -(m))
#define ROUND_UP(x, m)     ROUND_DOWN(x + m - 1, m)

#define KB 1024
#define MB (KB * 1024)
#define GB (MB * 1024)
#define TB (GB * 1024)

#define MEM_INIT_ZAG 0xcccccccc
#define MEM_FREE_ZAG 0xfeedfeed

// Yalx internal primitive types:
typedef int8_t   i8_t;
typedef uint8_t  u8_t;

typedef int16_t  i16_t;
typedef uint16_t u16_t;

typedef int32_t  i32_t;
typedef uint32_t u32_t;

typedef int64_t  i64_t;
typedef uint64_t u64_t;

typedef float    f32_t;
typedef float    f64_t;

typedef uint8_t * address_t;

// Raw string type:
struct yalx_str {
    const char *z;
    int         n;
};

#define YALX_STR(s) { .z = (s), .n = sizeof(s) - 1 }


// Version of yalx
extern const struct yalx_str yalx_version;

// Number of system cpus.
extern int ncpus;

// Size in bytes for system memory page.
extern int os_page_size;


int yalx_runtime_init(void);

void *fill_memory_zag(void *chunk, size_t n, uint32_t zag);

#ifdef NDEBUG
static inline void dbg_init_zag(void *chunk, size_t n) {
    USE(chunk);
    USE(n);
}
static inline void dbg_free_zag(void *chunk, size_t n) {
    USE(chunk);
    USE(n);
}
#else // NDEBUG
static inline void dbg_init_zag(void *chunk, size_t n) { fill_memory_zag(chunk, n, MEM_INIT_ZAG); }
static inline void dbg_free_zag(void *chunk, size_t n) { fill_memory_zag(chunk, n, MEM_FREE_ZAG); }
#endif

// implements in bootstrap-[Arch].s
int entry(void);
int asm_stub1(int, int);
int asm_stub2(const struct yalx_str *);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_RUNTIME_H_
