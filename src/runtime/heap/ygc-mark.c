#include "runtime/heap/ygc-mark.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/heap/ygc.h"
#include "runtime/object/any.h"
#include "runtime/runtime.h"
#include "runtime/checking.h"
#include <stdatomic.h>
#include <stdlib.h>


void ygc_mark_init(struct ygc_mark *mark, struct ygc_core *owns) {
    size_t n = ncpus / 2;
    if (n > YGC_MAX_MARKING_STRIPES) {
        n = YGC_MAX_MARKING_STRIPES;
    } else if (n < 4) {
        n = 4;
    }
    mark->owns = owns;
    yalx_job_init(&mark->job, "yalx-marking", n);
    memset(mark->stripes, 0, sizeof(mark->stripes));
}

void ygc_mark_final(struct ygc_mark *mark) {
    for (int i = 0; i < YGC_MAX_MARKING_STRIPES; i++) {
        ygc_marking_stripe_clear(&mark->stripes[i]);
    }
    yalx_job_final(&mark->job);
}

static size_t stripe_for_index(uintptr_t addr) {
    DCHECK(addr % pointer_size_in_bytes == 0);
    uintptr_t offset = ygc_offset(addr);
    return (offset >> YGC_GRANULE_SHIFT) % YGC_MAX_MARKING_STRIPES;
}

static void stack_push(struct ygc_marking_stack *stack, uintptr_t addr) {
    if (stack->top + 1 >= stack->max) {
        stack->max = (stack->top + 1) << 1; // 2 times
        stack->stack = realloc(stack->stack, sizeof(uintptr_t) * stack->max);
    }
    stack->stack[stack->top++] = addr;
}

static struct ygc_marking_stack *tls_current_stack(struct ygc_marking_stripe *stripe, size_t index) {
    DCHECK(index < YGC_MAX_MARKING_STRIPES);
    struct ygc_tls_struct *data = ygc_tls_data();
    if (!data->stacks[index]) {
        struct ygc_marking_stack *stack = MALLOC(struct ygc_marking_stack);
        stack->next = NULL;
        stack->stripe = stripe;
        stack->max = 16;
        stack->top = 0;
        stack->stack = (uintptr_t *) malloc(sizeof(uintptr_t) * stack->max);
        data->stacks[index] = stack;
    }
    return data->stacks[index];
}

void ygc_marking_mark_object(struct ygc_mark *mark, uintptr_t addr) {
    DCHECK(ygc_is_marked(addr) && "addr must be marked");
    size_t index = stripe_for_index(addr);
    struct ygc_marking_stripe *stripe = &mark->stripes[index];
    struct ygc_marking_stack *stack = tls_current_stack(stripe, index);
    stack_push(stack, addr);
}

static void visit_object_pointer(struct yalx_object_visitor *v, yalx_ref_t host, yalx_ref_t *p) {
    if (!*p || ygc_is_marked(*p)) {
        return;
    }

    struct ygc_core *ygc = (struct ygc_core *)v->ctx;
    ygc_barrier_mark_on_field(ygc, (_Atomic volatile yalx_ref_t *)p);
}

static void visit_object_pointers(struct yalx_object_visitor *v, yalx_ref_t host, yalx_ref_t *begin, yalx_ref_t *end) {
    struct ygc_core *ygc = (struct ygc_core *)v->ctx;

    for (yalx_ref_t *x = begin; x < end; x++) {
        if (!*x || ygc_is_marked(*x)) {
            return;
        }
        ygc_barrier_mark_on_field(ygc, (_Atomic volatile yalx_ref_t *)x);
    }
}

static void marking_follow_mark(struct ygc_mark *mark, uintptr_t addr) {
    //DCHECK(ygc_is_marked(addr));

    struct yalx_value_any *obj = (struct yalx_value_any *)addr;
    if (!obj) {
        return;
    }

    struct yalx_object_visitor visitor = {
        mark->owns,0,0,visit_object_pointers,visit_object_pointer,
    };

    yalx_object_shallow_visit(obj, &visitor);
}

static void marking_mark_stripe(struct ygc_mark *mark, struct ygc_marking_stripe *stripe) {
    for (;;) {
        volatile struct ygc_marking_stack *stack = NULL;
        do {
            stack = stripe->committed_stacks;
            if (!stack) {
                return;
            }
        } while (!atomic_compare_exchange_strong(&stripe->committed_stacks, &stack, stack->next));
        atomic_fetch_sub(&stripe->n_stacks, 1);

        while (stack->top > 0) {
            uintptr_t addr = stack->stack[--stack->top];
            marking_follow_mark(mark, addr);
        }
        free(stack->stack);
        free((void *)stack);
    }
}

static void marking_entry(struct yalx_worker *worker) {
    struct ygc_mark *mark = (struct ygc_mark *)worker->ctx;
    if (worker->total >= YGC_MAX_MARKING_STRIPES) {
        struct ygc_marking_stripe *stripe = &mark->stripes[worker->id];
        marking_mark_stripe(mark, stripe);
    } else {
        const size_t begin = worker->id * worker->total;
        const size_t end = (begin + worker->total);
        const size_t limit = end > YGC_MAX_MARKING_STRIPES ? YGC_MAX_MARKING_STRIPES : end;
        for (size_t i = begin; i < limit; i++) {
            struct ygc_marking_stripe *stripe = &mark->stripes[i];
            marking_mark_stripe(mark, stripe);
        }
    }
}

void ygc_marking_concurrent_mark(struct ygc_mark *mark) {
    yalx_job_submit(&mark->job, marking_entry, (void *)mark);
}

void ygc_marking_tls_commit(struct ygc_mark *mark, struct yalx_os_thread *thread) {
    struct ygc_tls_struct *data = (struct ygc_tls_struct *)thread->gc_data;
    for (int i = 0; i < YGC_MAX_MARKING_STRIPES; i++) {
        if (data->stacks[i]) {
            ygc_marking_stripe_commit(&mark->stripes[i], data->stacks[i]);
            data->stacks[i] = NULL;
        }
    }
}

void ygc_marking_stripe_commit(struct ygc_marking_stripe *stripe, struct ygc_marking_stack *stack) {
    volatile struct ygc_marking_stack *next = NULL;
    do {
        next = stripe->committed_stacks;
        stack->next = next;
    } while (!atomic_compare_exchange_strong(&stripe->committed_stacks, &next, stack));

    atomic_fetch_add(&stripe->n_stacks, 1);
}

void ygc_marking_stripe_clear(struct ygc_marking_stripe *stripe) {
    while (stripe->committed_stacks) {
        volatile struct ygc_marking_stack *stack = stripe->committed_stacks;
        stripe->committed_stacks = stack->next;
        free(stack->stack);
        free((void *)stack);
    }
    stripe->n_stacks = 0;
}