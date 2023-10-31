#pragma once
#ifndef YALX_RUNTIME_JOBS_H
#define YALX_RUNTIME_JOBS_H

#include "runtime/thread.h"
#include "runtime/locks.h"
#include "runtime/daemon.h"

#ifdef __cplusplus
extern "C" {
#endif

struct yalx_job;
struct yalx_worker;

typedef void (*yalx_working_fn_t)(struct yalx_worker *);

struct yalx_worker {
    struct yalx_os_thread thread;
    struct yalx_job *owns;
    yalx_working_fn_t run;
    void *ctx;
    int total;
    int id;
};

struct yalx_job {
    const char *name;
    struct yalx_worker *workers;
    size_t n_workers;
    struct task_queue queue;
    struct yalx_mutex mutex;
    struct yalx_cond all_done;
    _Atomic size_t active;
};

void yalx_job_init(struct yalx_job *job, const char *name, size_t n_concurrent);

void yalx_job_final(struct yalx_job *job);

void yalx_job_submit(struct yalx_job *job, yalx_working_fn_t worker, void *ctx);

void yalx_pool_start(struct yalx_job *job);
void yalx_pool_shutdown(struct yalx_job *job);
void yalx_pool_post(struct yalx_job *job, task_run_fn_t routine, void *params);

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_JOBS_H
