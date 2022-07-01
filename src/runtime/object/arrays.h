#pragma once
#ifndef YALX_RUNTIME_OBJECT_ARRAYS_H_
#define YALX_RUNTIME_OBJECT_ARRAYS_H_

#include "runtime/object/any.h"

#ifdef __cplusplus
extern "C" {
#endif

struct yalx_class;
struct heap;

struct yalx_value_array {
    YALX_VALUE_HEADER;
    const struct yalx_class *item;
    u32_t len;
    char  data[0];
}; // struct yalx_value_typed_array

struct yalx_value_multi_dims_array {
    YALX_VALUE_HEADER;
    const struct yalx_class *item;
    u32_t len; // total-size
    u32_t dims; // number of dimensions
    u32_t caps[1]; // number of capacities each dimensions
}; // struct yalx_value_multi_dims_array

struct yalx_value_array_header {
    YALX_VALUE_HEADER;
    const struct yalx_class *item;
    u32_t len;
}; // struct yalx_value_array_header

struct yalx_value_array *
yalx_new_refs_array_with_data(struct heap *heap, const struct yalx_class *item, yalx_ref_t *data, size_t nitems);

struct yalx_value_array *
yalx_new_vals_array_with_data(struct heap *heap, const struct yalx_class *item, const void *data, size_t nitems);

struct yalx_value_array *yalx_new_array(struct heap *heap, const struct yalx_class *item, size_t nitems);

struct yalx_value_multi_dims_array *
yalx_new_multi_dims_array(struct heap *heap, const struct yalx_class *item, u32_t dims, const u32_t *caps);

struct yalx_value_array_header *yalx_cast_to_array_if_possibly(yalx_ref_t obj);

static inline int yalx_is_array(yalx_ref_t obj) { return yalx_cast_to_array_if_possibly(obj) != NULL; }
static inline int yalx_is_not_array(yalx_ref_t obj) { return !yalx_is_array(obj); }

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_OBJECT_ARRAYS_H_
