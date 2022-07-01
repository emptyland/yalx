#include "runtime/object/type.h"


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
