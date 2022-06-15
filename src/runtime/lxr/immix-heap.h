#pragma once
#ifndef YALX_RUNTIME_LXR_IMMIX_HEAP_H_
#define YALX_RUNTIME_LXR_IMMIX_HEAP_H_

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ADDR_CARD_SHIFT 12
#define ADDR_CARD_SIZE (1u << ADDR_CARD_SHIFT)
#define ADDR_CARD_MASK ((1ull << ADDR_CARD_SHIFT) - 1)

struct lxr_block_header;
struct lxr_large_header;

struct lxr_immix_heap {
    int max_tls_blocks;
    int n_tls_blocks;
    
    struct lxr_block_header *dummy;
    struct lxr_large_header *large;
    struct lxr_block_header *stub0[2];
    struct lxr_large_header *stub1[2];
    
    pthread_mutex_t mutex;
    
    void *_Atomic addr_card_table[ADDR_CARD_SIZE];
}; // struct lxr_immix_heap

typedef enum {
    LXR_ADDR_INVALID,
    LXR_ADDR_LARGE,
    LXR_ADDR_CHUNK,
} lxr_addr_kind_t;

struct lxr_addr_test_result {
    lxr_addr_kind_t kind;
    union {
        struct lxr_block_header *normal;
        struct lxr_large_header *large;
    } block;
};

int lxr_init_immix_heap(struct lxr_immix_heap *immix, int max_tls_blocks);
void lxr_free_immix_heap(struct lxr_immix_heap *immix);

int lxr_thread_enter(struct lxr_immix_heap *immix);
void lxr_thread_exit(struct lxr_immix_heap *immix);

void *lxr_allocate(struct lxr_immix_heap *immix, size_t n);
void *lxr_allocate_fallback(struct lxr_immix_heap *immix, size_t n);
void *lxr_allocate_large(struct lxr_immix_heap *immix, size_t n);

struct lxr_addr_test_result lxr_test_addr(struct lxr_immix_heap *immix, void *addr);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_LXR_IMMIX_HEAP_H_
