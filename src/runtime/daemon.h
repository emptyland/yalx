#pragma once
#ifndef YALX_RUNTIME_DAEMON_H
#define YALX_RUNTIME_DAEMON_H

#include "runtime/locks.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct task_entry;

typedef void (*task_run_fn_t)(struct task_entry *);
typedef void (*task_finalize_fn_t)(struct task_entry *);

struct task_entry {
    struct task_entry *next;
    struct task_entry *prev;
    void *ctx;
    uintptr_t param1;
    uintptr_t param2;
    task_run_fn_t run;
    task_finalize_fn_t finalize;
};

struct task_queue {
    struct yalx_mutex mutex;
    size_t count; // count of tasks
    struct yalx_cond not_empty;
    struct task_entry tasks;
};

int task_queue_init(struct task_queue *q);
void task_queue_final(struct task_queue *q);

void task_queue_post(struct task_queue *q, struct task_entry *task);
int task_queue_take(struct task_queue *q, struct task_entry **task);

struct task_entry *task_alloc(struct task_queue *q, size_t size);

static inline
struct task_entry *task_new(struct task_queue *q, void *ctx, task_run_fn_t run, task_finalize_fn_t finalize) {
    struct task_entry *task = task_alloc(q, sizeof(struct task_entry));
    task->ctx = ctx;
    task->run = run;
    task->finalize = finalize;
    return task;
}

void task_final(struct task_queue *q, struct task_entry *task);

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_DAEMON_H
