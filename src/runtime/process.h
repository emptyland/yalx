#pragma once
#ifndef YALX_RUNTIME_PROCESS_H_
#define YALX_RUNTIME_PROCESS_H_

#include "runtime/stack.h"
#include "runtime/runtime.h"
#include "runtime/thread.h"
#include "runtime/locks.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PROCESSORS 100
#define MAX_THREADS    1000

// GMP model
//typedef i32_t pid_t;
//typedef i32_t mid_t;
//typedef i32_t cid_t;

struct yalx_value_throwable;
struct processor;
struct coroutine;
struct machine;

struct stack;

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

enum coroutine_state {
    CO_INIT,
    CO_PARKING,
    CO_WAITTING,
    CO_RUNNING,
    CO_SYSCALL,
    CO_DEAD,
}; // enum coroutine_state

struct processor_id {
    i32_t value;
};
typedef struct processor_id procid_t;

struct coroutine_id {
    u64_t value;
};
typedef struct coroutine_id coid_t;

struct unwind_node {
    struct unwind_node *prev;
    address_t addr; // address of function
};

struct coroutine {
    QUEUE_HEADER(struct coroutine);
    coid_t id;
    enum coroutine_state state;
    struct stack *stack;
    address_t entry;
    address_t c_sp; // Only for c0
    address_t c_fp; // Only for c0
    address_t n_pc;
    address_t n_sp;
    address_t n_fp;
    address_t stub; // stub address for none-c0 coroutine
    struct yalx_returning_vals *returning_vals;
    struct unwind_node *top_unwind_point; // unwind for exception handler
    struct yalx_value_throwable *exception; // the exception happened
}; // struct coroutine

#define ROOT_OFFSET_TOP_UNWIND offsetof(struct coroutine, top_unwind_point)
#define ROOT_OFFSET_EXCEPTION offsetof(struct coroutine, exception)


struct machine {
    QUEUE_HEADER(struct machine);
    struct processor *owns;
    struct yalx_os_thread thread;
    enum machine_state state;
    struct coroutine *running;
    struct stack_pool stack_pool;
    struct coroutine waitting_head;
    struct coroutine parking_head;
    // TODO:
}; // struct machine

struct processor {
    procid_t id;
    enum processor_state state;
    int n_threads;
    struct machine machine_head;
    struct yalx_mutex mutex;
}; // struct processor

// Implements in runtime.c
extern struct processor *procs;
extern int nprocs;
extern struct machine m0;
extern struct coroutine c0;

extern yalx_tls_t tls_mach;

#define thread_local_mach ((struct machine *)yalx_tls_get(tls_mach))


int yalx_init_processor(procid_t id, struct processor *proc);

int yalx_add_machine_to_processor(struct processor *proc, struct machine *m);

enum processor_state yalx_set_processor_state(struct processor *proc, enum processor_state state);

int yalx_init_machine(struct machine *mach, struct processor *owns);

static inline int yalx_is_m0(struct machine *m) { return m == &m0; }

int yalx_init_coroutine(coid_t id, struct coroutine *co, struct stack *stack, address_t entry);

void yalx_free_coroutine(struct coroutine *co);


#define CURRENT_COROUTINE (!thread_local_mach->running ? &c0 : thread_local_mach->running)

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_PROCESS_H_
