#pragma once
#ifndef YALX_RUNTIME_OBJECT_ANY_H_
#define YALX_RUNTIME_OBJECT_ANY_H_

#include "runtime/runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

#define YALX_VALUE_HEADER \
    uintptr_t klass;      \
    u32_t     tags;       \
    u32_t     oid

struct yalx_value_any {
    YALX_VALUE_HEADER;
}; // struct yalx_value_any

struct yalx_class;

#define CLASS(ptr) ((struct yalx_class *)(((yalx_ref_t)ptr)->klass & (~0x1)))

typedef struct yalx_value_any *yalx_ref_t;


//struct yalx_value_any EmptyTag;

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_OBJECT_ANY_H_
