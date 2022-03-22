#pragma once
#ifndef YALX_RUNTIME_OBJECT_ARRAYS_H_
#define YALX_RUNTIME_OBJECT_ARRAYS_H_

#include "runtime/object/any.h"

#ifdef __cplusplus
extern "C" {
#endif

struct yalx_value_typed_array {
    YALX_VALUE_HEADER;
    u32_t len;
    struct yalx_class *item;
    char  data[0];
}; // struct yalx_value_typed_array


struct yalx_value_refs_array {
    YALX_VALUE_HEADER;
    u32_t len;
    yalx_ref_t data[0];
}; // struct yalx_value_refs_array


struct yalx_value_dims_array {
    YALX_VALUE_HEADER;
    u32_t len;
    u32_t dims;
    yalx_ref_t arrays[0];
}; // struct yalx_value_dims_array

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_OBJECT_ARRAYS_H_
