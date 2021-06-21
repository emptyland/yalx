#include "runtime/object/type.h"
#include "runtime/object/any.h"
#include "runtime/object/number.h"
#include "runtime/object/yalx-string.h"


// implemnts in any-[os]-[arch].s
extern void y3zlang_any_id(void);
extern void y3zlang_any_hashCode(void);
extern void y3zlang_any_toString(void);
extern void y3zlang_any_isEmpty(void);
extern void y3zlang_any_finalize(void);

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
        .entry = (address_t)y3zlang_any_id,
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
        .entry = (address_t)y3zlang_any_hashCode,
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
        .entry = (address_t)y3zlang_any_toString,
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
        .entry = (address_t)y3zlang_any_isEmpty,
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
        .entry = (address_t)y3zlang_any_finalize,
    }
};

static struct yalx_class_method *any_class_vitabs[] = {
    &any_class_methods[0],
    &any_class_methods[1],
    &any_class_methods[2],
    &any_class_methods[3],
    &any_class_methods[4]
};

//----------------------------------------------------------------------------------------------------------------------
// string methods
//----------------------------------------------------------------------------------------------------------------------

// implemnts in yalx-string-[os]-[arch].s
extern void y3zlang_string_hashCode(void);
extern void y3zlang_string_toString(void);
extern void y3zlang_string_isEmpty(void);

static struct yalx_class_method string_class_methods[] = {
    {
        .index = 0,
        /* tags: */
        .access = ACC_PUBLIC,
        .is_native = 1,
        .is_override = 1,
        .should_virtual = 0,
        .is_builtin = 1,
        .is_ctor = 0,
        .vtab_index = 1,
        .n_annotations = 0,
        //yalx_annotation *annotation;
        .name = YALX_STR("hashCode"),
        .prototype_desc = YALX_STR("():u32"),
        .entry = (address_t)y3zlang_string_hashCode,
    }, {
        .index = 1,
        /* tags: */
        .access = ACC_PUBLIC,
        .is_native = 1,
        .is_override = 1,
        .should_virtual = 0,
        .is_builtin = 1,
        .is_ctor = 0,
        .vtab_index = 1,
        .n_annotations = 0,
        //yalx_annotation *annotation;
        .name = YALX_STR("toString"),
        .prototype_desc = YALX_STR("():string"),
        .entry = (address_t)y3zlang_string_toString,
    }, {
        .index = 2,
        /* tags: */
        .access = ACC_PUBLIC,
        .is_native = 1,
        .is_override = 1,
        .should_virtual = 0,
        .is_builtin = 1,
        .is_ctor = 0,
        .vtab_index = 2,
        .n_annotations = 0,
        //yalx_annotation *annotation;
        .name = YALX_STR("isEmpty"),
        .prototype_desc = YALX_STR("():bool"),
        .entry = (address_t)y3zlang_string_isEmpty,
    }
    // TODO:
};

static struct yalx_class_method *string_class_vitabs[] = {
    &any_class_methods[0],
    &string_class_methods[0],
    &string_class_methods[1],
    &string_class_methods[2],
    &any_class_methods[4]
};

static struct yalx_class_field string_fields[] = {
    {
        .access = ACC_PUBLIC,
        .constraint = 0,
        // TODO:
        .n_annotations = 0,
        //yalx_annotation *annotation;
        .name = YALX_STR("length"),
        .type = &builtin_classes[Type_u32],
        .offset_of_head = offsetof(struct yalx_value_str, len),
    },
};


//----------------------------------------------------------------------------------------------------------------------
// All buitlin types:
//----------------------------------------------------------------------------------------------------------------------

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
        .super = &builtin_classes[Type_any],
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
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("i8"),
        .location = YALX_STR("i8"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
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
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("u8"),
        .location = YALX_STR("u8"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL,  // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // u8
    
    //------------------------------------------------------------------------------------------------------------------
    // Boxing types:
    //------------------------------------------------------------------------------------------------------------------
    [Type_Bool] = {
        .id = (uint64_t)Type_Bool,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_number_l),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("Bool"),
        .location = YALX_STR("Bool"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // Bool
    
    [Type_I8] = {
        .id = (uint64_t)Type_I8,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_number_l),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("I8"),
        .location = YALX_STR("I8"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // I8
    
    [Type_U8] = {
        .id = (uint64_t)Type_U8,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_number_l),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("U8"),
        .location = YALX_STR("U8"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // U8
    
    [Type_I16] = {
        .id = (uint64_t)Type_I16,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_number_l),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("I16"),
        .location = YALX_STR("I16"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // U8
    
    [Type_U16] = {
        .id = (uint64_t)Type_U16,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_number_l),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("U16"),
        .location = YALX_STR("U16"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // U16
    
    [Type_I32] = {
        .id = (uint64_t)Type_I32,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_number_l),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("I32"),
        .location = YALX_STR("I32"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // I32
    
    [Type_U32] = {
        .id = (uint64_t)Type_U32,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_number_l),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("U32"),
        .location = YALX_STR("U32"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // U32
    
    [Type_I64] = {
        .id = (uint64_t)Type_I64,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_number_w),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("I64"),
        .location = YALX_STR("I64"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // I64
    
    [Type_U64] = {
        .id = (uint64_t)Type_U64,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_number_w),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("U64"),
        .location = YALX_STR("U64"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // U64
    
    [Type_F32] = {
        .id = (uint64_t)Type_F32,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_number_l),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("F32"),
        .location = YALX_STR("F32"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // F32
    
    [Type_F64] = {
        .id = (uint64_t)Type_F64,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_number_w),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("F64"),
        .location = YALX_STR("F64"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // F64
    
    //------------------------------------------------------------------------------------------------------------------
    // string types:
    //------------------------------------------------------------------------------------------------------------------

    [Type_string] = {
        .id = (uint64_t)Type_string,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = 0,
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("string"),
        .location = YALX_STR("string"),
        .n_annotations = 0,
        .n_fields = arraysize(string_fields),
        .fields = string_fields,
        .ctor = NULL,
        .n_methods = arraysize(string_class_methods),
        .methods = string_class_methods,  // TODO:
        .n_itab = arraysize(string_class_vitabs),
        .n_vtab = arraysize(string_class_vitabs),
        .itab = string_class_vitabs,
        .vtab = string_class_vitabs,
        // TODO:
    }, // string
};


const struct yalx_class *const yalx_lang_any_class = &builtin_classes[Type_any];
const struct yalx_class *const bool_class = &builtin_classes[Type_bool];
const struct yalx_class *const i8_class = &builtin_classes[Type_i8];
const struct yalx_class *const u8_class = &builtin_classes[Type_u8];

const struct yalx_class *const Bool_class = &builtin_classes[Type_Bool];
const struct yalx_class *const I8_class = &builtin_classes[Type_I8];
const struct yalx_class *const U8_class = &builtin_classes[Type_U8];
const struct yalx_class *const I16_class = &builtin_classes[Type_I16];
const struct yalx_class *const U16_class = &builtin_classes[Type_U16];
const struct yalx_class *const I32_class = &builtin_classes[Type_I32];
const struct yalx_class *const U32_class = &builtin_classes[Type_U32];
const struct yalx_class *const I64_class = &builtin_classes[Type_I64];
const struct yalx_class *const U64_class = &builtin_classes[Type_U64];
const struct yalx_class *const F32_class = &builtin_classes[Type_F32];
const struct yalx_class *const F64_class = &builtin_classes[Type_F64];

const struct yalx_class *const string_class = &builtin_classes[Type_string];
