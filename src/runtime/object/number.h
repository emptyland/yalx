#pragma once
#ifndef YALX_RUNTIME_NUMBER_ANY_H_
#define YALX_RUNTIME_NUMBER_ANY_H_

#include "runtime/object/any.h"

#ifdef __cplusplus
extern "C" {
#endif


// ---------------------------------------------------------------------------------------------------------------------
// Boxing's number types
// ---------------------------------------------------------------------------------------------------------------------

struct heap;
struct yalx_class;

// For small number types:
// bool, i8~32, u8~32, f32
struct yalx_value_number_l {
    YALX_VALUE_HEADER;
    union {
        i32_t i32;
        u32_t u32;
        f32_t f32;
    } box;
}; // struct yalx_value_number_l

// For big number types:
// bool, i64, u64, f64
struct yalx_value_number_w {
    YALX_VALUE_HEADER;
    union {
        i64_t i64;
        u64_t u64;
        f64_t f64;
    } box;
}; // struct yalx_value_number_w


struct yalx_value_number_l *yalx_new_i16(struct heap *heap, i16_t value);
struct yalx_value_number_l *yalx_new_u16(struct heap *heap, u16_t value);

struct yalx_value_number_l *yalx_new_i32(struct heap *heap, i32_t value);
struct yalx_value_number_l *yalx_new_u32(struct heap *heap, u32_t value);
struct yalx_value_number_l *yalx_new_f32(struct heap *heap, f32_t value);

struct yalx_value_number_w *yalx_new_i64(struct heap *heap, i64_t value);
struct yalx_value_number_w *yalx_new_u64(struct heap *heap, u64_t value);
struct yalx_value_number_w *yalx_new_f64(struct heap *heap, f64_t value);

struct yalx_value_number_l *yalx_new_small_boxing_number(struct heap *heap, const struct yalx_class *klass);
struct yalx_value_number_w *yalx_new_big_boxing_number(struct heap *heap, const struct yalx_class *klass);

#ifdef __cplusplus
}
#endif


#endif // YALX_RUNTIME_NUMBER_ANY_H_
