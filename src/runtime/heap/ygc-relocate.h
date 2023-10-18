#pragma once
#ifndef YALX_RUNTIME_HEAP_YGC_RELOCATE_H
#define YALX_RUNTIME_HEAP_YGC_RELOCATE_H

#include "runtime/jobs.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct forwarding;

struct ygc_relocate {
    struct yalx_job job;

};

void ygc_relocate_init(struct ygc_relocate *relocate);
void ygc_relocate_final(struct ygc_relocate *relocate);
uintptr_t ygc_relocate_forward_object(struct ygc_relocate *relocate, struct forwarding *fwd, uintptr_t from_addr);

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_HEAP_YGC_RELOCATE_H
