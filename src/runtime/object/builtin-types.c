#include "runtime/object/type.h"
#include "runtime/object/any.h"
#include "runtime/object/number.h"
#include "runtime/object/arrays.h"
#include "runtime/object/yalx-string.h"



//----------------------------------------------------------------------------------------------------------------------
// All buitlin types:
//----------------------------------------------------------------------------------------------------------------------

struct yalx_class builtin_classes[MAX_BUILTIN_TYPES] = {    
    [Type_bool] = {
        .id = (uint64_t)Type_bool,
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
        .id = (uint64_t)Type_i8,
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
        .id = (uint64_t)Type_u8,
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
    
    [Type_i16] = {
        .id = (uint64_t)Type_i16,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(i16_t),
        .instance_size = sizeof(i16_t),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("i16"),
        .location = YALX_STR("i16"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL,  // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // i16
    
    [Type_u16] = {
        .id = (uint64_t)Type_u16,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(u16_t),
        .instance_size = sizeof(u16_t),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("u16"),
        .location = YALX_STR("u16"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL,  // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // u16
    
    [Type_i32] = {
        .id = (uint64_t)Type_i32,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(i32_t),
        .instance_size = sizeof(i32_t),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("i32"),
        .location = YALX_STR("i32"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL,  // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // i32
    
    [Type_u32] = {
        .id = (uint64_t)Type_u32,
        .constraint = K_PRIMITIVE,
        .reference_size = sizeof(u32_t),
        .instance_size = sizeof(u32_t),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("u32"),
        .location = YALX_STR("u32"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL,  // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // u32
    
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
    
    [Type_typed_array] = {
        .id = (uint64_t)Type_typed_array,
        .constraint = K_CLASS,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_typed_array),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("TypedArray"),
        .location = YALX_STR("TypedArray"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // TypedArray
    
    [Type_refs_array] = {
        .id = (uint64_t)Type_refs_array,
        .constraint = K_CLASS,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_refs_array),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("RefsArray"),
        .location = YALX_STR("RefsArray"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // RefsArray
    
    [Type_dims_array] = {
        .id = (uint64_t)Type_dims_array,
        .constraint = K_CLASS,
        .reference_size = sizeof(yalx_ref_t),
        .instance_size = sizeof(struct yalx_value_dims_array),
        .super = &builtin_classes[Type_any],
        .name = YALX_STR("DimsArray"),
        .location = YALX_STR("DimsArray"),
        .n_annotations = 0,
        .n_fields = 0,
        .fields = NULL,
        .ctor = NULL,
        .n_methods = 0,
        .methods = NULL, // TODO:
        .n_itab = 0,
        .n_vtab = 0,
        // TODO:
    }, // RefsArray
};



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

const struct yalx_class *const typed_array_class = &builtin_classes[Type_typed_array];
const struct yalx_class *const refs_array_class = &builtin_classes[Type_refs_array];
const struct yalx_class *const dims_array_class = &builtin_classes[Type_dims_array];

const struct yalx_class *const any_class = &builtin_classes[Type_any];
const struct yalx_class *const string_class = &builtin_classes[Type_string];

const struct yalx_class *backtrace_frame_class = NULL;
const struct yalx_class *throwable_class = NULL;
const struct yalx_class *exception_class = NULL;
const struct yalx_class *bad_casting_exception_class = NULL;
const struct yalx_class *array_index_out_of_bounds_exception_class = NULL;
