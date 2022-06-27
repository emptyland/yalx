#include "runtime/object/arrays.h"
#include "runtime/object/type.h"
#include "runtime/heap/heap.h"
#include "runtime/checking.h"


struct yalx_value_refs_array *
yalx_new_refs_array_with_data(struct heap *heap, const struct yalx_class *item, yalx_ref_t *data, size_t nitems) {
    struct yalx_value_refs_array *bundle = yalx_new_refs_array(heap, item, nitems);
    bundle->len = (u32_t)nitems;
    bundle->item = item;
    memcpy(bundle->data, data, (nitems * sizeof(yalx_ref_t)));
    init_write_barrier_batch(heap, &bundle->data, nitems);
    return bundle;
}

struct yalx_value_dims_array *
yalx_new_dims_array_with_data(struct heap *heap, const struct yalx_class *item, yalx_ref_t *data, size_t nitems) {
    struct yalx_value_dims_array *bundle = yalx_new_dims_array(heap, item, nitems);
    bundle->len = (u32_t)nitems;
    bundle->dims = 0;
    bundle->item = item;
    memcpy(bundle->arrays, data, (nitems * sizeof(yalx_ref_t)));
    init_write_barrier_batch(heap, &bundle->arrays, nitems);
    return bundle;
}



struct yalx_value_typed_array *
yalx_new_typed_array_with_data(struct heap *heap, const struct yalx_class *item, const void *data, size_t nitems) {
    struct yalx_value_typed_array *bundle = yalx_new_typed_array(heap, item, nitems);
    bundle->len = (u32_t)nitems;
    bundle->item = item;
    memcpy(bundle->data, data, (nitems * item->reference_size));
    if (item->constraint == K_STRUCT) {
        address_t p = (address_t)data;
        for (size_t i = 0; i < nitems; i++) {
            init_typing_write_barrier_if_needed(heap, item, p);
            p += item->reference_size;
        }
    }
    return bundle;
}

struct yalx_value_refs_array *yalx_new_refs_array(struct heap *heap, const struct yalx_class *item, size_t nitems) {
    size_t size = sizeof(struct yalx_value_refs_array) + (nitems * sizeof(yalx_ref_t));
    struct allocate_result result = yalx_heap_allocate(heap, refs_array_class, size, 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    struct yalx_value_refs_array *bundle = (struct yalx_value_refs_array *)result.object;
    bundle->len = (u32_t)nitems;
    bundle->item = item;
    return bundle;
}

struct yalx_value_dims_array *yalx_new_dims_array(struct heap *heap, const struct yalx_class *item, size_t nitems) {
    size_t size = sizeof(struct yalx_value_dims_array) + (nitems * sizeof(yalx_ref_t));
    struct allocate_result result = yalx_heap_allocate(heap, dims_array_class, size, 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    struct yalx_value_dims_array *bundle = (struct yalx_value_refs_array *)result.object;
    bundle->len = (u32_t)nitems;
    bundle->dims = 0;
    bundle->item = item;
    return bundle;
}

struct yalx_value_typed_array *yalx_new_typed_array(struct heap *heap, const struct yalx_class *item, size_t nitems) {
    DCHECK(item != NULL);
    DCHECK(item->constraint == K_STRUCT || item->constraint == K_PRIMITIVE);

    size_t size = sizeof(struct yalx_value_typed_array) + (nitems * item->instance_size);
    struct allocate_result result = yalx_heap_allocate(heap, typed_array_class, size, 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    struct yalx_value_typed_array *bundle = (struct yalx_value_typed_array *)result.object;
    bundle->len = (u32_t)nitems;
    bundle->item = item;
    return bundle;
}

struct yalx_value_array_header *yalx_cast_to_array_if_possibly(yalx_ref_t obj) {
    if (!obj) {
        return NULL;
    }
    const struct yalx_class *ty = CLASS(obj);
    if (ty == typed_array_class || ty == dims_array_class || ty == refs_array_class) {
        return (struct yalx_value_array_header *)obj;
    } else {
        return NULL;
    }
}
