#include "runtime/locks.h"
#include "runtime/checking.h"
#include <semaphore.h>
#include <errno.h>

void yalx_sem_signal(struct yalx_sem *self, unsigned int n) {
    for (int i = 0; i < n; i++) {
        int rs = sem_post(&self->impl);
        GUARANTEE(rs == 0, "sem_post() fail");
    }
}

void yalx_sem_wait(struct yalx_sem *self) {
    int rs = 0;
    do {
        rs = sem_wait(&self->impl);
    } while (rs != 0 && errno == EINTR);
    GUARANTEE(rs == 0, "sem_wait fail");
}

void yalx_sem_try_wait(struct yalx_sem *self) {
    int rs = 0;
    do {
        rs = sem_trywait(&self->impl);
    } while (rs != 0 && errno == EINTR);
    GUARANTEE(rs == 0, "sem_wait fail");
}