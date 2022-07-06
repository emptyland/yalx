#include "runtime/object/arrays.h"
#include "runtime/object/type.h"
#include "runtime/heap/heap.h"
#include "runtime/checking.h"


struct yalx_value_array_header *
yalx_new_refs_array_with_data(struct heap *heap,
                              const struct yalx_class *item,
                              u32_t dims,
                              const u32_t *caps,
                              yalx_ref_t *data,
                              size_t nitems) {
    DCHECK(dims > 0);
    struct yalx_value_array_header *rs = NULL;
    address_t incoming = NULL;
    if (dims <= 1) {
        struct yalx_value_array *bundle = yalx_new_array(heap, item, nitems);
        incoming = bundle->data;
        rs = (struct yalx_value_array_header *)bundle;
    } else {
        struct yalx_value_multi_dims_array *bundle = yalx_new_multi_dims_array(heap, item, dims, caps);
        incoming = yalx_multi_dims_array_data(bundle);
        rs = (struct yalx_value_array_header *)bundle;
    }
    memcpy(incoming, data, (rs->len * sizeof(yalx_ref_t)));
    init_write_barrier_batch(heap, incoming, rs->len);
    return rs;
}

struct yalx_value_array_header *
yalx_new_vals_array_with_data(struct heap *heap,
                              const struct yalx_class *item,
                              u32_t dims,
                              const u32_t *caps,
                              const void *data,
                              size_t nitems) {
    DCHECK(dims > 0);
    struct yalx_value_array_header *rs = NULL;
    address_t incoming = NULL;
    if (dims <= 1) {
        struct yalx_value_array *bundle = yalx_new_array(heap, item, nitems);
        incoming = bundle->data;
        rs = (struct yalx_value_array_header *)bundle;
    } else {
        struct yalx_value_multi_dims_array *bundle = yalx_new_multi_dims_array(heap, item, dims, caps);
        incoming = yalx_multi_dims_array_data(bundle);
        rs = (struct yalx_value_array_header *)bundle;
    }
    memcpy(incoming, data, (rs->len * item->reference_size));
    if (item->constraint == K_STRUCT) {
        address_t p = incoming;
        for (size_t i = 0; i < rs->len; i++) {
            init_typing_write_barrier_if_needed(heap, item, p);
            p += item->reference_size;
        }
    }
    return rs;
}

struct yalx_value_array *yalx_new_array(struct heap *heap, const struct yalx_class *item, size_t nitems) {
    DCHECK(item != NULL);

    const size_t item_size = yalx_is_ref_type(item) ? item->reference_size : item->instance_size;
    const size_t size = sizeof(struct yalx_value_array) + (nitems * item_size);
    struct allocate_result result = yalx_heap_allocate(heap, array_class, size, 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    struct yalx_value_array *bundle = (struct yalx_value_array *)result.object;
    bundle->len = (u32_t)nitems;
    bundle->item = item;
    return bundle;
}

struct yalx_value_multi_dims_array *
yalx_new_multi_dims_array(struct heap *heap, const struct yalx_class *item, u32_t dims, const u32_t *caps) {
    DCHECK(item != NULL);
    DCHECK(dims > 1);
    DCHECK(caps != NULL);
    size_t nitems = caps[0];
    for (u32_t i = 1; i < dims; i++) {
        nitems *= caps[i];
    }
    
    const size_t item_size = yalx_is_ref_type(item) ? item->reference_size : item->instance_size;
    const size_t size = sizeof(struct yalx_value_multi_dims_array) // header
        + (dims - 1) * sizeof(u32_t) // caps
        + (nitems * item_size);      // data of items
    struct allocate_result result = yalx_heap_allocate(heap, multi_dims_array_class, size, 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    
    struct yalx_value_multi_dims_array *bundle = (struct yalx_value_multi_dims_array *)result.object;
    bundle->len = (u32_t)nitems;
    bundle->item = item;
    bundle->dims = dims;
    for (u32_t i = 0; i < dims; i++) {
        bundle->caps[i] = caps[i];
    }
    return bundle;
}

struct yalx_value_array_header *yalx_cast_to_array_if_possibly(yalx_ref_t obj) {
    if (!obj) {
        return NULL;
    }
    const struct yalx_class *ty = CLASS(obj);
    if (ty == array_class || ty == multi_dims_array_class) {
        return (struct yalx_value_array_header *)obj;
    } else {
        return NULL;
    }
}
