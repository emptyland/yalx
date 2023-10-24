#pragma once
#ifndef YALX_RUNTIME_HEAP_YGC_MARK_H
#define YALX_RUNTIME_HEAP_YGC_MARK_H

#include "runtime/jobs.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YGC_MAX_MARKING_STRIPES 16

struct yalx_value_any;
struct yalx_os_thread;
struct ygc_core;

struct ygc_marking_stack {
    volatile struct ygc_marking_stack *_Atomic next;
    struct ygc_marking_stripe *stripe;
    uintptr_t *stack;
    size_t top;
    size_t max;
};

struct ygc_marking_stripe {
    volatile struct ygc_marking_stack *_Atomic committed_stacks;
    _Atomic size_t n_stacks;
};

struct ygc_mark {
    struct ygc_core *owns;
    struct yalx_job job;
    struct ygc_marking_stripe stripes[YGC_MAX_MARKING_STRIPES];
};

void ygc_mark_init(struct ygc_mark *mark, struct ygc_core *owns);
void ygc_mark_final(struct ygc_mark *mark);

// Mark a single object
void ygc_marking_mark_object(struct ygc_mark *mark, uintptr_t addr);

void ygc_marking_concurrent_mark(struct ygc_mark *mark);

// Commit thread local all stacks to stripes
void ygc_marking_tls_commit(struct ygc_mark *mark, struct yalx_os_thread *thread);

void ygc_marking_stripe_commit(struct ygc_marking_stripe *stripe, struct ygc_marking_stack *stack);
void ygc_marking_stripe_clear(struct ygc_marking_stripe *stripe);

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_HEAP_YGC_MARK_H
