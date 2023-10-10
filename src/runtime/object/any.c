#include "runtime/object/any.h"
#include "runtime/object/arrays.h"
#include "runtime/object/type.h"
#include "runtime/checking.h"
#include <stdio.h>


size_t yalx_object_size_in_bytes(yalx_ref_t obj) {
    const struct yalx_class *klass = CLASS(obj);

    switch (klass->id) {
        case Type_string:
            return string_ty_size(klass, (const struct yalx_value_str *) obj);
        case Type_array:
            return array_ty_size(klass, (const struct yalx_value_array *) obj);
        case Type_multi_dims_array:
            return multi_dims_array_ty_size(klass, (const struct yalx_value_multi_dims_array *) obj);
    }
    return class_ty_size(klass, obj);
}

struct yalx_value_str *yalx_any_to_string(struct yalx_value_any *any) {
    NOREACHABLE();
    return NULL;
}

void yalx_Zplang_Zolang_ZdAny_ZdhashCode_stub(yalx_ref_handle self) {
    uintptr_t val = (uintptr_t)*self;
    yalx_return_i32((val >> 2) & 0xffffffff);
}

void yalx_Zplang_Zolang_ZdAny_Zdid_stub(yalx_ref_handle self) {
    yalx_return_i32((*self)->refs);
}

void yalx_Zplang_Zolang_ZdAny_ZdisEmpty_stub(yalx_ref_handle self) {
    yalx_return_i32(1);
}

void yalx_Zplang_Zolang_ZdAny_ZdtoString_stub(yalx_ref_handle self) {
    char buf[64] = {0};
    snprintf(buf, 64, "any@%p", *self);
    yalx_return_cstring(buf, strlen(buf));
}

