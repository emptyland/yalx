# Runtime

## Value & Type

```c
struct yalx_value_header {
    uintptr_t kclass;
    union {
        uint32_t tags[2];
        struct yalx_value_fun **vft;
    } u;
};

typedef yalx_value_header yalx_value_t;

struct yalx_class {
    uint64_t id;
    uint32_t tags;
    int32_t reference_size;
    int32_t instance_size;
    struct yalx_class *base;
    struct yalx_raw_str name;
    struct yalx_raw_str location;
    uint32_t n_fields;
    struct yalx_class_field *fields;
    struct yalx_class_method *ctor;
    uint32_t n_methods;
    struct yalx_class_method *methods;
    // uint32_t n_annotations;
    // yalx_annotation *annotation;
};

struct yalx_class_field {
    uint32_t index;
    uint32_t tags;
    struct yalx_raw_str name;
    yalx_class *type;
    uint32_t offset_of_object;
    // uint32_t n_annotations;
    // yalx_annotation *annotation;
};

struct yalx_class_method {
    uint32_t index;
    uint32_t tags;
    struct yalx_raw_str name;
    struct yalx_raw_str prototype_desc;
    // uint32_t n_annotations;
    // yalx_annotation *annotation;
    void *entry;
};

struct yalx_raw_str {
    const char *z;
    uint32_t    n;
}
```

## Annotation

```c

```

## Interface

```c
struct yalx_value_interface {
    yalx_value_t *self;
    uint32_t n_methods;
    struct yalx_value_fun *vtab[];
};
```

## String

struct yalx_value_str {
    struct yalx_value_header header;
    uint32_t hash_code;
    uint32_t len;
    uint8_t bytes[0];
};


## ABI


## Runtime API

```c
yalx_value_t *rt_new_object(const struct yalx_class *clazz, uint32_t tags);

struct yalx_value_str *rt_new_string(const char *z, uint32_t n);

struct yalx_value_str *rt_new_string_from_raw(struct yalx_raw_str *s);
```


