#pragma once
#ifndef YALX_RUNTIME_HEAP_HEAP_H_
#define YALX_RUNTIME_HEAP_HEAP_H_

#include "runtime/runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

struct yalx_value_any;

enum allocate_status {
    ALLOCATE_OK,
    ALLOCATE_NOTHING,
    // TODO:
};

struct allocate_result {
    struct yalx_value_any *object;
    enum allocate_status status;
}; // struct allocate_result


struct heap {
    // TODO:
}; // struct heap

extern struct heap heap;

int yalx_init_heap(struct heap *heap);
void yalx_free_heap(struct heap *heap);

struct allocate_result yalx_heap_allocate(size_t size, u32_t flags);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_HEAP_HEAP_H_
