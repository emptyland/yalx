#pragma once
#ifndef YALX_RUNTIME_HEAP_YGC_DRIVER_H
#define YALX_RUNTIME_HEAP_YGC_DRIVER_H

struct heap;
struct collected_statistics;

#ifdef __cplusplus
extern "C" {
#endif

void ygc_gc_sync(struct heap *h, struct collected_statistics *stat);

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_HEAP_YGC_DRIVER_H
