#pragma once
#ifndef YALX_RUNTIME_LXR_BLOCK_H_
#define YALX_RUNTIME_LXR_BLOCK_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum lxr_block_kind {
    LXR_1MB_BLOCK,
    LXR_4MB_BLOCK,
    LXR_10MB_BLOCK,
    LXR_LARGE_BLOCK,
};

//typedef unsigned char *lxr_block_ptr;

#define LXR_NORMAL_BLOCK_SHIFT 20
#define LXR_NORMAL_BLOCK_SIZE (1u << LXR_NORMAL_BLOCK_SHIFT)
#define LXR_BLOCK_MAX_REGIONS 7
#define LXR_BLOCK_BITMAP_LEN (LXR_NORMAL_BLOCK_SIZE >> 7)


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
    ptrdiff_t offset_of_logging_bits;
    uint32_t bitmap[LXR_BLOCK_BITMAP_LEN];
}; // struct lxr_block_header


struct lxr_block_header *lxr_new_normal_block(const uint32_t *offset_of_bitmap);

// 16,0|32,1|64,2|128,3|512,4|1024,5
void *lxr_block_allocate(struct lxr_block_header *const block, const size_t size, const size_t aligment);

void lxr_block_free(struct lxr_block_header *const block, void *chunk);

void lxr_free_block(struct lxr_block_header *block);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_LXR_BLOCK_H_
