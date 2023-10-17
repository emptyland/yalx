#include "runtime/heap/ygc-mark.h"
#include "runtime/heap/ygc.h"
#include "runtime/runtime.h"
#include "runtime/checking.h"
#include <stdatomic.h>
#include <stdlib.h>

void ygc_mark_init(struct ygc_mark *mark) {
    yalx_job_init(&mark->job, "yalx-marking", ncpus / 2 < 4 ? 4 : ncpus / 2);
    memset(mark->stripes, 0, sizeof(mark->stripes));
}

void ygc_mark_final(struct ygc_mark *mark) {
    for (int i = 0; i < YGC_MAX_MARKING_STRIPES; i++) {
        free(mark->stripes[i].stack);
    }
    yalx_job_final(&mark->job);
}

static struct ygc_marking_stripe *stripe_for_addr(struct ygc_mark *mark, uintptr_t addr) {
    DCHECK(addr % pointer_size_in_bytes == 0);
    uintptr_t offset = ygc_offset(addr);
    size_t index = (offset >> YGC_GRANULE_SHIFT) % YGC_MAX_MARKING_STRIPES;
    return &mark->stripes[index];
}

static void stripe_push_to(struct ygc_marking_stripe *stripe, uintptr_t addr) {
    static uintptr_t *const BUSY = (uintptr_t *)0x1;

    for (;;) {
        uintptr_t *stack = stripe->stack;
        if (!atomic_compare_exchange_strong(&stripe->stack, &stack, BUSY)) {
            continue;
        }
        if (stack == BUSY) {
            continue;
        }

        size_t top = stripe->top ++;
        if (top + 1 >= stripe->max) {
            if (stack == NULL && stripe->max == 0) {
                stripe->max = 128;
            } else {
                stripe->max <<= 1;
            }

            stack = realloc(stack, stripe->max * sizeof(uintptr_t));
        }
        stack[top] = addr;
        atomic_store(&stripe->stack, stack);
        break;
    }
}

void ygc_marking_mark_object(struct ygc_mark *mark, uintptr_t addr) {
    DCHECK(ygc_is_marked(addr) && "addr must be marked");
    struct ygc_marking_stripe *stripe = stripe_for_addr(mark, addr);
    stripe_push_to(stripe, addr);
}