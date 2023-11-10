#include "runtime/mm-thread.h"
#include "runtime/process.h"
#include "runtime/checking.h"
#include "runtime/runtime.h"
#include "runtime/utils.h"
#if defined(YALX_OS_POSIX)
#include <sys/mman.h>
#include <pthread.h>
#endif // defined(YALX_OS_POSIX)

const char *const YALX_MM_THREAD_NAME = "yalx-mm-thread";

static const char STOP_BODY[4] = "STOP";

static void wait_barrier_init(struct mm_wait_barrier *barrier) {
    barrier->barrier_tag = 0;
    barrier->waiters = 0;
    barrier->barrier_threads = 0;
    yalx_sem_init(&barrier->sem, 0);
}

static void wait_barrier_final(struct mm_wait_barrier *barrier) {
    yalx_sem_final(&barrier->sem);
}

static void wait_barrier_arm(struct mm_wait_barrier *barrier, int barrier_tag) {
    DCHECK(barrier->barrier_tag == 0);
    DCHECK(barrier->waiters == 0);
    barrier->barrier_tag = barrier_tag;
    barrier->waiters = 0;
    atomic_thread_fence(memory_order_release);
}

static int wait_if_needed(struct mm_wait_barrier *barrier) {
    DCHECK(barrier->barrier_tag == 0);
    int w = barrier->waiters;
    if (w == 0) {
        atomic_thread_fence(memory_order_seq_cst);
        return 0;
    }

    int expected = w;
    if (atomic_compare_exchange_strong(&barrier->waiters, &expected, w - 1)) {
        yalx_sem_signal(&barrier->sem, 1);
        return w - 1;
    }
    return w;
}

static void wait_barrier_disarm(struct mm_wait_barrier *barrier) {
    DCHECK(barrier->barrier_tag != 0);
    barrier->barrier_tag = 0;
    atomic_thread_fence(memory_order_seq_cst);

    int left;
    do {
        left = wait_if_needed(barrier);
        if (left == 0 && barrier->barrier_threads > 0) {
            sched_yield();
        }
    } while (left > 0 || barrier->barrier_threads > 0);

    atomic_thread_fence(memory_order_seq_cst);
}

static void wait_barrier_wait(struct mm_wait_barrier *barrier, int barrier_tag) {
    DCHECK(barrier->barrier_tag != 0);
    if (barrier->barrier_tag != barrier_tag) {
        atomic_thread_fence(memory_order_seq_cst);
        return;
    }

    atomic_fetch_add(&barrier->barrier_threads, 1);
    if (barrier_tag != 0 && barrier_tag == barrier->barrier_tag) {
        atomic_fetch_add(&barrier->waiters, 1);
        yalx_sem_wait(&barrier->sem);
        wait_if_needed(barrier);
    }
    atomic_fetch_sub(&barrier->barrier_threads, 1);
}

static int task_is_stop(struct task_entry *task) {
    const uint32_t stop_code = *(const int *)STOP_BODY;
    return (task->run == NULL && task->param1 == stop_code && task->param2 == stop_code);
}

static void mm_thread_entry(struct yalx_mm_thread *mm) {
    DLOG(INFO, "%s start ... %p", YALX_MM_THREAD_NAME, mm);
    for (;;) {
        struct task_entry *task = NULL;
        task_queue_take(&mm->worker_queue, &task);
        if (task_is_stop(task)) {
            task_final(&mm->worker_queue, task);
            break;
        }

        task->run(task);
        task_final(&mm->worker_queue, task);

        atomic_fetch_sub_explicit(&mm->active, 1, memory_order_acq_rel);
    }

    DLOG(INFO, "%s should be stop... is shutting down: %d", YALX_MM_THREAD_NAME, mm_thread_is_shutting_down(mm));
}


int yalx_mm_thread_start(struct yalx_mm_thread *mm) {
    mm->shutting_down = 0;
    mm->state = NOT_SYNCHRONIZED;
    mm->safepoint_counter = 0;
    wait_barrier_init(&mm->wait_barrier);

    task_queue_init(&mm->worker_queue);

    int rs = yalx_os_thread_start(&mm->thread, (yalx_os_thread_fn) mm_thread_entry, mm,
                                  YALX_MM_THREAD_NAME,__FILE__, __LINE__);
    if (rs < 0) {
        return rs;
    }

    return 0;
}

struct yalx_mm_thread *mm_thread_of_task(struct task_entry *task) {
    return ((struct mm_task *)task)->thread;
}

void mm_thread_wait_inactive(struct yalx_mm_thread *mm) {
    DLOG(INFO, "active: %d", mm->active);
    while (atomic_load_explicit(&mm->active, memory_order_acquire)) {
        sched_yield();
    }
}

int mm_thread_is_shutting_down(struct yalx_mm_thread *mm) {
    return atomic_load_explicit(&mm->shutting_down, memory_order_acquire);
}

void mm_thread_post_routine_to(struct yalx_mm_thread *mm, task_run_fn_t routine, void *params) {
    atomic_fetch_add_explicit(&mm->active, 1, memory_order_acq_rel);
    struct mm_task *task = (struct mm_task *) task_alloc(&mm->worker_queue, sizeof(struct mm_task));
    task->task.ctx = params;
    task->task.run = routine;
    task->thread = mm;
    task_queue_post(&mm->worker_queue, &task->task);
}

void yalx_mm_thread_shutdown(struct yalx_mm_thread *mm) {
    const uint32_t stop_code = *(const int *)STOP_BODY;
    struct task_entry *task = task_new(&mm->worker_queue, NULL, NULL, NULL);
    task->param1 = stop_code;
    task->param2 = stop_code;

    atomic_store_explicit(&mm->shutting_down, 1, memory_order_release);
    task_queue_post(&mm->worker_queue, task);

    yalx_os_thread_join(&mm->thread, 0);
    wait_barrier_final(&mm->wait_barrier);
}

static size_t arm_safepoint(struct yalx_mm_thread *mm) {
    wait_barrier_arm(&mm->wait_barrier, mm->safepoint_counter + 1);

    DCHECK((mm->safepoint_counter & 0x1) == 0 && "safepoint_counter must be even");
    atomic_fetch_add_explicit(&mm->safepoint_counter, 1, memory_order_release);

    mm->state = SYNCHRONIZING;
    atomic_thread_fence(memory_order_release);

    size_t n_threads = 0;
    for (int i = 0; i < nprocs; i++) {
        struct processor *const proc = &procs[i];

        yalx_mutex_lock(&proc->mutex);
        for (struct machine *m = proc->machine_head.next; m != &proc->machine_head; m = m->next) {
            m->polling_page = (address_t)mm_polling_page;
            n_threads++;
        }
        yalx_mutex_unlock(&proc->mutex);
    }
    mm_arm_polling_page(mm_polling_page, 0/*armed*/);
    return n_threads;
}

static void disarm_safepoint(struct yalx_mm_thread *mm) {
    atomic_thread_fence(memory_order_acquire);
    DCHECK(mm->state == SYNCHRONIZED);
    
    mm->state = NOT_SYNCHRONIZED;

    DCHECK((mm->safepoint_counter & 0x1) == 1 && "safepoint_counter must be odd");
    atomic_fetch_add_explicit(&mm->safepoint_counter, 1, memory_order_release);
    
    atomic_thread_fence(memory_order_release);
    
    for (int i = 0; i < nprocs; i++) {
        struct processor *const proc = &procs[i];

        yalx_mutex_lock(&proc->mutex);
        for (struct machine *m = proc->machine_head.next; m != &proc->machine_head; m = m->next) {
            m->polling_page = (address_t)mm_polling_page;
        }
        yalx_mutex_unlock(&proc->mutex);
    }
    
    mm_arm_polling_page(mm_polling_page, 1/*armed*/);

    wait_barrier_disarm(&mm->wait_barrier);
}

static void wait_all_threads_synchronized(const size_t n_threads) {
    for (;;) {
        size_t still_running = n_threads;

        for (int i = 0; i < nprocs; i++) {
            struct processor *const proc = &procs[i];

            yalx_mutex_lock(&proc->mutex);
            for (struct machine *m = proc->machine_head.next; m != &proc->machine_head; m = m->next) {
                atomic_thread_fence(memory_order_acquire);
                if (m->state != MACH_RUNNING && m->state != MACH_INIT) {
                    still_running--;
                }
                sched_yield();
            }
            yalx_mutex_unlock(&proc->mutex);
        }

        if (!still_running) {
            break;
        }
    }
}

void mm_synchronize_begin(struct yalx_mm_thread *mm) {
    // TODO:
    yalx_mutex_lock(&mach_threads_mutex);

    GUARANTEE(mm->state == NOT_SYNCHRONIZED, "Synchronize state error: %d", mm->state);

    size_t n_threads = arm_safepoint(mm);
    DLOG(INFO, "Arm safe-point threads: %zd", n_threads);

    wait_all_threads_synchronized(n_threads);

    mm->state = SYNCHRONIZED;
    atomic_thread_fence(memory_order_release);
}

void mm_synchronize_end(struct yalx_mm_thread *mm) {
    disarm_safepoint(mm);
    
    yalx_mutex_unlock(&mach_threads_mutex);
}

#define SPIN_COUNT 4096
#define PAUSE() (void)0

double mm_synchronize_poll(struct yalx_mm_thread *mm) {
    struct machine *mach = thread_local_mach;
    enum machine_state old_state = !mach ? MACH_INIT : mach->state;
    int safepoint_id = mm->safepoint_counter;

    int retry = 0;
    double jiffy = yalx_current_mills_in_precision();
    while (mm_synchronize_state(mm) != NOT_SYNCHRONIZED) {
        if (!retry && mach) {
            int rs = atomic_compare_exchange_strong(&mach->state, &old_state, MACH_SAFE);
            GUARANTEE(rs, "Mach state changed, new value: %d", old_state);
        }

        retry++;
        for (int n = 1; n < SPIN_COUNT; n <<= 1) {

            for (int i = 0; i < n; i++) {
                PAUSE();
            }
            if (mm_synchronize_state(mm) == NOT_SYNCHRONIZED) {
                goto end;
            }
            sched_yield();
        }

        DCHECK(mm->state == SYNCHRONIZED);
        wait_barrier_wait(&mm->wait_barrier, safepoint_id);
        DCHECK(mm->state != SYNCHRONIZED);

    }
end:
    if (retry > 0 && mach) {
        enum machine_state expected = MACH_SAFE;
        int rs = atomic_compare_exchange_strong(&mach->state, &expected, old_state);
        GUARANTEE(rs, "Mach state changed, actual value is: %d, should be \'MACH_SAFE_POINT\'", expected);
    }
    return !retry ? 0 : yalx_current_mills_in_precision() - jiffy;
}

void mm_synchronize_handle(struct yalx_mm_thread *mm, struct machine *mach) {
    DCHECK(mm->state == SYNCHRONIZED);
    int safepoint_id = atomic_load_explicit(&mm->safepoint_counter, memory_order_acquire);

    enum machine_state old_state = mach->state;

    int rs = atomic_compare_exchange_strong(&mach->state, &old_state, MACH_SAFE);
    GUARANTEE(rs, "Mach state changed, new value: %d", old_state);

    wait_barrier_wait(&mm->wait_barrier, safepoint_id);

    enum machine_state expected = MACH_SAFE;
    rs = atomic_compare_exchange_strong(&mach->state, &expected, old_state);
    GUARANTEE(rs, "Mach state changed, actual value is: %d, should be \'MACH_SAFE_POINT\'", expected);

    DLOG(INFO, "mm_synchronize_handle()...ok");
}

enum synchronize_state mm_synchronize_state(struct yalx_mm_thread const *mm) {
    atomic_thread_fence(memory_order_acquire);
    return mm->state;
}

#if defined(YALX_OS_POSIX)
void *mm_new_polling_page(void) {
    void *page = mmap(NULL, os_page_size, PROT_READ, MAP_ANON|MAP_PRIVATE, -1, 0);
    GUARANTEE(page != MAP_FAILED, "Polling page allocating fail!");
    return page;
}

void mm_free_polling_page(void *page) {
    if (!page) {
        return;
    }
    munmap(page, os_page_size);
}

void mm_arm_polling_page(void *page, int armed) {
    int rs = mprotect(page, os_page_size, armed ? PROT_READ : PROT_NONE);
    GUARANTEE(rs == 0, "Arm polling page fail! %d", rs);
}
#endif

int mm_is_polling_page(void *p) {
    address_t addr = (address_t)p;
    return addr >= mm_polling_page && addr < mm_polling_page + os_page_size;
}