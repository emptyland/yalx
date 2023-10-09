#pragma once
#ifndef YALX_RUNTIME_DAEMON_H
#define YALX_RUNTIME_DAEMON_H

#include "runtime/locks.h"
#include <stdint.h>


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
int task_queue_final(struct task_queue *q);

void task_queue_post(struct task_queue *q, struct task_entry *task);
int task_queue_take(struct task_queue *q, struct task_entry **task);

struct task_entry *task_new(struct task_queue *q, void *ctx, task_run_fn_t run, task_finalize_fn_t finalize);
void task_final(struct task_queue *q, struct task_entry *task);



#endif //YALX_RUNTIME_DAEMON_H
