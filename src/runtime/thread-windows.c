#include "runtime/thread.h"
#include "runtime/checking.h"

static yalx_tls_t self_thread = TLS_OUT_OF_INDEXES;

static WINAPI DWORD native_entry(void *ctx) {
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
    DCHECK(self_thread == TLS_OUT_OF_INDEXES);
    return yalx_tls_alloc(&self_thread);
}

void yalx_os_threading_env_exit() {
    DCHECK(self_thread != TLS_OUT_OF_INDEXES);
    yalx_tls_free(self_thread);
    self_thread = TLS_OUT_OF_INDEXES;
}

int yalx_os_thread_start(
        struct yalx_os_thread *thread,
        yalx_os_thread_fn entry,
        void *param,
        const char *name,
        const char *file,
        int line) {
    struct os_thread_bundle *bundle = (struct os_thread_bundle *)malloc(sizeof(struct os_thread_bundle));
    if (!bundle) {
        return -1;
    }
    assert(entry != NULL);
    bundle->entry = entry;
    bundle->param = param;
    thread->start_point.file = file;
    thread->start_point.line = line;
    bundle->thread = thread;

    thread->native_handle = CreateThread(NULL, 0, &native_entry, bundle, 0, &thread->native_id);
    if (thread->native_handle == INVALID_HANDLE_VALUE) {
        free(bundle);
        return -1;
    }
    return 0;
}

struct yalx_os_thread *yalx_os_thread_self() {
    void *const val = yalx_tls_get(self_thread);
    DCHECK(val != NULL);
    return (struct yalx_os_thread *) val;
}

struct yalx_os_thread *yalx_os_thread_self_or_null(void) {
    return (struct yalx_os_thread *) yalx_tls_get(self_thread);
}

struct yalx_os_thread *yalx_os_thread_attach_self(struct yalx_os_thread *thread) {
    DCHECK(yalx_tls_get(self_thread) == NULL);
    thread->native_handle = GetCurrentThread();
    thread->native_id = GetThreadId(thread->native_handle);
    yalx_tls_set(self_thread, thread);
    return thread;
}

struct yalx_os_thread *yalx_os_thread_detach_self(struct yalx_os_thread *thread) {

}