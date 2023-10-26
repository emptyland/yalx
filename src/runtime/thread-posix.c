#include "runtime/thread.h"
#include "runtime/heap/heap.h"
#include "runtime/checking.h"
#include "runtime/runtime.h"
#include <dlfcn.h>
#include <sched.h>
#include <stdlib.h>
#if defined(YALX_OS_DRAWIN)

#endif
#if defined(YALX_OS_LINUX)
#include <sys/prctl.h>
#endif

static yalx_tls_t self_thread;
static _Atomic uint64_t global_next_thread_id = 1;

#if defined(YALX_OS_LINUX)
static int (*yalx_sched_getcpu)(void);
#endif


static void *native_entry(void *ctx) {
    DCHECK(ctx);
    struct os_thread_bundle *bundle = (struct os_thread_bundle *)ctx;
    struct yalx_os_thread *thread = bundle->thread;
    yalx_os_thread_fn entry = bundle->entry;
    void *param = bundle->param;
    char *name = (char *)bundle->name;
    free(bundle);

    yalx_tls_set(self_thread, thread);
#if defined(YALX_OS_LINUX)
    if (name) {
        prctl(PR_SET_NAME, name);
        free(name);
    }
#endif
    if (heap) { heap->thread_enter(heap, thread); }
    entry(param);
    if (heap) { heap->thread_exit(heap, thread); }
    yalx_tls_set(self_thread, NULL);

    return NULL;
}

int yalx_os_threading_env_enter(void) {
    DCHECK(ncpus > 0);
#if defined(YALX_OS_LINUX)
    yalx_sched_getcpu = dlsym((void *)RTLD_LOCAL, "sched_getcpu");
    if (!yalx_sched_getcpu) {
        DLOG(ERROR, "libc function `sched_getcpu' not found!");
    }
#endif
    
#if defined(YALX_OS_DRAWIN)
  
#endif
    return yalx_tls_alloc(&self_thread);
}

void yalx_os_threading_env_exit(void) {
    yalx_tls_free(self_thread);
}

int yalx_os_thread_start(
        struct yalx_os_thread *thread,
        yalx_os_thread_fn entry,
        void *param,
        const char *name,
        const char *file,
        int line) {
    struct os_thread_bundle *bundle = MALLOC(struct os_thread_bundle);
    if (!bundle) {
        return -1;
    }
    assert(entry != NULL);
    bundle->entry = entry;
    bundle->param = param;
    thread->start_point.file = file;
    thread->start_point.line = line;
    thread->id = atomic_fetch_add(&global_next_thread_id, 1);
    bundle->thread = thread;
    bundle->name = strdup(name);

    if (pthread_create(&thread->native_handle, NULL, native_entry, bundle) == 0) {
        return 0;
    }
    free(bundle);
    return -1;
}

int yalx_os_thread_join(struct yalx_os_thread *thread, uint64_t timeout_in_mills) {
    USE(timeout_in_mills);
    void *rt = NULL;
    return pthread_join(thread->native_handle, &rt);
}

struct yalx_os_thread *yalx_os_thread_self(void) {
    return (struct yalx_os_thread *) yalx_tls_get(self_thread);
}

struct yalx_os_thread *yalx_os_thread_attach_self(struct yalx_os_thread *thread) {
    DCHECK(yalx_tls_get(self_thread) == NULL);
    thread->native_handle = pthread_self();
    thread->id = atomic_fetch_add(&global_next_thread_id, 1);
    yalx_tls_set(self_thread, thread);
    if (heap) { heap->thread_enter(heap, thread); }
    return thread;
}

struct yalx_os_thread *yalx_os_thread_detach_self() {
    DCHECK(yalx_tls_get(self_thread) != NULL);
    struct yalx_os_thread *thread = (struct yalx_os_thread *) yalx_tls_get(self_thread);
    if (heap) { heap->thread_exit(heap, thread); }
    yalx_tls_set(self_thread, NULL);
    return thread;
}

struct per_cpu_storage *per_cpu_storage_new(void) {
    DCHECK(ncpus > 0);
    size_t placement_size_in_bytes = sizeof(struct per_cpu_storage) + sizeof(void *) * (ncpus - 1);
    struct per_cpu_storage *storage = (struct per_cpu_storage *) malloc(placement_size_in_bytes);
    if (!storage) {
        return NULL;
    }
    storage->n = ncpus;
    memset(&storage->items[0], 0, sizeof(void *) * storage->n);
    return storage;
}

void per_cpu_storage_free(struct per_cpu_storage *storage) {
    free(storage);
}

int cpu_id(void) {
#if defined(YALX_OS_LINUX)
    int id = !yalx_sched_getcpu ? -1 : (int)yalx_sched_getcpu();
#endif
    
#if defined(YALX_OS_DARWIN)
    int id = 0; // TODO:
    
#endif
    
    DCHECK(id >= 0 && id < ncpus);
    return id;
}
