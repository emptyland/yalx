#include "runtime/daemon.h"
#include "runtime/macros.h"
#include "runtime/runtime.h"
#include <stdlib.h>

int task_queue_init(struct task_queue *q) {
    q->tasks.next = &q->tasks;
    q->tasks.prev = &q->tasks;
    q->count = 0;
    yalx_mutex_init(&q->mutex);
    yalx_cond_init(&q->not_empty);
    return 0;
}

int task_queue_final(struct task_queue *q) {
    // TODO:
    yalx_mutex_final(&q->mutex);
    yalx_cond_final(&q->not_empty);
}

void task_queue_post(struct task_queue *q, struct task_entry *task) {
    yalx_mutex_lock(&q->mutex);
    QUEUE_INSERT_TAIL(&q->tasks, task);
    q->count++;
    yalx_cond_notify_one(&q->not_empty);
    yalx_mutex_unlock(&q->mutex);
}

int task_queue_take(struct task_queue *q, struct task_entry **task) {
    yalx_mutex_lock(&q->mutex);
    if (q->count == 0) {
        yalx_cond_wait(&q->not_empty, &q->mutex);
    }

    struct task_entry *head = q->tasks.next;
    QUEUE_REMOVE(head);
    yalx_mutex_unlock(&q->mutex);

    *task = head;
    return 1;
}

struct task_entry *task_new(struct task_queue *q, void *ctx, task_run_fn_t run, task_finalize_fn_t finalize) {
    struct task_entry *task = MALLOC(struct task_entry);
    task->next = task;
    task->prev = task;
    task->ctx = ctx;
    task->run = run;
    task->finalize = finalize;
    task->param1 = 0;
    task->param2 = 0;
    return task;
}

void task_final(struct task_queue *q, struct task_entry *task) {
    if (task->finalize) {
        task->finalize(task);
    }
    free(task);
}