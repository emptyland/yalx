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

static inline int get_fit_region_shift(size_t size) {
    int shift = approximate_log2(size);
    if (shift < 4) {
        shift = 4;
    }
    if (shift >= LXR_BLOCK_MAX_REGIONS) {
        shift = LXR_BLOCK_MAX_REGIONS - 1;
    }
    return shift;
}

static inline void link_to_fit_region(struct lxr_block_header *const block, struct lxr_block_chunk *const chunk) {
    int shift = get_fit_region_shift(chunk->size);
    chunk->next = block->regions[shift];
    block->regions[shift] = chunk;
}

// block->bitmap[(shift + 31) / 32] |= (1u << (shift % 32));
#define set_bit(block, shift)   (block)->bitmap[(shift + 31) >> 5] |= (1u << (shift & 0x1fu))
#define clear_bit(block, shift) (block)->bitmap[(shift + 31) >> 5] &= ~(1u << (shift & 0x1fu))
#define test_bit(block, shift)  (block->bitmap[(shift + 31) >> 5] & (1u << (shift & 0x1fu)))

static inline void mark_allocated(struct lxr_block_header *const block, address_t chunk, size_t size) {
    ptrdiff_t offset = chunk - (address_t)block;
    assert(offset > 0);
    assert(offset % 4 == 0);
    size = ROUND_UP(size, 4);
    
    // offset / 4
    int shift = offset >> 2;
    set_bit(block, shift);
    shift = (offset + size) >> 2;
    set_bit(block, shift);
}

static inline size_t mark_freed(struct lxr_block_header *const block, address_t chunk) {
    ptrdiff_t offset = chunk - (address_t)block;
    assert(offset > 0);
    assert(offset % 4 == 0);
    
    // offset / 4
    int shift = offset >> 2;
    assert(test_bit(block, shift));
    for (int i = shift + 1; i < (shift + 4096 / 4); i++) {
        if ((block->bitmap[(i + 31) >> 5] != 0) && test_bit(block, i)) {
            clear_bit(block, shift);
            clear_bit(block, i);
            return (i << 2); // i * 4;
        }
    }
    return 0;
}

static inline size_t get_marked_size(struct lxr_block_header *const block, address_t chunk) {
    ptrdiff_t offset = chunk - (address_t)block;
    assert(offset > 0);
    assert(offset % 4 == 0);
    
    // offset / 4
    int shift = offset >> 2;
    assert(test_bit(block, shift));
    for (int i = shift + 1; i < (4096 / 4); i++) {
        if (test_bit(block, i)) {
            return (i << 2); // i * 4;
        }
    }
    return 0;
}

struct lxr_block_header *lxr_new_normal_block(const uint32_t *offset_of_logging) {
    struct lxr_block_header *block = (struct lxr_block_header *)aligned_page_allocate(LXR_NORMAL_BLOCK_SIZE,
                                                                                      LXR_NORMAL_BLOCK_SIZE);
    if (!block) {
        return NULL;
    }
    block->offset_of_logging_bits = (address_t)offset_of_logging - (address_t)block;
    block->next = block;
    block->prev = block;
    
    memset(block->regions, 0, sizeof(struct lxr_block_chunk *) * LXR_BLOCK_MAX_REGIONS);
    address_t available = (address_t)ROUND_UP((uintptr_t)(block + 1), 4);
    struct lxr_block_chunk *const chunk = (struct lxr_block_chunk *)(available);
    chunk->next = NULL;
    chunk->size = LXR_NORMAL_BLOCK_SIZE - sizeof(struct lxr_block_header);
    block->regions[LXR_BLOCK_MAX_REGIONS - 1] = chunk;

    memset(block->bitmap, 0, LXR_BLOCK_BITMAP_LEN * sizeof(uint32_t));
    return block;
}


void *lxr_block_allocate(struct lxr_block_header *const block, const size_t size, const size_t aligment) {
    assert(aligment >= 8);
    assert(aligment % 2 == 0);
    const size_t request_size = ROUND_UP(size, aligment);
    assert(request_size < 4096);
    if (request_size == 0) {
        return NULL;
    }
    int shift = get_fit_region_shift(request_size);
    struct lxr_block_chunk **location = NULL;
    while (shift < LXR_BLOCK_MAX_REGIONS) {
        location = &block->regions[shift++];
        if (*location != NULL) {
            break;
        }
    }
    if (!*location) {
        return NULL;
    }
    struct lxr_block_chunk *chunk = *location;
    assert(chunk->size >= request_size);
    *location = chunk->next;

    const size_t remain_size = chunk->size - request_size;
    if (remain_size > sizeof(struct lxr_block_chunk)) {
        struct lxr_block_chunk *remain = (struct lxr_block_chunk *)((address_t)chunk + request_size);
        remain->next = NULL;
        remain->size = remain_size;
        link_to_fit_region(block, remain);
    }
    

    address_t result = (address_t)chunk;
    mark_allocated(block, result, size);
    dbg_init_zag(result, size);
    return result;
}

void lxr_block_free(struct lxr_block_header *const block, void *chunk) {
    if (!chunk) {
        return; // Ignore null pointer
    }
    address_t addr = (address_t)chunk;
    assert(addr > (address_t)block && "invalid chunk address");
    assert(addr < (address_t)block + LXR_NORMAL_BLOCK_SIZE);
    
    size_t size = mark_freed(block, addr);
    assert(size > 0);
    ((struct lxr_block_chunk *)chunk)->size = size;
    ((struct lxr_block_chunk *)chunk)->next = NULL;
    link_to_fit_region(block, chunk);
    dbg_free_zag(chunk, size);
}


void lxr_free_block(struct lxr_block_header *block) {
    munmap((void *)block, LXR_NORMAL_BLOCK_SIZE);
}
