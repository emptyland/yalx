#pragma once
#ifndef YALX_RUNTIME_HEAP_YGC_MARK_H
#define YALX_RUNTIME_HEAP_YGC_MARK_H

#include "runtime/jobs.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YGC_MAX_MARKING_STRIPES 16

struct yalx_value_any;

struct ygc_marking_stripe {
    uintptr_t *_Atomic stack;
    size_t top;
    size_t max;
};

struct ygc_mark {
    struct yalx_job job;
    struct ygc_marking_stripe stripes[YGC_MAX_MARKING_STRIPES];
};

void ygc_mark_init(struct ygc_mark *mark);
void ygc_mark_final(struct ygc_mark *mark);
void ygc_marking_mark_object(struct ygc_mark *mark, uintptr_t addr);
void ygc_marking_mark(struct ygc_mark *mark);

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_HEAP_YGC_MARK_H
