#include "runtime/scheduler.h"
#include "runtime/checking.h"
#include <pthread.h>

struct scheduler scheduler;

int yalx_init_scheduler(struct scheduler *sched) {
    pthread_mutex_init(&sched->mutex, NULL);
    sched->next_coid = 1;
    sched->notifier_page = NULL;
    // TODO:
    return 0;
}

void yalx_free_scheduler(struct scheduler *sched) {
    pthread_mutex_destroy(&sched->mutex);
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
