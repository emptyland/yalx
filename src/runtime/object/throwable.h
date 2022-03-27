#pragma once
#ifndef YALX_RUNTIME_OBJECT_THROWABLE_H_
#define YALX_RUNTIME_OBJECT_THROWABLE_H_

#include "runtime/object/arrays.h"
#include "runtime/object/any.h"

#ifdef __cplusplus
extern "C" {
#endif

struct backtrace_frame {
    YALX_VALUE_HEADER;
    address_t address;
    struct yalx_value_str *function;
    struct yalx_value_str *file;
    u32_t line;
}; // struct yalx_value_typed_array

struct yalx_value_throwable {
    YALX_VALUE_HEADER;
    struct yalx_value_str *message;
    struct yalx_value_throwable *linked;
    struct yalx_value_typed_array *backtrace; // backtrace_frame[]
}; // struct yalx_value_throwable

struct yalx_value_exception {
    YALX_VALUE_HEADER;
    struct yalx_value_str *message;
    struct yalx_value_throwable *linked;
    struct yalx_value_typed_array *backtrace; // backtrace_frame[]
}; // struct yalx_value_throwable

typedef struct yalx_value_throwable **yalx_throwable_handle;
typedef struct yalx_value_exception **yalx_exception_handle;

void throw_bad_casting_exception(const struct yalx_class *const from, const struct yalx_class *const to);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_OBJECT_THROWABLE_H_
