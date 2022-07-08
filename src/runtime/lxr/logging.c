#include "runtime/lxr/logging.h"
#include "runtime/lxr/block.h"
#include "runtime/runtime.h"
#include "runtime/checking.h"
#include "runtime/macros.h"
#include <pthread/sched.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

static const uintptr_t kPendingMask = 1;
static const uintptr_t kCreatedMask = ~kPendingMask;

int lxr_init_fields_logger(struct lxr_fields_logger *logger) {
    static const size_t kStripesSizeInBytes = TOP_STRIPES_SIZE * sizeof(struct lxr_log_stripe *);

    memset(logger, 0, sizeof(*logger));
    logger->top_stripes = (_Atomic(struct lxr_log_stripe *) *)malloc(kStripesSizeInBytes);
    memset(logger->top_stripes, 0, kStripesSizeInBytes);
    lxr_init_log_queue(&logger->decrments);
    lxr_init_log_queue(&logger->modification);
    return 0;
}

void lxr_free_fields_logger(struct lxr_fields_logger *logger) {
    lxr_free_log_queue(&logger->modification);
    lxr_free_log_queue(&logger->decrments);
    for (int i = 0; i < TOP_STRIPES_SIZE; i++) {
        if (!logger->top_stripes[i]) {
            continue;
        }
        
        struct lxr_log_stripe *stripe = logger->top_stripes[i];
        for (int j = 0; j < SECONDARY_STRIPES_SIZE; j++) {
            free(stripe->u.next[j]);
        }
        free(stripe);
    }
    free(logger->top_stripes);
}

static inline struct lxr_log_stripe *load_stripe(struct lxr_log_stripe *_Atomic *slot) {
    int i = 0;
    struct lxr_log_stripe *stripe = NULL;
    do {
        stripe = atomic_load_explicit(slot, memory_order_relaxed);
        if (!stripe) {
            break;;
        }
        if (i++ > 10000) {
            sched_yield();
        }
    } while((uintptr_t)stripe == kPendingMask);
    return stripe;
}

static inline int need_init(struct lxr_log_stripe *_Atomic *slot) {
    struct lxr_log_stripe *expected = NULL;
    if (atomic_compare_exchange_strong(slot, &expected, (struct lxr_log_stripe *)kPendingMask)) {
        return 1;
    }
    while ((uintptr_t)atomic_load_explicit(slot, memory_order_acquire) == kPendingMask) {
        sched_yield();
    }
    return 0;
}

static struct lxr_log_stripe *lazy_init_or_get_top_stripe(struct lxr_fields_logger *logger,
                                                          struct lxr_log_stripe *_Atomic *slot) {
    if (!((uintptr_t)atomic_load_explicit(slot, memory_order_acquire) & kCreatedMask) && need_init(slot)) {
        size_t request_size = sizeof(struct lxr_log_stripe) + (SECONDARY_STRIPES_SIZE) * sizeof(struct lxr_log_stripe *);
        
        struct lxr_log_stripe *stripe = (struct lxr_log_stripe *)malloc(request_size);
        memset(stripe, 0, request_size);
        atomic_fetch_add(&logger->used_memory_in_bytes, request_size);
        atomic_store_explicit(slot, stripe, memory_order_release);
    }
    return atomic_load_explicit(slot, memory_order_relaxed);
}

static struct lxr_log_stripe *lazy_init_or_get_scondary_stripe(struct lxr_fields_logger *logger,
                                                               struct lxr_log_stripe *_Atomic *slot) {
    if (!((uintptr_t)atomic_load_explicit(slot, memory_order_acquire) & kCreatedMask) && need_init(slot)) {
        size_t request_size = sizeof(struct lxr_log_stripe) + (CARD_SIZE / 4) * sizeof(uint8_t);
        
        struct lxr_log_stripe *stripe = (struct lxr_log_stripe *)malloc(request_size);
        memset(stripe, 0, request_size);
        atomic_fetch_add(&logger->used_memory_in_bytes, request_size);
        atomic_store_explicit(slot, stripe, memory_order_release);
    }
    return atomic_load_explicit(slot, memory_order_relaxed);
}

static_assert(SECONDARY_STRIPES_SHIFT + CARD_SHIFT == LXR_NORMAL_BLOCK_SHIFT, "");

int lxr_attempt_to_log(struct lxr_fields_logger *logger, void *address) {
    const uintptr_t tagged = (uintptr_t)address;
    int index = ((tagged & ~LXR_BLOCK_MASK) >> LXR_NORMAL_BLOCK_SHIFT) & TOP_STRIPES_MASK;
    DCHECK(index >= 0 && index < TOP_STRIPES_SIZE);
    struct lxr_log_stripe *stripe = lazy_init_or_get_top_stripe(logger, &logger->top_stripes[index]);
    if (!stripe) {
        return 0;
    }
    
    index = (tagged & LXR_BLOCK_MASK) >> CARD_SHIFT;
    DCHECK(index >= 0 && index < SECONDARY_STRIPES_SIZE);
    stripe = lazy_init_or_get_scondary_stripe(logger, &stripe->u.next[index]);
    if (!stripe) {
        return 0;
    }
    
    DCHECK(tagged % 4 == 0);
    index = (tagged & CARD_MASK) >> 2;
    DCHECK(index >= 0 && index < (CARD_SIZE / 4));
    uint8_t expected = 0;
    return atomic_compare_exchange_strong(&stripe->u.bytes[index], &expected, 1);
}

static _Atomic uint8_t *attempt_get_card_slot(struct lxr_fields_logger *logger, void *address) {
    const uintptr_t tagged = (uintptr_t)address;
    int index = ((tagged & ~LXR_BLOCK_MASK) >> LXR_NORMAL_BLOCK_SHIFT) & TOP_STRIPES_MASK;
    DCHECK(index >= 0 && index < TOP_STRIPES_SIZE);
    struct lxr_log_stripe *stripe = load_stripe(&logger->top_stripes[index]);
    if (!stripe) {
        return NULL;
    }
    
    index = (tagged & LXR_BLOCK_MASK) >> CARD_SHIFT;
    DCHECK(index >= 0 && index < SECONDARY_STRIPES_SIZE);
    stripe = load_stripe(&stripe->u.next[index]);
    if (!stripe) {
        return NULL;
    }
    
    DCHECK(tagged % 4 == 0);
    index = (tagged & CARD_MASK) >> 2;
    DCHECK(index >= 0 && index < (CARD_SIZE / 4));
    return &stripe->u.bytes[index];
}

int lxr_attempt_to_unlog(struct lxr_fields_logger *logger, void *address) {
    _Atomic uint8_t *slot = attempt_get_card_slot(logger, address);
    if (!slot) {
        return 0;
    }
    uint8_t expected = 1;
    return atomic_compare_exchange_strong(slot, &expected, 0);
}

int lxr_has_logged(struct lxr_fields_logger *logger, void *address) {
    _Atomic uint8_t *slot = attempt_get_card_slot(logger, address);
    if (!slot) {
        return 0;
    }
    return (atomic_load_explicit(slot, memory_order_acquire) != 0);
}


int lxr_init_log_queue(struct lxr_log_queue *queue) {
    memset(queue, 0, sizeof(*queue));
    for (int i = 0; i < 2; i++) {
        queue->chunk[i].block = (uint8_t *)malloc(LXR_NORMAL_BLOCK_SIZE);
        if (!queue->chunk[i].block) {
            return -1;
        }
        queue->chunk[i].free = queue->chunk[i].block;
    }
    return 0;
}

void lxr_free_log_queue(struct lxr_log_queue *queue) {
    for (int i = 0; i < 2; i++) {
        free(queue->chunk[i].block);
        queue->chunk[i].free = NULL;
    }
    queue->head = NULL;
}

struct lxr_log_node *lxr_log_queue_push(struct lxr_log_queue *queue, void *data) {
    struct lxr_log_node *spawn = NULL;
    for (int i = 0; i < 2; i++) {
        const uint8_t *const end = queue->chunk[i].block + LXR_NORMAL_BLOCK_SIZE;
        uint8_t *p = atomic_fetch_add(&queue->chunk[i].free, sizeof(struct lxr_log_node));
        if (p < end) {
            spawn = (struct lxr_log_node *)p;
            break;
        }
    }
    if (!spawn) {
        return NULL;
    }
    spawn->data = data;

    struct lxr_log_node *expected = atomic_load_explicit(&queue->head, memory_order_relaxed);
    do {
        spawn->next = expected;
    } while (!atomic_compare_exchange_strong(&queue->head, &expected, spawn));
    return spawn;
}

struct lxr_log_node *lxr_log_queue_take(struct lxr_log_queue *queue) {
    struct lxr_log_node *expected = atomic_load_explicit(&queue->head, memory_order_relaxed);
    do {
        if (!expected) {
            break;
        }
    } while (!atomic_compare_exchange_strong(&queue->head, &expected, expected->next));
    return expected;
}

void lxr_log_queue_clear(struct lxr_log_queue *queue) {
    atomic_store_explicit(&queue->head, NULL, memory_order_relaxed);
    atomic_store_explicit(&queue->chunk[0].free, NULL, memory_order_relaxed);
    atomic_store_explicit(&queue->chunk[1].free, NULL, memory_order_relaxed);
}
