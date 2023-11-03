#include "runtime/mm-thread.h"
#include "runtime/process.h"
#include "runtime/checking.h"
#include "runtime/runtime.h"
#if defined(YALX_OS_POSIX)
#include <sys/mman.h>
#endif // defined(YALX_OS_POSIX)

const char *const YALX_MM_THREAD_NAME = "yalx-mm-thread";

static const char STOP_BODY[4] = "STOP";

struct mm_task {
    struct task_entry task;
    struct yalx_mm_thread *thread;
};

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
    mm->safepoint_count = 0;

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
}

static size_t arm_safepoint(struct yalx_mm_thread *mm) {
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
    mm_arm_polling_page(mm_polling_page, 0/*disarm*/);
    return n_threads;
}

static void wait_all_threads_synchronized(const size_t n_threads) {
    for (;;) {
        size_t still_running = n_threads;

        for (int i = 0; i < nprocs; i++) {
            struct processor *const proc = &procs[i];

            yalx_mutex_lock(&proc->mutex);
            for (struct machine *m = proc->machine_head.next; m != &proc->machine_head; m = m->next) {
                atomic_thread_fence(memory_order_acquire);
                if (m->state == MACH_SAFE_POINT) {
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

    atomic_fetch_add(&mm->safepoint_count, 1);

    GUARANTEE(mm->state == NOT_SYNCHRONIZED, "Synchronize state error: %d", mm->state);

    mm->state = SYNCHRONIZING;
    atomic_thread_fence(memory_order_release);

    size_t n_threads = arm_safepoint(mm);

    wait_all_threads_synchronized(n_threads);

    mm->state = SYNCHRONIZED;
    atomic_thread_fence(memory_order_release);
}

void mm_synchronize_end(struct yalx_mm_thread *mm) {
    // TODO:
    NOT_IMPL();
    yalx_mutex_unlock(&mach_threads_mutex);
}

enum synchronize_state mm_synchronize_state(struct yalx_mm_thread const *mm) {
    atomic_thread_fence(memory_order_acquire);
    return mm->state;
}

#if defined(YALX_OS_POSIX)
void *mm_new_polling_page() {
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