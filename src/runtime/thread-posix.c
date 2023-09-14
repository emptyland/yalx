#include "runtime/thread.h"
#include "runtime/checking.h"
#include "runtime/runtime.h"
#include <stdlib.h>

static yalx_tls_t self_thread;

static int native_entry(void *ctx) {
    DCHECK(ctx);
    struct os_thread_bundle *bundle = (struct os_thread_bundle *)ctx;
    struct yalx_os_thread *thread = bundle->thread;
    yalx_os_thread_fn entry = bundle->entry;
    void *param = bundle->param;
    free(bundle);

    yalx_tls_set(self_thread, thread);
    entry(param);
    yalx_tls_set(self_thread, NULL);

    return 0;
}

int yalx_os_threading_env_enter() {
    return yalx_tls_alloc(&self_thread);
}

void yalx_os_threading_env_exit() {
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
    bundle->thread = thread;

    int rs = thrd_create(&thread->native_handle, native_entry, bundle);
    if (rs == thrd_success) {
        return 0;
    }
    free(bundle);
    return -1;
}

struct yalx_os_thread *yalx_os_thread_self() {
    return (struct yalx_os_thread *) yalx_tls_get(self_thread);
}

struct yalx_os_thread *yalx_os_thread_attach_self(struct yalx_os_thread *thread) {
    DCHECK(yalx_tls_get(self_thread) == NULL);
    thread->native_handle = thrd_current();
    //thread->native_id = GetThreadId(thread->native_handle);
    yalx_tls_set(self_thread, thread);
    return thread;
}