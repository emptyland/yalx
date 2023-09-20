#pragma once
#ifndef YALX_RUNTIME_TLS_H
#define YALX_RUNTIME_TLS_H

#include "runtime/macros.h"

struct yalx_os_thread;
struct machine;
struct ygc_page;

struct thread_local_slots {
    struct machine *m; // Not used yet
    struct yalx_os_thread *self_thread; // Not used yet
};

#endif //YALX_RUNTIME_TLS_H
