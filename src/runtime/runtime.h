#pragma once
#ifndef YALX_RUNTIME_RUNTIME_H_
#define YALX_RUNTIME_RUNTIME_H_

#include "runtime/macros.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#define _Atomic
#else
#include <stdatomic.h>
#endif

#ifdef arraysize
#undef arraysize
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

#define YALX_MAGIC_NUMBER1 1347046214
#define YALX_MAGIC_NUMBER2 1465142347
#define YALX_MAGIC_NUMBER3 1195658056


#if defined(YALX_ARCH_X64) || defined(YALX_ARCH_ARM64)
#define STACK_SLOT_ALIGNMENT 4
#define STACK_SIZE_ALIGNMENT 16
#else
#error not suppport it
#endif

#define ANY_CLASS_NAME       "yalx/lang:lang.Any"
#define STRING_CLASS_NAME    "yalx/lang:lang.String"
#define BAD_CASTING_EXCEPTION_CLASS_NAME "yalx/lang:lang.BadCastingException"
#define ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION_CLASS_NAME "yalx/lang:lang.ArrayIndexOutOfBoundsException"
#define EXCEPTION_CLASS_NAME "yalx/lang:lang.Exception"
#define THROWABLE_CLASS_NAME "yalx/lang:lang.Throwable"
#define BACKTRACE_FRAME_CLASS_NAME "yalx/lang:lang.BacktraceFrame"


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

struct yalx_value_any;
struct yalx_class;
struct backtrace_frame;

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

char *yalx_symbol_mangle(const char *const plain_name, const char *postfix);
size_t yalx_symbol_demangle_on_place(char symbol[], size_t size);

int yalx_name_symbolize(const char *const plain_name, char symbol[], size_t size);

void die(const char *fmt, ...);

void dbg_class_output(const struct yalx_class *klass);

struct yalx_returning_vals {
    struct yalx_returning_vals *prev;
    address_t fun_addr;
    u16_t total_size;
    u16_t allocated;
    u32_t offset;
    address_t buf;
    u8_t inline_buf[16];
};

int yalx_enter_returning_scope(struct yalx_returning_vals *state, size_t reserved_size, address_t fun_addr);
int yalx_exit_returning_scope(struct yalx_returning_vals *state);

void *yalx_return_reserved(size_t n);

static inline int yalx_return_i32(i32_t value) {
    i32_t *place = (i32_t *)yalx_return_reserved(sizeof(value));
    if (!place) {
        return -1;
    }
    *place = value;
    return 0;
}

static inline int yalx_return_u32(u32_t value) {
    u32_t *place = (u32_t *)yalx_return_reserved(sizeof(value));
    if (!place) {
        return -1;
    }
    *place = value;
    return 0;
}

int yalx_return_cstring(const char *z, size_t n);

static inline int yalx_return_ref(struct yalx_value_any *ref) {
    struct yalx_value_any **place = (struct yalx_value_any **)yalx_return_reserved(sizeof(ref));
    if (!place) {
        return -1;
    }
    *place = ref;
    return 0;
}

static inline int yalx_return(const void *const p, size_t n) {
    void *place = (u32_t *)yalx_return_reserved(n);
    if (!place) {
        return -1;
    }
    memcpy(place, p, n);
    return 0;
}

void *yalx_return_reserved_do(struct yalx_returning_vals *state, size_t n);

const struct yalx_class *yalx_find_class(const char *plain_name);

struct backtrace_frame **yalx_unwind(size_t *depth, int dummy);

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
void call0_returning_vals(void *returning_vals, size_t size_in_bytes, void *yalx_fun);
void call1_returning_vals(void *returning_vals, size_t size_in_bytes, void *yalx_fun, intptr_t arg0);

struct pkg_global_slots {
    size_t size_in_bytes;
    address_t slots;
    size_t mark_size;
    int    marks[0];
};

// runtime libs called by generated code
void pkg_init_once(void *init_fun, const char *plain_name);
int pkg_initialized_count(void);
int pkg_has_initialized(const char *plain_name);

const struct pkg_global_slots *pkg_get_global_slots(const char *plain_name);

// put field for post write barrier
void put_field(struct yalx_value_any **address, struct yalx_value_any *field);

void put_field_chunk(struct yalx_value_any *host, address_t address, address_t field);

void put_field_chunk_by_index(struct yalx_value_any *host, int index_of_field, address_t field);

// lazy load object
struct yalx_value_any *lazy_load_object(struct yalx_value_any * _Atomic *address, const struct yalx_class *clazz);

void associate_stub_returning_vals(struct yalx_returning_vals *state,
                                   address_t returning_addr,
                                   size_t reserved_size,
                                   address_t fun_addr);
void *reserve_handle_returning_vals(u32_t size);

struct yalx_value_any *heap_alloc(const struct yalx_class *clazz);

struct yalx_value_array_header *array_alloc(const struct yalx_class *element_ty, void *elements, int nitems);

struct coroutine *current_root(void);

void throw_it(struct yalx_value_any *exception);

u8_t is_instance_of(struct yalx_value_any *host, const struct yalx_class *for_test);

struct yalx_value_any *ref_asserted_to(struct yalx_value_any *from, const struct yalx_class *clazz);

// generated entry symbol: main:main.main(): unit
void y2zmain_main(void);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_RUNTIME_H_
