#include "runtime/locks.h"
#include "runtime/runtime.h"
#include <pthread.h>

#define SPIN_LOCK_BITS 1
#define SPIN_COUNT 4096

#define PAUSE() (void)0

void yalx_spin_lock(struct yalx_spin_lock *lock) {
    for (;;) {
        int expected = 0;
        if (atomic_compare_exchange_strong(&lock->core, &expected, SPIN_LOCK_BITS) && expected == 0) {
            return;
        }
        
        if (ncpus > 1) {
            for (int n = 1; n < SPIN_COUNT; n <<= 1) {

                for (int i = 0; i < n; i++) {
                    PAUSE();
                }

                expected = 0;
                if (atomic_compare_exchange_strong(&lock->core, &expected, SPIN_LOCK_BITS) && expected == 0) {
                    return;
                }
            }
        }
        //pthread_yield_np();
        sched_yield();
        //thrd_yield();
    }
}


void yalx_spin_unlock(struct yalx_spin_lock *lock) {
    atomic_store_explicit(&lock->core, 0, memory_order_relaxed);
}
