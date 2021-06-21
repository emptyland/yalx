#include "runtime/object/type.h"
#include "runtime/object/any.h"

static struct yalx_class_method any_class_methods[] = {
    {
        .index = 0,
        /* tags: */
        .access = ACC_PUBLIC,
        .is_native = 1,
        .is_override = 0,
        .should_virtual = 1,
        .is_builtin = 1,
        .is_ctor = 0,
        .vtab_index = 0,
        .n_annotations = 0,
        //yalx_annotation *annotation;
        .name = YALX_STR("id"),
        .prototype_desc = YALX_STR("():int"),
        .entry = NULL, // TODO:
    }, {
        .index = 1,
        /* tags: */
        .access = ACC_PUBLIC,
        .is_native = 1,
        .is_override = 0,
        .should_virtual = 1,
        .is_builtin = 1,
        .is_ctor = 0,
        .vtab_index = 1,
        .n_annotations = 0,
        //yalx_annotation *annotation;
        .name = YALX_STR("hashCode"),
        .prototype_desc = YALX_STR("():u32"),
        .entry = NULL, // TODO:
    }, {
        .index = 2,
        /* tags: */
        .access = ACC_PUBLIC,
        .is_native = 1,
        .is_override = 0,
        .should_virtual = 1,
        .is_builtin = 1,
        .is_ctor = 0,
        .vtab_index = 1,
        .n_annotations = 0,
        //yalx_annotation *annotation;
        .name = YALX_STR("toString"),
        .prototype_desc = YALX_STR("():string"),
        .entry = NULL, // TODO:
    }, {
        .index = 3,
        /* tags: */
        .access = ACC_PUBLIC,
        .is_native = 1,
        .is_override = 0,
        .should_virtual = 1,
        .is_builtin = 1,
        .is_ctor = 0,
        .vtab_index = 1,
        .n_annotations = 0,
        //yalx_annotation *annotation;
        .name = YALX_STR("isEmpty"),
        .prototype_desc = YALX_STR("():bool"),
        .entry = NULL, // TODO:
    }, {
        .index = 4,
        /* tags: */
        .access = ACC_PUBLIC,
        .is_native = 1,
        .is_override = 0,
        .should_virtual = 1,
        .is_builtin = 1,
        .is_ctor = 0,
        .vtab_index = 1,
        .n_annotations = 0,
        //yalx_annotation *annotation;
        .name = YALX_STR("finalize"),
        .prototype_desc = YALX_STR("():unit"),
        .entry = NULL, // TODO:
    }
};

static struct yalx_class_method *any_class_vitabs[] = {
    &any_class_methods[0],
    &any_class_methods[1],
    &any_class_methods[2],
    &any_class_methods[3],
    &any_class_methods[4]
};


struct yalx_class builtin_classes[MAX_BUILTIN_TYPES] = {
    [Type_any] = {
        .id = 0,
        .constraint = K_CLASS,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_any),
        .super = NULL,
        .name = YALX_STR("any"),
        .location = YALX_STR("yalx.lang.any"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 5,
        .methods = any_class_methods,
        .n_itab = 5,
        .n_vtab = 5,
        .itab = any_class_vitabs,
        .vtab = any_class_vitabs,
        // TODO:
    }, // any
    
    [Type_bool] = {
        .id = 1,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(char),
        .instance_size = sizeof(char),
        .super = &yalx_lang_any_class,
        .name = YALX_STR("bool"),
        .location = YALX_STR("bool"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 4,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // bool
    
    [Type_i8] = {
        .id = 2,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(i8_t),
        .instance_size = sizeof(i8_t),
        .super = &yalx_lang_any_class,
        .name = YALX_STR("i8"),
        .location = YALX_STR("i8"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 4,
        .methods = NULL,  // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // i8
    
    [Type_u8] = {
        .id = 2,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(u8_t),
        .instance_size = sizeof(u8_t),
        .super = &yalx_lang_any_class,
        .name = YALX_STR("u8"),
        .location = YALX_STR("u8"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 4,
        .methods = NULL,  // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // u8
    
    //------------------------------------------------------------------------------------------------------------------
    // Boxing types:
    //------------------------------------------------------------------------------------------------------------------

    [Type_string] = {
        .id = 16,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = 0,
        .super = &yalx_lang_any_class,
        .name = YALX_STR("string"),
        .location = YALX_STR("string"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 4,
        .methods = NULL,  // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // string
};


const struct yalx_class *const yalx_lang_any_class = &builtin_classes[Type_any];
const struct yalx_class *const bool_class = &builtin_classes[Type_bool];
const struct yalx_class *const i8_class = &builtin_classes[Type_i8];
const struct yalx_class *const u8_class = &builtin_classes[Type_u8];

const struct yalx_class *const string_class = &builtin_classes[Type_string];
