#pragma once
#ifndef YALX_RUNTIME_LXR_BLOCK_H_
#define YALX_RUNTIME_LXR_BLOCK_H_

#include "runtime/locks.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define LXR_NORMAL_BLOCK_SHIFT 20
#define LXR_NORMAL_BLOCK_SIZE (1u << LXR_NORMAL_BLOCK_SHIFT)
#define LXR_BLOCK_MAX_REGIONS 7
#define LXR_BLOCK_BITMAP_LEN (LXR_NORMAL_BLOCK_SIZE >> 7)
#define LXR_BLOCK_MIN_ALIGMENT 8
#define LXR_BLOCK_MASK ((1ull << LXR_NORMAL_BLOCK_SHIFT) - 1)

#define LXR_LARGE_BLOCK_THRESHOLD_SIZE 4096

struct lxr_block_chunk {
    struct lxr_block_chunk *next;
    size_t size;
}; // struct lxr_block_entry

struct lxr_block_header {
    struct lxr_block_header *next;
    struct lxr_block_header *prev;
    // [0] 0, 16 bytes
    // [1] 32 bytes
    // [2] 64 bytes
    // [3] 128 bytes
    // [4] 256 bytes
    // [5] 512 bytes
    // [6] > 1024 bytes
    struct lxr_block_chunk *regions[LXR_BLOCK_MAX_REGIONS];
    struct yalx_spin_lock mutex;
    uint32_t bitmap[LXR_BLOCK_BITMAP_LEN];
}; // struct lxr_block_header

struct lxr_large_header {
    struct lxr_large_header *next;
    struct lxr_large_header *prev;
    size_t size_in_bytes;
};

extern struct lxr_block_header *lxr_new_normal_block(void);
void lxr_delete_block(struct lxr_block_header *block);

struct lxr_large_header *lxr_new_large_block(size_t n);
void lxr_delete_large(struct lxr_large_header *block);

// 16,0|32,1|64,2|128,3|512,4|1024,5
void *lxr_block_allocate(struct lxr_block_header *const block, const size_t size, const size_t aligment);

void lxr_block_free(struct lxr_block_header *const block, void *chunk);

#define lxr_owns_block(chunk) ((struct lxr_block_header *)(((uintptr_t)chunk) & ~LXR_BLOCK_MASK))

size_t lxr_block_marked_size(struct lxr_block_header *const block, void *chunk);


#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_LXR_BLOCK_H_
