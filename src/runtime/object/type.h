#pragma once
#ifndef YALX_RUNTIME_OBJECT_TYPE_H_
#define YALX_RUNTIME_OBJECT_TYPE_H_

#include "runtime/runtime.h"

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
    K_PRIMITIVE,
};

struct yalx_class {
    // unique identifer
    uint64_t id;
    /* class tags */
    uint8_t constraint;
    int32_t reference_size;
    int32_t instance_size;
    struct yalx_class *super;
    struct yalx_str name;
    struct yalx_str location;
    uint32_t n_annotations;
    //yalx_annotation_t *annotation;
    uint32_t n_fields;
    struct yalx_class_field *fields;
    struct yalx_class_method *ctor;
    uint32_t n_methods;
    struct yalx_class_method *methods;
    uint32_t n_vtab;
    uint32_t n_itab;
    struct yalx_class_method **vtab;
    struct yalx_class_method **itab;
}; // struct yalx_class


struct yalx_class_field {
    // tags
    uint8_t access; // yalx_access_desc
    uint32_t n_annotations;
    //yalx_annotation *annotation;
    struct yalx_str name;
    struct yalx_class *type;
    uint32_t offset_of_object;
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
    struct yalx_str name;
    struct yalx_str prototype_desc;
    address_t entry;
}; // struct yalx_class_method


// builtin-types:
extern struct yalx_class yalx_lang_any_class;
extern struct yalx_class bool_class;
extern struct yalx_class i8_class;

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_OBJECT_TYPE_H_
