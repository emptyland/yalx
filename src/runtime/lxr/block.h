#pragma once
#ifndef YALX_RUNTIME_LXR_BLOCK_H_
#define YALX_RUNTIME_LXR_BLOCK_H_

#include <stddef.h>
#include <stdint.h>

enum lxr_block_kind {
    LXR_1MB_BLOCK,
    LXR_4MB_BLOCK,
    LXR_10MB_BLOCK,
    LXR_LARGE_BLOCK,
};

//typedef unsigned char *lxr_block_ptr;

#define LXR_NORMAL_BLOCK_SHIFT 20
#define LXR_NORMAL_BLOCK_SIZE (1u << LXR_NORMAL_BLOCK_SHIFT)

struct lxr_block_header {
    struct lxr_block_header *next;
    struct lxr_block_header *prev;
    void *free;
    ptrdiff_t offset_of_logging_bits;
}; // struct lxr_block_header

struct lxr_block_entry {
    void *next;
    size_t size;
}; // struct lxr_block_entry


#define lxr_block_bitmap(block) (uint32_t *)((address_t)(block) + sizeof(struct lxr_block_header))

struct lxr_block_header *lxr_new_normal_block(const uint32_t *associated_logging_bits);

// 16,0|32,1|64,2|128,3|512,4|1024,5
void *lxr_block_allocate(struct lxr_block_header *const block, const size_t size, const size_t aligment);

void lxr_free_block(struct lxr_block_header *block);

#endif // YALX_RUNTIME_LXR_BLOCK_H_
