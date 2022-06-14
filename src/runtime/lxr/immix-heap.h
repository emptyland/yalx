#pragma once
#ifndef YALX_RUNTIME_LXR_IMMIX_HEAP_H_
#define YALX_RUNTIME_LXR_IMMIX_HEAP_H_

#include <pthread.h>

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
}; // struct lxr_immix_heap

int lxr_init_immix_heap(struct lxr_immix_heap *immix, int max_tls_blocks);
void lxr_free_immix_heap(struct lxr_immix_heap *immix);

int lxr_thread_init(struct lxr_immix_heap *immix);
void lxr_thread_finalize(struct lxr_immix_heap *immix);

void *lxr_allocate(struct lxr_immix_heap *immix, size_t n);
void *lxr_allocate_large(struct lxr_immix_heap *immix, size_t n);

#endif // YALX_RUNTIME_LXR_IMMIX_HEAP_H_
