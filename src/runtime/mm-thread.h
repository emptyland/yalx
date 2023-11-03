#pragma once
#ifndef YALX_RUNTIME_MM_THREAD_H
#define YALX_RUNTIME_MM_THREAD_H

#include "runtime/thread.h"
#include "runtime/daemon.h"

#ifdef __cplusplus
extern "C" {
#endif

enum synchronize_state {
    NOT_SYNCHRONIZED,
    SYNCHRONIZING,
    SYNCHRONIZED,
};

// Memory Management Thread
struct yalx_mm_thread {
    struct yalx_os_thread thread;
    struct task_queue worker_queue;
    _Atomic int active;
    _Atomic int shutting_down;

    volatile enum synchronize_state state;
    volatile _Atomic int safepoint_count;
};

extern const char *const YALX_MM_THREAD_NAME;

/** Global MM Thread, Impl in runtime.c */
extern struct yalx_mm_thread mm_thread;

extern uint8_t *mm_polling_page;

int yalx_mm_thread_start(struct yalx_mm_thread *mm);
void yalx_mm_thread_shutdown(struct yalx_mm_thread *mm);

struct yalx_mm_thread *mm_thread_of_task(struct task_entry *task);

void mm_thread_wait_inactive(struct yalx_mm_thread *mm);

int mm_thread_is_shutting_down(struct yalx_mm_thread *mm);

void mm_thread_post_routine_to(struct yalx_mm_thread *mm, task_run_fn_t routine, void *params);

void *mm_new_polling_page();
void mm_free_polling_page(void *page);
void mm_arm_polling_page(void *page, int armed);

/** Must call in MM thread */
void mm_synchronize_begin(struct yalx_mm_thread *mm);

/** Must call in MM thread */
void mm_synchronize_end(struct yalx_mm_thread *mm);

/** Must call in MM thread */
static inline void mm_pause_call(struct yalx_mm_thread *mm, void (*callback)(void *), void *params) {
    mm_synchronize_begin(mm);
    callback(params);
    mm_synchronize_end(mm);
}

enum synchronize_state mm_synchronize_state(struct yalx_mm_thread const *mm);

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_MM_THREAD_H
