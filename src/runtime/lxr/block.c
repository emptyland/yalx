#include "runtime/lxr/block.h"
#include "runtime/runtime.h"

#include <sys/mman.h>
#include <assert.h>

static inline int approximate_log2(size_t n) {
    int j = 0;
    for (size_t i = 1; i < n; i <<= 1) {
        j++;
    }
    return j;
}

static void *aligned_page_allocate(const size_t size, const size_t alignment) {
    const size_t page_size = os_page_size;
    size_t request_size = ROUND_UP(size + (alignment - page_size), page_size);
    void *result = mmap(NULL, request_size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
    if (!result) {
        return NULL;
    }
    
    address_t base = (address_t)result;
    address_t aligned_base = (address_t)ROUND_UP((uintptr_t)base, alignment);
    if (aligned_base != base) {
        assert(base < aligned_base);
        ptrdiff_t prefix_size = aligned_base - base;
        munmap(base, prefix_size);
        request_size -= prefix_size;
    }
    
    if (size != request_size) {
        assert(size < request_size);
        const size_t suffix_size = request_size - size;
        munmap(aligned_base + size, suffix_size);
        request_size -= suffix_size;
    }
    
    assert(request_size == size);
    return aligned_base;
}

struct lxr_block_header *lxr_new_normal_block(const uint32_t *associated_logging_bits) {
    struct lxr_block_header *block = (struct lxr_block_header *)aligned_page_allocate(LXR_NORMAL_BLOCK_SIZE,
                                                                                      LXR_NORMAL_BLOCK_SIZE);
    if (!block) {
        return NULL;
    }
    block->offset_of_logging_bits = (address_t)associated_logging_bits - (address_t)block;
    block->next = block;
    block->prev = block;
    // TODO:
    return block;
}


void lxr_free_block(struct lxr_block_header *block) {
    munmap((void *)block, LXR_NORMAL_BLOCK_SIZE);
}
