#include "runtime/lxr/immix-heap.h"
#include "runtime/lxr/block.h"
#include "runtime/runtime.h"
#include "runtime/checking.h"
#include "runtime/macros.h"
#include <string.h>

_Thread_local struct lxr_block_header *tls_block = NULL;

int lxr_init_immix_heap(struct lxr_immix_heap *immix, int max_tls_blocks) {
    memset(immix, 0, sizeof(*immix));
    DCHECK(max_tls_blocks >= 0 && max_tls_blocks < 30);
    immix->max_tls_blocks = max_tls_blocks;
    immix->n_tls_blocks = 0;
    immix->dummy = (struct lxr_block_header *)immix->stub0;
    immix->dummy->next = immix->dummy;
    immix->dummy->prev = immix->dummy;
    immix->large = (struct lxr_block_header *)immix->stub1;
    immix->large->next = immix->large;
    immix->large->prev = immix->large;
    pthread_mutex_init(&immix->mutex, NULL);
    return 0;
}

void lxr_free_immix_heap(struct lxr_immix_heap *immix) {
    while (!QUEUE_EMPTY(immix->dummy)) {
        struct lxr_block_header *node = immix->dummy->next;
        QUEUE_REMOVE(node);
        lxr_delete_block(node);
    }
    while (!QUEUE_EMPTY(immix->large)) {
        struct lxr_large_header *node = immix->large->next;
        QUEUE_REMOVE(node);
        lxr_delete_large(node);
    }
    pthread_mutex_destroy(&immix->mutex);
}

static int mark_addr_card(struct lxr_immix_heap *immix, void *addr, uintptr_t flags) {
    uintptr_t tagged = (uintptr_t)addr;
    int index = ((tagged & ~LXR_BLOCK_MASK) >> LXR_NORMAL_BLOCK_SHIFT) & ADDR_CARD_MASK;
    DCHECK(index >= 0 && index < ADDR_CARD_SIZE);
    DCHECK(flags < 4); // max 2 bits
    
    void *expected = NULL;
    int ok = atomic_compare_exchange_strong(&immix->addr_card_table[index], &expected, tagged | flags);
    DCHECK(ok);
    return ok;
}

static int is_addr_marked(struct lxr_immix_heap *immix, void *addr, uintptr_t *flags) {
    uintptr_t tagged = (uintptr_t)addr;
    int index = ((tagged & ~LXR_BLOCK_MASK) >> LXR_NORMAL_BLOCK_SHIFT) & ADDR_CARD_MASK;
    DCHECK(index >= 0 && index < ADDR_CARD_SIZE);
    
    void *card = atomic_load_explicit(&immix->addr_card_table[index], memory_order_relaxed);
    if (!card) {
        return 0;
    }
    if (flags) {
        *flags = (uintptr_t)card & 0x3;
    }
    card = (void *)((uintptr_t)card & ~0x3ull);
    return card == addr;
}

int lxr_thread_enter(struct lxr_immix_heap *immix) {
    pthread_mutex_lock(&immix->mutex);
    
    if (immix->n_tls_blocks >= immix->max_tls_blocks) {
        pthread_mutex_unlock(&immix->mutex);
        return -1;
    }
    
    tls_block = lxr_new_normal_block(NULL);
    if (!tls_block) {
        pthread_mutex_unlock(&immix->mutex);
        return -1;
    }
    mark_addr_card(immix, tls_block, 0/*flags*/);
    
    immix->n_tls_blocks++;
    
    pthread_mutex_unlock(&immix->mutex);
    return 0;
}

void lxr_thread_exit(struct lxr_immix_heap *immix) {
    //DCHECK(tls_block != NULL);
    if (!tls_block) {
        return;
    }
    pthread_mutex_lock(&immix->mutex);

    QUEUE_INSERT_TAIL(immix->dummy, tls_block);
    
    tls_block = NULL;
    immix->n_tls_blocks--;
    
    pthread_mutex_unlock(&immix->mutex);
}


void *lxr_allocate(struct lxr_immix_heap *immix, size_t n) {
    size_t request_size = ROUND_UP(n, 8);
    if (request_size >= LXR_LARGE_BLOCK_THRESHOLD_SIZE) { // large
        return lxr_allocate_large(immix, request_size);
    }
    
    // Fast path
    if (tls_block) {
        void *chunk = lxr_block_allocate(tls_block, request_size, 8);
        if (chunk) {
            return chunk;
        }
    }
    return lxr_allocate_fallback(immix, request_size);
}

void *lxr_allocate_fallback(struct lxr_immix_heap *immix, size_t n) {
    void *chunk = NULL;
    pthread_mutex_lock(&immix->mutex);
    for (struct lxr_block_header *b = immix->dummy->next; b != immix->dummy; b = b->next) {
        chunk = lxr_block_allocate(b, n, 8);
        if (chunk) {
            goto done;
        }
    }

    struct lxr_block_header *block = lxr_new_normal_block(NULL);
    if (!block) {
        goto done;
    }
    mark_addr_card(immix, block, 0/*flags*/);
    QUEUE_INSERT_HEAD(immix->dummy, block);
    chunk = lxr_block_allocate(block, n, 8);
    
done:
    pthread_mutex_unlock(&immix->mutex);
    if (chunk) {
        dbg_init_zag(chunk, n);
    }
    return chunk;
}


void *lxr_allocate_large(struct lxr_immix_heap *immix, size_t n) {
    size_t request_size = ROUND_UP(n, 8);
    struct lxr_large_header *block = lxr_new_large_block(request_size);
    if (!block) {
        return NULL;
    }
    mark_addr_card(immix, block, 1/*flags*/);
    
    pthread_mutex_lock(&immix->mutex);
    QUEUE_INSERT_TAIL(immix->large, block);
    pthread_mutex_unlock(&immix->mutex);
    
    void *chunk = (void *)(block + 1);
    dbg_init_zag(chunk, n);
    return chunk;
}


struct lxr_addr_test_result lxr_test_addr(struct lxr_immix_heap *immix, void *addr) {
    struct lxr_addr_test_result rs;
    rs.block.large = NULL;
    rs.kind = LXR_ADDR_INVALID;
    
    void *maybe_header = (void *)((uintptr_t)addr & ~LXR_BLOCK_MASK);
    uintptr_t flags = 0;
    if (is_addr_marked(immix, maybe_header, &flags)) {
        if (flags) { // is large
            rs.block.large = (struct lxr_large_header *)maybe_header;
            rs.kind = LXR_ADDR_LARGE;
        } else {
            rs.block.normal = (struct lxr_block_header *)maybe_header;
            rs.kind = LXR_ADDR_CHUNK;
        }
    }
    return rs;
}
