#include "runtime/scheduler.h"
#include "runtime/checking.h"
#include <pthread.h>
#if defined(YALX_OS_POSIX)
#include <sys/mman.h>
#endif // defined(YALX_OS_POSIX)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct scheduler scheduler;

static address_t allocate_notifier_page() {
    void *page = mmap(NULL, os_page_size, PROT_READ, MAP_ANON|MAP_PRIVATE, -1, 0);
    if (page == MAP_FAILED) {
        return NULL;
    }
    return (address_t)page;
}

static void free_notifier_page(address_t page) {
    munmap(page, os_page_size);
}

int yalx_init_scheduler(struct scheduler *sched) {
    pthread_mutex_init(&sched->mutex, NULL);
    sched->next_coid = 1;
    sched->notifier_page = allocate_notifier_page();
    // TODO:
    return 0;
}

void yalx_free_scheduler(struct scheduler *sched) {
    pthread_mutex_destroy(&sched->mutex);
    free_notifier_page(sched->notifier_page);
    sched->notifier_page = NULL;
    // TODO:
}

coid_t yalx_next_coid(void) {
    coid_t id;
    pthread_mutex_lock(&scheduler.mutex);
    id.value = scheduler.next_coid++;
    pthread_mutex_unlock(&scheduler.mutex);
    return id;
}


int yalx_install_coroutine(address_t entry, size_t params_bytes, address_t params_begin) {
    DCHECK(params_bytes % STACK_ALIGNMENT_SIZE == 0 && "must be alignment");
    struct machine *mach = thread_local_mach;
    struct stack *stack = yalx_new_stack_from_pool(&mach->stack_pool, STACK_DEFAULT_SIZE);
    if (!stack) {
        return -1;
    }
    address_t top = stack->top - params_bytes;
    if (params_bytes > 0) {
        memcpy(top, params_begin, params_bytes);
        top -= params_bytes;
    }
    address_t stub = (address_t)&coroutine_finalize_stub;
    top -= sizeof(&stub);
    *(address_t *)top = stub;
    //memcpy(top, &stub, sizeof(&stub));
    
    struct coroutine *co = MALLOC(struct coroutine); //(struct coroutine *)malloc(sizeof(struct coroutine));
    if (!co) {
        return -1;
    }
    coid_t id = { scheduler.next_coid++ };
    if (yalx_init_coroutine(id, co, stack, entry) < 0) {
        return -1;
    }
    co->state = CO_WAITTING;
    co->n_pc = entry;
    co->n_sp = top;
    co->n_fp = top;
    co->stub = entry;
    
    pthread_mutex_lock(&scheduler.mutex);
    QUEUE_INSERT_HEAD(&mach->waiting_head, co);
    pthread_mutex_unlock(&scheduler.mutex);
    return 0;
}


int yalx_schedule(void) {
    puts("yalx_schedule");
    struct machine *mach = thread_local_mach;
    struct coroutine *old_co = mach->running;
    DCHECK(old_co != NULL);

    pthread_mutex_lock(&scheduler.mutex);
    if (old_co->state == CO_DEAD) {
        yalx_delete_stack_to_pool(&mach->stack_pool, old_co->stack);
        free(old_co);
        old_co = NULL;
    }
    
    if (!QUEUE_EMPTY(&mach->waiting_head)) {
        struct coroutine *co = mach->waiting_head.next;
        QUEUE_REMOVE(co);
        co->state = CO_RUNNING;
        mach->running = co;

        if (old_co) {
            old_co->state = CO_WAITTING;
            QUEUE_INSERT_TAIL(&mach->waiting_head, old_co);
        }
        pthread_mutex_unlock(&scheduler.mutex);
        return 1; /* scheduled */
    }
    pthread_mutex_unlock(&scheduler.mutex);
    
    //pthread_exit(NULL);
    return old_co != mach->running;
}
