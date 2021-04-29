# Runtime

## Value & Type

```c
struct yalx_value_header {
    uintptr_t kclass;
    uint32_t tags;
};

typedef yalx_value_header yalx_value_t;

struct yalx_class {
    /** unique identifer */
    uint64_t id;
    /** class tags */
    uint32_t tags;
    int32_t reference_size;
    int32_t instance_size;
    struct yalx_class *super;
    struct yalx_raw_str name;
    struct yalx_raw_str location;
    uint32_t n_annotations;
    yalx_annotation_t *annotation;
    uint32_t n_fields;
    struct yalx_class_field *fields;
    struct yalx_class_method *ctor;
    uint32_t n_methods;
    struct yalx_class_method *methods;
    uint32_t n_vtab;
    uint32_t n_itab;
    yalx_vitab_entry_t *vtab;
    yalx_vitab_entry_t *itab;
};

enum yalx_access_desc {
    ACC_PUBLIC,
    ACC_PRIVATE,
    ACC_PROTECTED
}

struct yalx_class_field {
    /** tags */
    uint32_t access: 2; // yalx_access_desc
    uint32_t is_native: 1; // is native function?
    uint32_t is_override: 1; // is override function?
    uint32_t should_virtual: 1; // should in virtual table?
    uint32_t is_builtin: 1; // is builtin-function?
    uint32_t is_ctor: 1; // is constructor?
    uint32_t reserved: 1;
    uint32_t vtab_index: 24;
    uint32_t n_annotations;
    yalx_annotation *annotation;
    struct yalx_raw_str name;
    yalx_class *type;
    uint32_t offset_of_object;
};

struct yalx_class_method {
    uint32_t index;
    uint32_t tags;
    uint32_t n_annotations;
    yalx_annotation *annotation;
    struct yalx_raw_str name;
    struct yalx_raw_str prototype_desc;
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


## Safepoint Mechanism

Safepoint for

1. GC stop world.
2. Debug

Implements: Like jvm, signal `SIGSEG` break the thread.


## Garbage Collection

### Allocator

```c
struct yalx_heap {
    struct yalx_new_space   new_space;
    struct yalx_old_space   old_space;
    struct yalx_large_space large_space;
};

struct yalx_allocator {
    struct allocation_result (*allocate)(size_t, uint32_t);
};

```

### New Space

```
<After Minor GC>

|<------used------>|
still_survival     |
|      |           |
+------------------------------------+------------------------------------+
| ssss |           | xxxxxxxxxxxxxxx |                                    |
+------------------------------------+------------------------------------+
^                  ^---- free        ^
| origianl_space                     | survivor_space
```

New Space has 2 semi-spaces: `origianl_space` and `survivor_space`

```
<In GC>

|<------used------>|
still_survival     |                 | copy reached objects
|      |           |                 v           v
+------------------------------------+------------------------------------+
| ssss |           | xxxxxxxxxxxxxxx | ///////// |                        |
+------------------------------------+------------------------------------+
^                  ^---- free        ^
| origianl_space                     | survivor_space


<Promote still_survival objects to Old Spaces>

+------------------------------------+------------------------------------+
| pppp |           | xxxxxxxxxxxxxxx | ///////// |                        |
+------------------------------------+------------------------------------+
^                                    ^           ^----- new free
| origianl_space                     | survivor_space


<Fliping: Exchange origianl_space and survivor_space pointer>

                                                 v----- still_survival
+------------------------------------+------------------------------------+
|                Clean               | ///////// | xxxxxxxxxxxxxxxxxxxxxx |
+------------------------------------+------------------------------------+
^                                    ^           ^----- free
| survivor_space                     | origianl_space

```

In minor-GC: 
1. Copy all reachable objects from `origianl_space` to `survivor_space`.
2. Promote `still_survival` objects to old spaces.
3. Exchange `origianl_space` and `survivor_space` pointers.

### Old Space

### Large Space

