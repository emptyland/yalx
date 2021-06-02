#include "runtime/process.h"
#include "runtime/checking.h"
#include <stdlib.h>

int yalx_init_processor(const procid_t id, struct processor *proc) {
    // TODO:
    proc->id = id;
    proc->state = PROC_INIT;
    //proc->machine_hea
    proc->machine_head.next = &proc->machine_head;
    proc->machine_head.prev = &proc->machine_head;
    
    pthread_mutex_init(&proc->mutex, NULL);
    return 0;
}

int yalx_add_machine_to_processor(struct processor *proc, struct machine *m) {
    pthread_mutex_lock(&proc->mutex);
    QUEUE_INSERT_HEAD(&proc->machine_head, m);
    proc->n_threads++;
    m->owns = proc;
    pthread_mutex_unlock(&proc->mutex);
    return proc->n_threads;
}

enum processor_state yalx_set_processor_state(struct processor *proc, enum processor_state state) {
    pthread_mutex_lock(&proc->mutex);
    enum processor_state old = proc->state;
    proc->state = state;
    pthread_mutex_unlock(&proc->mutex);
    return old;
}


int yalx_init_machine(struct machine *mach, struct processor *owns) {
    mach->next = mach;
    mach->prev = mach;
    mach->owns = owns;
    mach->state = MACH_INIT;
    mach->running = NULL;
    mach->coroutine_head.next = &mach->coroutine_head;
    mach->coroutine_head.prev = &mach->coroutine_head;
    yalx_init_stack_pool(&mach->stack_pool, 10 * MB);
    return 0;
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
