#include "runtime/scheduler.h"
#include "runtime/checking.h"
#include <pthread.h>
#if defined(YALX_OS_DARWIN)
#include <sys/mman.h>
#endif // defined(YALX_OS_DARWIN)
#include <stdio.h>

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

int yalx_schedule() {
    
    puts("yalx_schedule");
    printf("%p\n", thread_local_mach->running->n_pc);
    return 1;
}
