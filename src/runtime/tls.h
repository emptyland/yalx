#pragma once
#ifndef YALX_RUNTIME_TLS_H
#define YALX_RUNTIME_TLS_H

#include "runtime/macros.h"

struct yalx_os_thread;
struct machine;

struct thread_local_slots {
    struct machine *m;
    struct yalx_os_thread *self_thread;
};

#endif //YALX_RUNTIME_TLS_H
