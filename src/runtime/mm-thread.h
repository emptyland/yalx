#pragma once
#ifndef YALX_RUNTIME_MM_THREAD_H
#define YALX_RUNTIME_MM_THREAD_H

#include "runtime/thread.h"
#include "runtime/daemon.h"

#ifdef __cplusplus
extern "C" {
#endif

// Memory Management Thread
struct yalx_mm_thread {
    struct yalx_os_thread thread;
    struct task_queue worker_queue;
    _Atomic int active;
    _Atomic int shutting_down;
};

extern const char *const YALX_MM_THREAD_NAME;

int yalx_mm_thread_start(struct yalx_mm_thread *mm);
void yalx_mm_thread_shutdown(struct yalx_mm_thread *mm);

struct yalx_mm_thread *mm_thread_of_task(struct task_entry *task);

void mm_thread_wait_inactive(struct yalx_mm_thread *mm);

int mm_thread_is_shutting_down(struct yalx_mm_thread *mm);

void mm_thread_post_routine_to(struct yalx_mm_thread *mm, task_run_fn_t routine, void *params);

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_MM_THREAD_H
