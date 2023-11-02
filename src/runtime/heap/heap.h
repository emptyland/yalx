#pragma once
#ifndef YALX_RUNTIME_HEAP_HEAP_H_
#define YALX_RUNTIME_HEAP_HEAP_H_

#include "runtime/locks.h"
#include "runtime/runtime.h"
#include "runtime/locks.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct yalx_value_number_l;
struct yalx_value_number_w;
struct yalx_value_any;
struct yalx_value_str;
struct yalx_class;
struct yalx_root_visitor;

#define KPOOL_STRIPES_SIZE  16
#define KPOOL_REHASH_FACTOR 0.8

#define OBJECT_ALIGNMENT_IN_BYTES (sizeof(uintptr_t))

enum allocate_status {
    ALLOCATE_OK,
    ALLOCATE_NOTHING,
    // TODO:
    ALLOCATE_NOT_ENOUGH_MEMORY,
    ALLOCATE_NO_OS_MEMORY,
};

typedef enum gc_algorithm {
    GC_NONE, // No gc
    GC_LXR,  // LXR gc algorithm
    GC_YGC,  // YGC pauseless gc algorithm
} gc_t;

struct allocate_result {
    struct yalx_value_any *object;
    enum allocate_status status;
}; // struct allocate_result


struct one_time_memory_pool {
    address_t chunk;
    address_t free;
    size_t size;
}; // struct one_time_memory_pool

struct string_pool_entry {
    QUEUE_HEADER(struct string_pool_entry);
    struct yalx_value_str *value; // NOTICE: [strong ref]
}; // struct string_pool_entry

struct string_pool {
    struct string_pool_entry *slots;
    int slots_shift;
    int n_entries;
    struct yalx_mutex mutex;
}; // struct string_pool


// [strong ref] For number boxing:
struct boxing_number_pool {
    struct yalx_value_number_l *bool_values[2]; // true and false
    struct yalx_value_number_l *i8_values[256]; // all i8
    struct yalx_value_number_l *u8_values[256]; // all u8
    struct yalx_value_number_l *i16_values[201]; // -100~100 i16
    struct yalx_value_number_l *u16_values[201]; // 0~200 u16
    struct yalx_value_number_l *i32_values[201]; // -100~100 i32
    struct yalx_value_number_l *u32_values[201]; // 0~200 u32
    struct yalx_value_number_w *i64_values[201]; // -100~100 i32
    struct yalx_value_number_w *u64_values[201]; // 0~200 u32
    struct yalx_value_number_l *f32_values[3]; // -1.0 0 1.0
    struct yalx_value_number_w *f64_values[3]; // -1.0 0 1.0
}; // struct number_pool

struct heap;
struct yalx_os_thread;

struct barrier_set {
    struct yalx_value_any *(*prefix_load_barrier)(struct heap *, struct yalx_value_any *, struct yalx_value_any *_Atomic volatile *);
    void (*prefix_write_barrier)(struct heap *, struct yalx_value_any *, struct yalx_value_any *);
    void (*prefix_write_barrier_batch)(struct heap *, struct yalx_value_any *, struct yalx_value_any **, size_t);
    void (*post_write_barrier)(struct heap *, struct yalx_value_any **, struct yalx_value_any *);
    void (*post_write_barrier_batch)(struct heap *, struct yalx_value_any **, struct yalx_value_any **, size_t);
    void (*init_write_barrier)(struct heap *, struct yalx_value_any **);
    void (*init_write_barrier_batch)(struct heap *, struct yalx_value_any **, size_t);
};

struct heap {
    struct barrier_set barrier_ops;
    
    struct string_pool kpool_stripes[KPOOL_STRIPES_SIZE];
    struct boxing_number_pool fast_boxing_numbers;

    struct yalx_mutex mutex;
    struct allocate_result (*allocate)(struct heap *, size_t, u32_t);
    int (*is_in)(const struct heap *, uintptr_t);
    void (*finalize)(struct heap *);
    void (*thread_enter)(struct heap *, struct yalx_os_thread *);
    void (*thread_exit)(struct heap *, struct yalx_os_thread *);
    
    gc_t gc;
}; // struct heap

struct collected_statistics {
    double pause_mills;
    double total_mills;
    size_t collected_rss_in_bytes;
    size_t collected_in_bytes;
};

extern struct heap *heap;

int yalx_init_heap(gc_t gc, size_t max_heap_in_bytes, struct heap **receiver);
void yalx_free_heap(struct heap *);

// For GC root marking~
void yalx_heap_visit_root(struct heap *h, struct yalx_root_visitor *visitor);

// Find a string from string-pool
// If string value exists, return pointer of it.
struct string_pool_entry *yalx_ensure_space_kpool(struct heap *h, const char *z, size_t n);

void string_pool_init(struct string_pool *pool, int slots_shift);
void string_pool_free(struct string_pool *pool);

struct string_pool_entry *string_pool_ensure_space(struct string_pool *pool, const char *z, size_t n);

void string_pool_rehash(struct string_pool *pool, int slot_shift);

struct allocate_result yalx_heap_allocate(struct heap *h, const struct yalx_class *klass, size_t size, u32_t flags);


//---------------------------------------------------- Barriers --------------------------------------------------------

void init_typing_write_barrier_if_needed(struct heap *h, const struct yalx_class *item, address_t data);
void post_typing_write_barrier_if_needed(struct heap *h, const struct yalx_class *item, address_t location,
                                         address_t data);

static inline void prefix_write_barrier(struct heap *h, struct yalx_value_any *host, struct yalx_value_any *mutator) {
    h->barrier_ops.prefix_write_barrier(h, host, mutator);
}

static inline void prefix_write_barrier_batch(struct heap *h, struct yalx_value_any *host,
                                              struct yalx_value_any **mutators, size_t nitems) {
    h->barrier_ops.prefix_write_barrier_batch(h, host, mutators, nitems);
}

static inline void post_write_barrier(struct heap *h, struct yalx_value_any **field, struct yalx_value_any *mutator) {
    h->barrier_ops.post_write_barrier(h, field, mutator);
}

static inline void post_write_barrier_batch(struct heap *h, struct yalx_value_any **field,
                                            struct yalx_value_any **mutators, size_t nitems) {
    h->barrier_ops.post_write_barrier_batch(h, field, mutators, nitems);
}

static inline void init_write_barrier(struct heap *h, struct yalx_value_any **field) {
    h->barrier_ops.init_write_barrier(h, field);
}

static inline void init_write_barrier_batch(struct heap *h, struct yalx_value_any **field, size_t nitems) {
    h->barrier_ops.init_write_barrier_batch(h, field, nitems);
}

#define yalx_bool_value(b) (heap->fast_boxing_numbers.bool_values[(b) ? 1 : 0])
#define yalx_true_value() (heap->fast_boxing_numbers.bool_values[1])
#define yalx_false_value() (heap->fast_boxing_numbers.bool_values[0])

static inline struct yalx_value_number_l *yalx_i8_value(i8_t value) {
    int i = (int)value + 128;
    return heap->fast_boxing_numbers.i8_values[i];
}

static inline struct yalx_value_number_l *
yalx_u8_value(u8_t value) { return heap->fast_boxing_numbers.u8_values[value]; }

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_HEAP_HEAP_H_
