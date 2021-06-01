#include "runtime/process.h"
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


struct processor *yalx_find_idle_processor() {
    for (int i = 0; i < nprocs; i++) {
        struct processor *proc = &procs[i];
        pthread_mutex_lock(&proc->mutex);
        if (proc->n_threads == 0) {
            pthread_mutex_unlock(&proc->mutex);
            return proc;
        }
        
        if (proc->state == PROC_IDLE) {
            pthread_mutex_unlock(&proc->mutex);
            return proc;
        }
        // TODO:
        pthread_mutex_unlock(&proc->mutex);
    }
    
    return NULL;
}
