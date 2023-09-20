#pragma once
#ifndef YALX_RUNTIME_SCHEDULER_H_
#define YALX_RUNTIME_SCHEDULER_H_

#include "runtime/process.h"

#ifdef __cplusplus
extern "C" {
#endif


struct scheduler {
    // Next coroutine id
    u64_t next_coid;
    // Mutex for scheduing
    pthread_mutex_t mutex;
    // Safepoint polling ygc_page
    address_t notifier_page;
    // TODO:
}; // struct scheduler

extern struct scheduler scheduler;

int yalx_init_scheduler(struct scheduler *sched);

void yalx_free_scheduler(struct scheduler *sched);

coid_t yalx_next_coid(void);

int yalx_schedule(void);

int yalx_install_coroutine(address_t entry, size_t params_bytes, address_t params_begin);

#ifdef __cplusplus
}
#endif


#endif // YALX_RUNTIME_SCHEDULER_H_
