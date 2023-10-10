#pragma once
#ifndef YALX_RUNTIME_LOCKS_H_
#define YALX_RUNTIME_LOCKS_H_

#include "runtime/macros.h"


#ifdef __cplusplus
extern "C" {

// #define _Atomic

#if defined(YALX_OS_POSIX)
#include <pthread.h>
#endif

#if defined(YALX_OS_WINDOWS)
#include <Windows.h>
#endif

#else
#include <stdatomic.h>

#ifndef __STDC_NO_THREADS__
#include <threads.h>
#endif

#if defined(YALX_OS_POSIX)
#include <pthread.h>
#endif

#if defined(YALX_OS_WINDOWS)
#include <Windows.h>
#endif

#endif

#if !defined(__STDC_NO_THREADS__) && !defined(__cplusplus)
struct yalx_mutex {
    mtx_t impl;
};

static inline int yalx_mutex_init(struct yalx_mutex *self) {
    return mtx_init(&self->impl, mtx_plain | mtx_recursive) == thrd_success ? 0 : -1;
}

static inline void yalx_mutex_final(struct yalx_mutex *self) {
    mtx_destroy(&self->impl);
}

static inline void yalx_mutex_lock(struct yalx_mutex *self) {
    mtx_lock(&self->impl);
}

static inline int yalx_mutex_try_lock(struct yalx_mutex *self) {
    int rs = mtx_trylock(&self->impl);
    if (rs == thrd_success) {
        return 0;
    } else if (rs == thrd_busy) {
        return 1;
    } else {
        return -1;
    }
}

static inline void yalx_mutex_unlock(struct yalx_mutex *self) {
    mtx_unlock(&self->impl);
}

struct yalx_cond {
    cnd_t impl;
};

static inline int yalx_cond_init(struct yalx_cond *self) {
    return cnd_init(&self->impl) == thrd_success ? 0 : -1;
}

static inline void yalx_cond_final(struct yalx_cond *self) {
    cnd_destroy(&self->impl);
}

static inline int yalx_cond_notify_one(struct yalx_cond *self) {
    return cnd_signal(&self->impl) == thrd_success ? 0 : -1;
}

static inline int yalx_cond_notify_all(struct yalx_cond *self) {
    return cnd_broadcast(&self->impl) == thrd_success ? 0 : -1;
}

static inline int yalx_cond_wait(struct yalx_cond *self, struct yalx_mutex *mutex) {
    return cnd_wait(&self->impl, &mutex->impl) == thrd_success ? 0 : -1;
}

#else
#if defined(YALX_OS_POSIX)
struct yalx_mutex {
    pthread_mutex_t impl;
};

struct yalx_cond {
    pthread_cond_t impl;
};

static inline int yalx_mutex_init(struct yalx_mutex *self) {
    return pthread_mutex_init(&self->impl, NULL);
}

static inline void yalx_mutex_final(struct yalx_mutex *self) {
    pthread_mutex_destroy(&self->impl);
}

static inline void yalx_mutex_lock(struct yalx_mutex *self) {
    pthread_mutex_lock(&self->impl);
}

static inline int yalx_mutex_try_lock(struct yalx_mutex *self) {
    return pthread_mutex_trylock(&self->impl);
}

static inline void yalx_mutex_unlock(struct yalx_mutex *self) {
    pthread_mutex_unlock(&self->impl);
}
#endif

#if defined(YALX_OS_WINDOWS)
struct yalx_mutex {
    CRITICAL_SECTION impl;
};

static inline int yalx_mutex_init(struct yalx_mutex *self) {
    return InitializeCriticalSectionAndSpinCount(&self->impl, 10000);
}

static inline void yalx_mutex_final(struct yalx_mutex *self) {
    DeleteCriticalSection(&self->impl);
}

static inline void yalx_mutex_lock(struct yalx_mutex *self) {
    EnterCriticalSection(&self->impl);
}

static inline int yalx_mutex_try_lock(struct yalx_mutex *self) {
    return TryEnterCriticalSection(&self->impl);
}

static inline void yalx_mutex_unlock(struct yalx_mutex *self) {
    LeaveCriticalSection(&self->impl);
}

#endif
#endif

struct yalx_spin_lock {
    _Atomic int core;
};

static inline void yalx_init_spin_lock(struct yalx_spin_lock *lock) { lock->core = 0; }

void yalx_spin_lock(struct yalx_spin_lock *lock);

void yalx_spin_unlock(struct yalx_spin_lock *lock);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_LOCKS_H_
