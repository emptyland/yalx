#include "runtime/process.h"
#include "runtime/mm-thread.h"
#include "runtime/checking.h"
#include "runtime/locks.h"
#include <stdlib.h>

/** Mutex for thread of machines start and ends */
struct yalx_mutex mach_threads_mutex;

int yalx_init_processor(const procid_t id, struct processor *proc) {
    // TODO:
    proc->id = id;
    proc->state = PROC_INIT;
    //proc->machine_hea
    proc->machine_head.next = &proc->machine_head;
    proc->machine_head.prev = &proc->machine_head;

    yalx_mutex_init(&proc->mutex);
    return 0;
}

int yalx_add_machine_to_processor(struct processor *proc, struct machine *m) {
    yalx_mutex_lock(&proc->mutex);
    QUEUE_INSERT_HEAD(&proc->machine_head, m);
    proc->n_threads++;
    m->owns = proc;
    yalx_mutex_unlock(&proc->mutex);
    return proc->n_threads;
}

void yalx_remove_machine_from_processor(struct machine *mach) {
    DCHECK(mach->owns != NULL);
    struct processor *proc = mach->owns;
    yalx_mutex_lock(&proc->mutex);
    QUEUE_REMOVE(mach);
    proc->n_threads++;
    mach->owns = NULL;
    yalx_mutex_unlock(&proc->mutex);
}

enum processor_state yalx_set_processor_state(struct processor *proc, enum processor_state state) {
    yalx_mutex_lock(&proc->mutex);
    enum processor_state old = proc->state;
    proc->state = state;
    yalx_mutex_unlock(&proc->mutex);
    return old;
}


int yalx_init_machine(struct machine *mach) {
    mach->next = mach;
    mach->prev = mach;
    mach->owns = NULL;
    mach->state = MACH_INIT;
    mach->running = NULL;
    mach->polling_page = mm_polling_page;
    mach->dummy.run = NULL;
    mach->dummy.params = NULL;
    mach->waiting_head.next = &mach->waiting_head;
    mach->waiting_head.prev = &mach->waiting_head;
    mach->parking_head.next = &mach->parking_head;
    mach->parking_head.prev = &mach->parking_head;
    yalx_init_stack_pool(&mach->stack_pool, 10 * MB);
    return 0;
}

static void mach_dummy_entry(void *ctx) {
    struct machine *const mach = (struct machine *)ctx;
    yalx_tls_set(tls_mach, mach);
    atomic_store_explicit(&mach->state, MACH_RUNNING, memory_order_release);
    mach->dummy.run(mach->dummy.params);
    yalx_tls_set(tls_mach, NULL);
}

int yalx_mach_run_dummy(struct machine *mach, void (*run)(void *), void *params) {
    GUARANTEE(mach->owns != NULL, "No processor attach by machine");
    GUARANTEE(mach->state == MACH_INIT, "Machine not ready, state is %d", mach->state);

    mach->dummy.run = run;
    mach->dummy.params = params;

    return yalx_os_thread_start(&mach->thread, mach_dummy_entry, mach, "yalx-mach-dummy",
                                __FILE__, __LINE__);
}

void yalx_mach_join(struct machine *mach) {
    yalx_os_thread_join(&mach->thread, 0);
    yalx_remove_machine_from_processor(mach);
    yalx_free_stack_pool(&mach->stack_pool);
}

int yalx_init_coroutine(const coid_t id, struct coroutine *co, struct stack *stack, address_t entry) {
    memset(co, 0, sizeof(*co));
    co->entry = entry;
    co->next = co;
    co->prev = co;
    co->id = id;
    co->state = CO_INIT;
    DCHECK(stack != NULL);
    co->stack = stack;
    return 0;
}

void yalx_free_coroutine(struct coroutine *co) {
    //yalx_free
    // TODO:
}
