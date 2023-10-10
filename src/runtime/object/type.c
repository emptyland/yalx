#include "runtime/object/type.h"
#include "runtime/object/any.h"
#include "runtime/object/arrays.h"
#include "runtime/object/yalx-string.h"
#include "runtime/checking.h"

int yalx_is_ref_type(const struct yalx_class * klass) {
    enum yalx_builtin_type maybe_builtin_ty = yalx_builtin_type(klass);
    switch (maybe_builtin_ty) {
        case Type_any:
        case Type_string:
        case Type_array:
        case Type_multi_dims_array:
            return 1;
            
        case NOT_BUILTIN_TYPE:
            return klass->constraint == K_CLASS;

        default:
            return 0;
    }
}


int yalx_is_compact_enum_type_fallback(const struct yalx_class * klass) {
    if (klass->constraint == K_ENUM && klass->instance_size == sizeof(yalx_ref_t)) {
        if (klass->n_methods == 2) {
            int refs_ty_count = 0;
            int none_ty_count = 0;
            for (int i = 0; i < klass->n_methods; i++) {
                if (!klass->fields[i].type) {
                    none_ty_count++;
                } else if (yalx_is_ref_type(klass->fields[i].type)) {
                    refs_ty_count++;
                }
            }
            return refs_ty_count == 1 && none_ty_count == 1;
        }
    }
    return 0;
}


size_t class_ty_size(const struct yalx_class *klass, const struct yalx_value_any *obj) {
    DCHECK(yalx_is_ref_type(klass));
    DCHECK(klass == CLASS(obj));
    return (size_t)klass->instance_size;
}

size_t string_ty_size(const struct yalx_class *klass, const struct yalx_value_str *obj) {
    DCHECK(klass->id == Type_string);
    DCHECK(klass == CLASS(obj));
    return yalx_reserve_string_bytes(obj->bytes, obj->len);
}

size_t array_ty_size(const struct yalx_class *klass, const struct yalx_value_array *obj) {
    DCHECK(klass->id == Type_array);
    DCHECK(klass == CLASS(obj));

    const struct yalx_class *item = obj->item;
    const size_t item_size = yalx_is_ref_type(item) ? item->reference_size : item->instance_size;
    return sizeof(struct yalx_value_array) + (obj->len * item_size);
}

size_t multi_dims_array_ty_size(const struct yalx_class *klass, const struct yalx_value_multi_dims_array *obj) {
    DCHECK(klass->id == Type_array);
    DCHECK(klass == CLASS(obj));

    size_t nitems = obj->caps[0];
    for (u32_t i = 1; i < obj->rank; i++) {
        nitems *= obj->caps[i];
    }

    const struct yalx_class *item = obj->item;
    const size_t item_size = yalx_is_ref_type(item) ? item->reference_size : item->instance_size;
    const size_t size = sizeof(struct yalx_value_multi_dims_array) // header
                        + (obj->rank - 1) * sizeof(u32_t) // caps
                        + (nitems * item_size);      // data of items
    return size;
}