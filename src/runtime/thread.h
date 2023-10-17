#pragma once
#ifndef YALX_RUNTIME_THREAD_H
#define YALX_RUNTIME_THREAD_H

#include "runtime/macros.h"
#if defined(YALX_OS_POSIX)
    #ifndef __STDC_NO_THREADS__
        #include <threads.h>
    #endif
#include <pthread.h>
#endif
#if defined(YALX_OS_WINDOWS)
#include <Windows.h>
#endif
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*yalx_os_thread_fn)(void *);

struct yalx_os_thread {
#if defined(YALX_OS_WINDOWS)
    HANDLE native_handle;
    DWORD  native_id;
#elif defined(YALX_OS_POSIX)
    pthread_t native_handle;
#endif
    uint64_t id;
    struct {
        const char *file;
        int line;
    } start_point;
};

struct os_thread_bundle {
    struct yalx_os_thread *thread;
    const char *name;
    yalx_os_thread_fn entry;
    void *param;
};

#if defined(YALX_OS_WINDOWS)

#define INIT_OS_THREAD { INVALID_HANDLE_VALUE, 0, {NULL, 0} }

typedef DWORD yalx_tls_t;

static inline int yalx_tls_alloc(yalx_tls_t *key) {
    DWORD index = TlsAlloc();
    if (index == TLS_OUT_OF_INDEXES) {
        return -1;
    }
    *key = index;
    return 0;
}

#define yalx_tls_set(tls, value) TlsSetValue(tls, (void *)(value))
#define yalx_tls_get(tls) ((void *)TlsGetValue(tls))
#define yalx_tls_free(tls) TlsFree(tls)

#elif defined(YALX_OS_POSIX) // defined(YALX_OS_WINDOWS)
// pthread_key_t

#ifndef __STDC_NO_THREADS__ // std thread.h

typedef tss_t yalx_tls_t;

static inline int yalx_tls_alloc(yalx_tls_t *key) {
    return tss_create(key, NULL) == thrd_success ? 0 : -1;
}

#define yalx_tls_set(tls, value) tss_set(tls, (void *)(value))
#define yalx_tls_get(tls) ((void *)tss_get(tls))
#define yalx_tls_free(tls) tss_delete(tls)

#else // pthread

typedef pthread_key_t yalx_tls_t;

static inline int yalx_tls_alloc(yalx_tls_t *key) {
    return pthread_key_create(key, NULL);
}

#define yalx_tls_set(tls, value) pthread_setspecific(tls, (void *)(value))
#define yalx_tls_get(tls) ((void *)pthread_getspecific(tls))
#define yalx_tls_free(tls) pthread_key_delete(tls)

#endif

#endif // defined(YALX_OS_POSIX)

int yalx_os_threading_env_enter(void);
void yalx_os_threading_env_exit(void);

int yalx_os_thread_start(
        struct yalx_os_thread *thread,
        yalx_os_thread_fn entry,
        void *param,
        const char *name,
        const char *file,
        int line);

int yalx_os_thread_join(struct yalx_os_thread *thread, uint64_t timeout_in_mills);

struct yalx_os_thread *yalx_os_thread_self(void);
struct yalx_os_thread *yalx_os_thread_attach_self(struct yalx_os_thread *thread);


struct per_cpu_storage {
    int n;
    void *items[1];
};

struct per_cpu_storage *per_cpu_storage_new(void);
void per_cpu_storage_free(struct per_cpu_storage *storage);

int cpu_id(void);

#define per_cpu_at(ty, storage) ((ty)(&(storage)->items[cpu_id()]))
#define per_cpu_get(ty, storage) ((ty)((storage)->items[cpu_id()]))
#define per_cpu_set(storage, item) (storage)->items[cpu_id()] = (item)

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_THREAD_H
