#pragma once
#ifndef YALX_RUNTIME_PROCESS_H_
#define YALX_RUNTIME_PROCESS_H_

#include "runtime/runtime.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PROCESSORS 100
#define MAX_THREADS    1000

// GMP model
//typedef i32_t pid_t;
//typedef i32_t mid_t;
//typedef i32_t cid_t;

struct processor;
struct coroutine;
struct machine;

enum processor_state {
    PROC_INIT,
    PROC_IDLE,
    PROC_RUNNING,
    PROC_SYSCALL,
}; // enum processor_state

enum machine_state {
    MACH_INIT,
    MACH_IDLE,
    MACH_RUNNING,
    MACH_SYSCALL
}; // enum processor_state

struct processor_id {
    i32_t value;
};
typedef struct processor_id procid_t;

struct coroutine_id {
    i64_t value;
};
typedef struct coroutine_id coid_t;

struct coroutine {
    QUEUE_HEADER(struct coroutine);
    coid_t id;
}; // struct coroutine


struct machine {
    QUEUE_HEADER(struct machine);
    struct processor *owns;
    pthread_t thread;
    enum machine_state state;
    
    // TODO:
}; // struct machine

struct processor {
    procid_t id;
    enum processor_state state;
    int n_threads;
    struct machine machine_head;
    pthread_mutex_t mutex;
}; // struct processor

// Implements in runtime.c
extern struct processor *procs;
extern int nprocs;
extern struct machine m0;
extern _Thread_local struct machine *thread_local_mach;

int yalx_init_processor(const procid_t id, struct processor *proc);

int yalx_add_machine_to_processor(struct processor *proc, struct machine *m);

enum processor_state yalx_set_processor_state(struct processor *proc, enum processor_state state);

struct processor *yalx_find_idle_processor(void);

static inline int yalx_is_m0(struct machine *m) { return m == &m0; }

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_PROCESS_H_
