#pragma once
#ifndef YALX_RUNTIME_RUNTIME_H_
#define YALX_RUNTIME_RUNTIME_H_

#include "runtime/macros.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define arraysize(a) (sizeof(a)/sizeof(a[0]))

#define IS_POWER_OF_TWO(x) (((x) & ((x) - 1)) == 0)

#define ROUND_DOWN(x, m)   ((x) & -(m))
#define ROUND_UP(x, m)     ROUND_DOWN(x + m - 1, m)

#define MALLOC(t) ((t *)malloc(sizeof(t)))
#define MALLOC_N(t, n) ((t *)malloc(n * sizeof(t)))

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

struct yalx_class;


// Version of yalx
extern const struct yalx_str yalx_version;

// Number of system cpus.
extern int ncpus;

// Size in bytes for system memory page.
extern int os_page_size;


int yalx_runtime_init(void);

void yalx_runtime_eixt(void);

int yalx_rt0(int argc, char *argv[]);

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

void *yalx_zalloc(size_t n);

int yalx_name_symbolize(const char *const plain_name, char symbol[], size_t size);

void die(const char *fmt, ...);

void dbg_class_output(const struct yalx_class *klass);

struct yalx_returning_vals {
    u32_t total_size;
    u32_t offset;
};

int yalx_return_i32(struct yalx_returning_vals *state, i32_t value);
int yalx_return_u32(struct yalx_returning_vals *state, u32_t value);
int yalx_return_cstring(struct yalx_returning_vals *state, const char *const z, size_t n);
int yalx_return(const void *const p, size_t n);


// implements in test-stub-[Arch].s
int asm_stub1(int, int);
int asm_stub2(const struct yalx_str *);
u64_t asm_stub3(void);
int asm_stub4(void);
void *asm_stub5(void);
void *asm_stub6(void);

// implements in boot-[Arch].s
int trampoline(void);
void coroutine_finalize_stub(void);
void call_returning_vals(void *returnning_vals, size_t size_in_bytes, void *yalx_fun);

// runtime libs called by generated code
void pkg_init_once(void *init_fun, const char *const plain_name);
int pkg_initialized_count();
int pkg_has_initialized(const char *const plain_name);

void *reserve_handle_returning_vals(u32_t size);

struct coroutine *current_root();

// generated entry symbol: main:main.main(): unit
void y2zmain_main(void);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_RUNTIME_H_
