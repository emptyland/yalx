#pragma once
#ifndef YALX_RUNTIME_HEAP_HEAP_H_
#define YALX_RUNTIME_HEAP_HEAP_H_

#include "runtime/runtime.h"
#include <pthread.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct yalx_value_any;
struct yalx_value_str;
struct yalx_class;

#define KPOOL_STRIPES_SIZE  16
#define KPOOL_REHASH_FACTOR 0.8

enum allocate_status {
    ALLOCATE_OK,
    ALLOCATE_NOTHING,
    // TODO:
    ALLOCATE_NOT_ENOUGH_MEMORY,
    ALLOCATE_NO_OS_MEMORY,
};

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
    struct yalx_value_str *value;
}; // struct string_pool_entry

struct string_pool {
    struct string_pool_entry *slots;
    int slots_shift;
    int n_entries;
    pthread_mutex_t mutex;
}; // struct string_pool

struct heap {
    // 1 pad allocation memory pool.
    // Only for no-gc model.
    struct one_time_memory_pool one_time_pool;
    struct string_pool kpool_stripes[KPOOL_STRIPES_SIZE];
    
    pthread_mutex_t mutex;
    struct allocate_result (*allocate)(struct heap *, size_t, u32_t);
    void (*finalize)(struct heap *);
}; // struct heap

extern struct heap heap;

int yalx_init_heap(struct heap *heap);
void yalx_free_heap(struct heap *heap);

// Find a string from string-pool
// If string value exists, return pointer of it.
struct string_pool_entry *yalx_ensure_space_kpool(struct heap *heap, const char *z, size_t n);

void string_pool_init(struct string_pool *pool, int slots_shift);
void string_pool_free(struct string_pool *pool);

struct string_pool_entry *string_pool_ensure_space(struct string_pool *pool, const char *z, size_t n);

void string_pool_rehash(struct string_pool *pool, int slot_shift);

struct allocate_result yalx_heap_allocate(struct heap *heap, const struct yalx_class *klass, size_t size, u32_t flags);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_HEAP_HEAP_H_
