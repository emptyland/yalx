#include "runtime/object/number.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/type.h"
#include "runtime/heap/heap.h"
#include "runtime/checking.h"
#include <stdio.h>

struct yalx_value_number_l *yalx_new_i16(struct heap *heap, i16_t value) {
    if (value >= -100 && value <= 100) {
        return heap->fast_boxing_numbers.i16_values[value + 100];
    }
    struct yalx_value_number_l *l = yalx_new_small_boxing_number(heap, I16_class);
    l->box.i16 = value;
    return l;
}

struct yalx_value_number_l *yalx_new_u16(struct heap *heap, u16_t value) {
    if (value <= 200) {
        return heap->fast_boxing_numbers.u16_values[value];
    }
    struct yalx_value_number_l *l = yalx_new_small_boxing_number(heap, I16_class);
    l->box.u32 = value;
    return l;
}

struct yalx_value_number_l *yalx_new_i32(struct heap *heap, i32_t value) {
    if (value >= -100 && value <= 100) {
        return heap->fast_boxing_numbers.i32_values[value + 100];
    }
    struct yalx_value_number_l *l = yalx_new_small_boxing_number(heap, I32_class);
    l->box.i32 = value;
    return l;
}

struct yalx_value_number_l *yalx_new_u32(struct heap *heap, u32_t value) {
    if (value <= 200) {
        return heap->fast_boxing_numbers.u32_values[value];
    }
    struct yalx_value_number_l *l = yalx_new_small_boxing_number(heap, U32_class);
    l->box.u32 = value;
    return l;
}

struct yalx_value_number_l *yalx_new_f32(struct heap *heap, f32_t value) {
    if (value == -1.0) {
        return heap->fast_boxing_numbers.f32_values[0];
    } else if (value == 0.0) {
        return heap->fast_boxing_numbers.f32_values[1];
    } else if (value == 1.0) {
        return heap->fast_boxing_numbers.f32_values[2];
    }
    struct yalx_value_number_l *l = yalx_new_small_boxing_number(heap, F32_class);
    l->box.f32 = value;
    return l;
}

struct yalx_value_number_w *yalx_new_i64(struct heap *heap, i64_t value) {
    if (value >= -100 && value <= 100) {
        return heap->fast_boxing_numbers.i64_values[value + 100];
    }
    struct yalx_value_number_w *w = yalx_new_big_boxing_number(heap, I64_class);
    w->box.i64 = value;
    return w;
}

struct yalx_value_number_w *yalx_new_u64(struct heap *heap, u64_t value) {
    if (value <= 200) {
        return heap->fast_boxing_numbers.u64_values[value];
    }
    struct yalx_value_number_w *w = yalx_new_big_boxing_number(heap, U64_class);
    w->box.u64 = value;
    return w;
}

struct yalx_value_number_w *yalx_new_f64(struct heap *heap, f64_t value) {
    if (value == -1.0) {
        return heap->fast_boxing_numbers.f64_values[0];
    } else if (value == 0.0) {
        return heap->fast_boxing_numbers.f64_values[1];
    } else if (value == 1.0) {
        return heap->fast_boxing_numbers.f64_values[2];
    }
    struct yalx_value_number_w *w = yalx_new_big_boxing_number(heap, F64_class);
    w->box.f64 = value;
    return w;
}

struct yalx_value_number_l *yalx_new_small_boxing_number(struct heap *heap, const struct yalx_class *klass) {
    enum yalx_builtin_type ty = yalx_builtin_type(klass);
    DCHECK(ty >= Type_Bool && ty <= Type_F64 && "klass not a number");
    //DCHECK(klass->instance_size <= sizeof(u32_t) && "klass size too big");
    USE(ty);
    
    struct allocate_result result = yalx_heap_allocate(heap, klass, sizeof(struct yalx_value_number_l), 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    struct yalx_value_number_l *l = (struct yalx_value_number_l *)result.object;
    l->box.u32 = 0;
    return l;
}

struct yalx_value_number_w *yalx_new_big_boxing_number(struct heap *heap, const struct yalx_class *klass) {
    enum yalx_builtin_type ty = yalx_builtin_type(klass);
    DCHECK(ty >= Type_Bool && ty <= Type_F64 && "klass not a number");
    //DCHECK(klass->instance_size == sizeof(u64_t) && "klass size too small");
    USE(ty);
    
    struct allocate_result result = yalx_heap_allocate(heap, klass, sizeof(struct yalx_value_number_w), 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    struct yalx_value_number_w *w = (struct yalx_value_number_w *)result.object;
    w->box.u64 = 0;
    return w;
}


void yalx_Zplang_Zolang_ZdI32_ZdtoString_stub(yalx_ref_handle self) {
    assert(!"TODO");
}

void yalx_Zplang_Zolang_ZdU32_ZdtoString_stub(yalx_ref_handle self) {
    assert(!"TODO");
}

void yalx_Zplang_Zolang_Zdu8ToString_stub(u8_t value) {
    yalx_return_ref((yalx_ref_t)yalx_uint_to_string(&heap, value, 10));
}

void yalx_Zplang_Zolang_Zdi8ToString_stub(i8_t value) {
    
    yalx_return_cstring("1", 1);
}

void yalx_Zplang_Zolang_Zdu16ToString_stub(u16_t value) {
    yalx_return_ref((yalx_ref_t)yalx_uint_to_string(&heap, value, 10));
}

void yalx_Zplang_Zolang_Zdi16ToString_stub(i16_t value) {
    yalx_return_ref((yalx_ref_t)yalx_int_to_string(&heap, value, 10));
}

void yalx_Zplang_Zolang_Zdi32ToString_stub(i32_t value) {
    yalx_return_ref((yalx_ref_t)yalx_int_to_string(&heap, value, 10));
}

void yalx_Zplang_Zolang_Zdu32ToString_stub(u32_t value) {
    yalx_return_ref((yalx_ref_t)yalx_uint_to_string(&heap, value, 10));
}

void yalx_Zplang_Zolang_Zdu64ToString_stub(u64_t value) {
    yalx_return_ref((yalx_ref_t)yalx_uint_to_string(&heap, value, 10));
}

void yalx_Zplang_Zolang_Zdi64ToString_stub(i64_t value) {
    yalx_return_ref((yalx_ref_t)yalx_int_to_string(&heap, value, 10));
}

void yalx_Zplang_Zolang_Zdf32ToString_stub(f32_t value) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%f", value);
    yalx_return_cstring(buf, strlen(buf));
}

void yalx_Zplang_Zolang_Zdf64ToString_stub(f64_t value) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%f", value);
    yalx_return_cstring(buf, strlen(buf));
}
