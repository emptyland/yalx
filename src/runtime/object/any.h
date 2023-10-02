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
    u32_t     refs

struct yalx_value_any {
    YALX_VALUE_HEADER;
}; // struct yalx_value_any

struct yalx_class;
struct yalx_value_str;

#define CLASS(ptr) ((struct yalx_class *)(((yalx_ref_t)ptr)->klass & (~0x1)))

typedef struct yalx_value_any *yalx_ref_t;
typedef struct yalx_value_any **yalx_ref_handle;

struct yalx_value_interface {
    yalx_ref_t onws;
    address_t  itab;
}; // struct yalx_val_interface

#define INTERFACE_OFFSET_OF_OWNS offsetof(struct yalx_value_interface, owns)
#define INTERFACE_OFFSET_OF_ITAB offsetof(struct yalx_value_interface, itab)

struct yalx_value_closure {
    YALX_VALUE_HEADER;
    address_t entry;
};

#define CLOSURE_OFFSET_OF_ENTRY offsetof(struct yalx_value_closure, entry)

// reference by any-[os]-[arch].s
struct yalx_value_str *yalx_any_to_string(struct yalx_value_any *any);

size_t yalx_object_size_in_bytes(yalx_ref_t obj);

#define ANY_OFFSET_OF_KLASS offsetof(struct yalx_value_any, klass)

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_OBJECT_ANY_H_
