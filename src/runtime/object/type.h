#pragma once
#ifndef YALX_RUNTIME_OBJECT_TYPE_H_
#define YALX_RUNTIME_OBJECT_TYPE_H_

#include "runtime/runtime.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum yalx_access_desc {
    ACC_PUBLIC,
    ACC_PRIVATE,
    ACC_PROTECTED
}; // enum yalx_access_desc

enum yalx_class_constraint {
    K_CLASS,
    K_STRUCT,
    K_ENUM,
    K_PRIMITIVE,
};

struct yalx_class {
    // unique identifer
    uint64_t id;
    /* class tags */
    uint8_t constraint; // enum yalx_class_constraint
    uint8_t compact_enum; // is compact enum ty?
    int32_t reference_size;
    int32_t instance_size;
    struct yalx_class *super;
    struct yalx_str name;
    struct yalx_str location;
    uint32_t n_annotations;
    //yalx_annotation_t *annotation;
    void *reserved0;
    uint32_t n_fields;
    struct yalx_class_field *fields;
    struct yalx_class_method *ctor;
    uint32_t n_methods;
    struct yalx_class_method *methods;
    uint32_t n_vtab;
    uint32_t n_itab;
    address_t *vtab;
    address_t *itab;
    uint32_t refs_mark_len;
    struct {
        const struct yalx_class *ty;
        ptrdiff_t offset;
    } refs_mark[0];
}; // struct yalx_class


#define CLASS_OFFSET_OF_VTAB offsetof(struct yalx_class, vtab)
#define CLASS_OFFSET_OF_ITAB offsetof(struct yalx_class, itab)

struct yalx_class_field {
    // tags
    uint32_t access: 2; // yalx_access_desc
    uint32_t constraint: 2; // val? var?
    uint32_t enum_code: 16; // id of enum
    uint32_t n_annotations;
    //yalx_annotation *annotation;
    void *reserved0;
    struct yalx_str name;
    struct yalx_class *type;
    uint32_t offset_of_head;
}; // struct yalx_class_field


struct yalx_class_method {
    uint32_t index;
    /* tags: */
    uint32_t access: 2; // yalx_access_desc
    uint32_t is_native: 1; // is native function?
    uint32_t is_override: 1; // is override function?
    uint32_t should_virtual: 1; // should in virtual table?
    uint32_t is_builtin: 1; // is builtin-function?
    uint32_t is_ctor: 1; // is constructor?
    uint32_t reserved: 1;
    uint32_t vtab_index: 24;
    uint32_t n_annotations;
    //yalx_annotation *annotation;
    void *reserved0;
    struct yalx_str name;
    struct yalx_str prototype_desc;
    address_t entry;
}; // struct yalx_class_method

enum yalx_builtin_type {
    Type_any,
    Type_bool,
    Type_i8,
    Type_u8,
    Type_i16,
    Type_u16,
    Type_i32,
    Type_u32,
    Type_i64,
    Type_u64,
    Type_f32,
    Type_f64,
    Type_Bool,
    Type_I8,
    Type_U8,
    Type_I16,
    Type_U16,
    Type_I32,
    Type_U32,
    Type_I64,
    Type_U64,
    Type_F32,
    Type_F64,
    Type_string,
    Type_array,
    Type_unused0,
    Type_multi_dims_array,
    MAX_BUILTIN_TYPES,
    NOT_BUILTIN_TYPE,
};

extern struct yalx_class *builtin_classes;

static inline enum yalx_builtin_type yalx_builtin_type(const struct yalx_class * klass) {
    ptrdiff_t diff = klass - builtin_classes;
    if (diff < 0 || diff >= MAX_BUILTIN_TYPES) {
        return NOT_BUILTIN_TYPE;
    }
    return (enum yalx_builtin_type)diff;
}

int yalx_is_ref_type(const struct yalx_class * klass);
int yalx_is_compact_enum_type_fallback(const struct yalx_class * klass);

#define yalx_is_compact_enum_type(klass) ((klass)->compact_enum)

//int yalx_has_at_least_one_ref_type(const struct yalx_class * klass);

// builtin-types:

extern const struct yalx_class *const bool_class;
extern const struct yalx_class *const i8_class;
extern const struct yalx_class *const u8_class;

extern const struct yalx_class *const Bool_class;
extern const struct yalx_class *const I8_class;
extern const struct yalx_class *const U8_class;
extern const struct yalx_class *const I16_class;
extern const struct yalx_class *const U16_class;
extern const struct yalx_class *const I32_class;
extern const struct yalx_class *const U32_class;
extern const struct yalx_class *const I64_class;
extern const struct yalx_class *const U64_class;
extern const struct yalx_class *const F32_class;
extern const struct yalx_class *const F64_class;

extern const struct yalx_class *const any_class;
extern const struct yalx_class *const string_class;


// Classes of arrays
extern const struct yalx_class *const array_class;
extern const struct yalx_class *const multi_dims_array_class;

// Classes of throwing
extern const struct yalx_class *backtrace_frame_class;
extern const struct yalx_class *throwable_class;
extern const struct yalx_class *exception_class;
extern const struct yalx_class *bad_casting_exception_class;
extern const struct yalx_class *array_index_out_of_bounds_exception_class;

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_OBJECT_TYPE_H_
