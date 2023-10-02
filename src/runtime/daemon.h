#pragma once
#ifndef YALX_RUNTIME_DAEMON_H
#define YALX_RUNTIME_DAEMON_H

#include "runtime/locks.h"
#include <stdint.h>

struct task_entry {
    struct task_entry *next;
    struct task_entry *prev;
    void *ctx;
    uintptr_t param1;
    uintptr_t param2;
    void (*run)(struct task_entry *);
    void (*finalize)(struct task_entry *);
};

struct task_queue {
    struct yalx_mutex mutex;
    //struct yalx_cond not_full;
    struct yalx_cond not_empty;
    struct task_entry tasks;
};

int task_queue_init(struct task_queue *q);
int task_queue_final(struct task_queue *q);

void task_queue_post(struct task_queue *q, struct task_entry *task);
int task_queue_take(struct task_queue *q, struct task_entry **task);



#endif //YALX_RUNTIME_DAEMON_H
