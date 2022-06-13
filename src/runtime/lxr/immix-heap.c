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
    pthread_mutex_destroy(&immix->mutex);
}

int lxr_thread_init(struct lxr_immix_heap *immix) {
    pthread_mutex_lock(&immix->mutex);
    
    if (immix->n_tls_blocks >= immix->max_tls_blocks) {
        pthread_mutex_unlock(&immix->mutex);
        return -1;
    }
    
    tls_block = lxr_new_normal_block(NULL); // TODO:
    if (!tls_block) {
        pthread_mutex_unlock(&immix->mutex);
        return -1;
    }
    
    immix->n_tls_blocks++;
    
    pthread_mutex_unlock(&immix->mutex);
    return 0;
}

void lxr_thread_finalize(struct lxr_immix_heap *immix) {
    DCHECK(tls_block != NULL);
    pthread_mutex_lock(&immix->mutex);

    QUEUE_INSERT_TAIL(immix->dummy, tls_block);
    
    tls_block = NULL;
    immix->n_tls_blocks--;
    
    pthread_mutex_unlock(&immix->mutex);
}


void *lxr_allocate(struct lxr_immix_heap *immix, size_t n) {
    size_t request_size = ROUND_UP(n, 8);
    if (request_size >= 4096) { // large
        
    }
}


void *lxr_allocate_large(struct lxr_immix_heap *immix, size_t n) {
    pthread_mutex_lock(&immix->mutex);
    
    
    pthread_mutex_unlock(&immix->mutex);
}
