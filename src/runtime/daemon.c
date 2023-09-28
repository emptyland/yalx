#include "runtime/daemon.h"
#include "runtime/macros.h"

int task_queue_init(struct task_queue *q) {
    q->tasks.next = &q->tasks;
    q->tasks.prev = &q->tasks;
    yalx_mutex_init(&q->mutex);
    yalx_cond_init(&q->cond);
}

int task_queue_final(struct task_queue *q) {
    // TODO:
    yalx_mutex_final(&q->mutex);
    yalx_cond_final(&q->cond);
}

void task_queue_post(struct task_queue *q, struct task_entry *task) {
    yalx_mutex_lock(&q->mutex);
    QUEUE_INSERT_TAIL(&q->tasks, task);
    yalx_mutex_unlock(&q->mutex);
}

int task_queue_take(struct task_queue *q, struct task_entry **task) {
    yalx_mutex_lock(&q->mutex);
    if (QUEUE_EMPTY(&q->tasks)) {
        yalx_cond_wait(&q->cond, &q->mutex);
    }

    struct task_entry *head = q->tasks.next;
    QUEUE_REMOVE(head);
    yalx_mutex_unlock(&q->mutex);


}