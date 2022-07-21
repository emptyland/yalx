#include "runtime/object/type.h"
#include "runtime/object/any.h"

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
