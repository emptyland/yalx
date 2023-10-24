#include "runtime/object/any.h"
#include "runtime/object/arrays.h"
#include "runtime/object/type.h"
#include "runtime/heap/object-visitor.h"
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

void struct_shallow_visit(yalx_ref_t host, address_t base, const struct yalx_class *ty,
                          struct yalx_object_visitor *visitor) {
    DCHECK(ty->constraint == K_STRUCT);
    for (int i = 0; i < ty->n_fields; i++) {
        struct yalx_class_field const *field = &ty->fields[i];

        if (yalx_is_ref_type(field->type) || yalx_is_compact_enum_type(field->type)) {
            visitor->visit_pointer(visitor, host, (yalx_ref_t *)(base + field->offset_of_head));
        } else if (field->type->constraint == K_STRUCT) {
            struct_shallow_visit(host, base + field->offset_of_head, field->type, visitor);
        } else if (field->type->constraint == K_ENUM) {
            UNREACHABLE();
        }
    }
}

void yalx_object_shallow_visit(yalx_ref_t obj, struct yalx_object_visitor *visitor) {
    const struct yalx_class *klass = CLASS(obj);

    switch (klass->id) {
        case Type_array: {
            struct yalx_value_array const *arr = (struct yalx_value_array *)obj;
            struct yalx_class const *item_ty = arr->item;
            if (yalx_is_ref_type(item_ty) || yalx_is_compact_enum_type(item_ty)) {
                yalx_ref_t *begin = (yalx_ref_t *)arr->data;
                yalx_ref_t *end = begin + arr->len;
                visitor->visit_pointers(visitor, obj, begin, end);
            } else if (item_ty->constraint == K_STRUCT) {
                address_t base = (address_t)arr->data;
                for (int i = 0; i < arr->len; i++) {
                    struct_shallow_visit(obj, base, item_ty, visitor);
                    base += item_ty->instance_size;
                }
            } else if (item_ty->constraint == K_ENUM) {
                UNREACHABLE();
            }
        } break;

        case Type_multi_dims_array: {
            struct yalx_value_multi_dims_array *arr = (struct yalx_value_multi_dims_array *)obj;
            struct yalx_class const *item_ty = arr->item;
            if (yalx_is_ref_type(item_ty) || yalx_is_compact_enum_type(item_ty)) {
                yalx_ref_t *begin = (yalx_ref_t *)yalx_multi_dims_array_data(arr);
                yalx_ref_t *end = begin + arr->len;
                visitor->visit_pointers(visitor, obj, begin, end);
            } else if (item_ty->constraint == K_STRUCT) {
                address_t base = yalx_multi_dims_array_data(arr);
                for (int i = 0; i < arr->len; i++) {
                    struct_shallow_visit(obj, base, item_ty, visitor);
                    base += item_ty->instance_size;
                }
            } else if (item_ty->constraint == K_ENUM) {
                UNREACHABLE();
            }
        } break;

        default: {
            DCHECK(yalx_is_ref_type(klass) || yalx_is_compact_enum_type(klass));

            address_t base_addr = (address_t)obj;
            for (int i = 0; i < klass->n_fields; i++) {
                struct yalx_class_field *field = &klass->fields[i];

                if (yalx_is_ref_type(field->type) || yalx_is_compact_enum_type(field->type)) {
                    yalx_ref_t *addr = (yalx_ref_t *)(base_addr + field->offset_of_head);
                    visitor->visit_pointer(visitor, obj, addr);
                } else if (field->type->constraint == K_STRUCT) {
                    address_t base = base_addr + field->offset_of_head;
                    struct_shallow_visit(obj, base, field->type, visitor);
                } else if (field->type->constraint == K_ENUM) {
                    UNREACHABLE();
                }
            }
        } break;
    }
}

struct yalx_value_str *yalx_any_to_string(struct yalx_value_any *any) {
    UNREACHABLE();
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

