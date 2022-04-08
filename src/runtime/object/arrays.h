#pragma once
#ifndef YALX_RUNTIME_OBJECT_ARRAYS_H_
#define YALX_RUNTIME_OBJECT_ARRAYS_H_

#include "runtime/object/any.h"

#ifdef __cplusplus
extern "C" {
#endif

struct yalx_class;
struct heap;

struct yalx_value_typed_array {
    YALX_VALUE_HEADER;
    u32_t len;
    const struct yalx_class *item;
    char  data[0];
}; // struct yalx_value_typed_array


struct yalx_value_refs_array {
    YALX_VALUE_HEADER;
    u32_t len;
    const struct yalx_class *item;
    yalx_ref_t data[0];
}; // struct yalx_value_refs_array


struct yalx_value_dims_array {
    YALX_VALUE_HEADER;
    u32_t len;
    u32_t dims;
    yalx_ref_t arrays[0];
}; // struct yalx_value_dims_array

struct yalx_value_array_header {
    YALX_VALUE_HEADER;
    u32_t len;
}; // struct yalx_value_array_header

struct yalx_value_refs_array *yalx_new_refs_array(struct heap *heap, const struct yalx_class *item, yalx_ref_t *data,
                                                  size_t nitems);

struct yalx_value_array_header *yalx_cast_to_array_if_possibly(yalx_ref_t obj);

static inline int yalx_is_array(yalx_ref_t obj) { return yalx_cast_to_array_if_possibly(obj) != NULL; }
static inline int yalx_is_not_array(yalx_ref_t obj) { return !yalx_is_array(obj); }

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_OBJECT_ARRAYS_H_
