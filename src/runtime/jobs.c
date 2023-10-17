#include "runtime/jobs.h"
#include "runtime/locks.h"
#include "runtime/checking.h"
#include "runtime/runtime.h"
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>

void yalx_job_init(struct yalx_job *job, const char *name, size_t n_concurrent) {
    DCHECK(n_concurrent > 0);
    job->name = (!name || !name[0]) ? "unknown" : strdup(name);
    job->n_workers = n_concurrent;
    job->workers = MALLOC_N(struct yalx_worker, job->n_workers);
    for (int i = 0; i < job->n_workers; i++) {
        job->workers[i].id = i;
        job->workers[i].owns = job;
        job->workers[i].total = (int)n_concurrent;
        job->workers[i].run = NULL;
    }
    task_queue_init(&job->queue);
    yalx_mutex_init(&job->mutex);
    yalx_cond_init(&job->all_done);
    job->active = 0;
}

void yalx_job_final(struct yalx_job *job) {
    while (atomic_load_explicit(&job->active, memory_order_acquire) > 0) {
        sched_yield();
    }
    yalx_cond_final(&job->all_done);
    yalx_mutex_final(&job->mutex);
    task_queue_final(&job->queue);

    free(job->workers);
    free((void *)job->name);
}

static void worker_entry(struct yalx_worker *worker) {
    worker->run(worker);
    atomic_fetch_sub(&worker->owns->active, 1);
}

void yalx_job_submit(struct yalx_job *job, yalx_working_fn_t worker, void *ctx) {
    size_t prev = atomic_fetch_add(&job->active, job->n_workers);
    DCHECK(prev == 0);

    char name[260];
    for (int i = 0; i < job->n_workers; i++) {
        snprintf(name, arraysize(name), "yalx-job-%s[%d]", job->name, i);
        job->workers[i].run = worker;
        job->workers[i].ctx = ctx;
        yalx_os_thread_start(&job->workers[i].thread, (yalx_os_thread_fn)worker_entry, &job->workers[i], name,
                             __FILE__, __LINE__);
    }

    for (int i = 0; i < job->n_workers; i++) {
        yalx_os_thread_join(&job->workers[i].thread, 0);
    }
}

static void pool_entry(struct yalx_worker *worker) {
    struct yalx_job *job = worker->owns;
    for (;;) {
        struct task_entry *task = NULL;
        task_queue_take(&job->queue, &task);
        if (task->run == NULL) {
            break;
        }
        task->param2 = (uintptr_t)worker;
        task->run(task);
        task_final(&job->queue, task);
    }
    atomic_fetch_sub(&worker->owns->active, 1);
}

void yalx_pool_start(struct yalx_job *job) {
    size_t prev = atomic_fetch_add(&job->active, job->n_workers);
    DCHECK(prev == 0);

    char name[260];
    for (int i = 0; i < job->n_workers; i++) {
        snprintf(name, arraysize(name), "yalx-pool-%s[%d]", job->name, i);
        yalx_os_thread_start(&job->workers[i].thread, (yalx_os_thread_fn)pool_entry, &job->workers[i], name,
                             __FILE__, __LINE__);
    }
}

void yalx_pool_shutdown(struct yalx_job *job) {
    // TODO:
    UNREACHABLE();
}

void yalx_pool_post(struct yalx_job *job, task_run_fn_t routine, void *params) {
    // TODO:
    UNREACHABLE();
}
