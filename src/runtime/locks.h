#pragma once
#ifndef YALX_RUNTIME_LOCKS_H_
#define YALX_RUNTIME_LOCKS_H_

#ifdef __cplusplus
extern "C" {
#else
#include <stdatomic.h>
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
