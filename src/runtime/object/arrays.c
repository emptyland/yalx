#include "runtime/object/arrays.h"
#include "runtime/object/type.h"
#include "runtime/heap/heap.h"
#include "runtime/checking.h"
#include <stdio.h>

struct yalx_value_array_header *
yalx_new_refs_array_with_data(struct heap *h,
                              const struct yalx_class *item,
                              u32_t dims,
                              const u32_t *caps,
                              yalx_ref_t *data,
                              size_t nitems) {
    DCHECK(dims > 0);
    struct yalx_value_array_header *rs = NULL;
    address_t incoming = NULL;
    if (dims <= 1) {
        struct yalx_value_array *bundle = yalx_new_array(h, item, nitems);
        incoming = bundle->data;
        rs = (struct yalx_value_array_header *)bundle;
    } else {
        struct yalx_value_multi_dims_array *bundle = yalx_new_multi_dims_array(h, item, dims, caps);
        incoming = yalx_multi_dims_array_data(bundle);
        rs = (struct yalx_value_array_header *)bundle;
    }
    memcpy(incoming, data, (rs->len * sizeof(yalx_ref_t)));
    DCHECK(h->barrier_ops.init_write_barrier_batch != NULL);
    init_write_barrier_batch(h, (yalx_ref_t *)incoming, rs->len);
    return rs;
}

struct yalx_value_array_header *
yalx_new_vals_array_with_data(struct heap *h,
                              const struct yalx_class *item,
                              u32_t dims,
                              const u32_t *caps,
                              const void *data,
                              size_t nitems) {
    DCHECK(dims > 0);
    struct yalx_value_array_header *rs = NULL;
    address_t incoming = NULL;
    if (dims <= 1) {
        struct yalx_value_array *bundle = yalx_new_array(h, item, nitems);
        incoming = bundle->data;
        rs = (struct yalx_value_array_header *)bundle;
    } else {
        struct yalx_value_multi_dims_array *bundle = yalx_new_multi_dims_array(h, item, dims, caps);
        incoming = yalx_multi_dims_array_data(bundle);
        rs = (struct yalx_value_array_header *)bundle;
    }
    memcpy(incoming, data, (rs->len * item->instance_size));
    if ((item->constraint == K_STRUCT || item->constraint == K_ENUM) && item->refs_mark_len > 0) {
        address_t p = incoming;
        for (size_t i = 0; i < rs->len; i++) {
            init_typing_write_barrier_if_needed(h, item, p);
            p += item->instance_size;
        }
    }
    return rs;
}

struct yalx_value_array *yalx_new_array(struct heap *h, const struct yalx_class *item, size_t nitems) {
    DCHECK(item != NULL);

    const size_t item_size = yalx_is_ref_type(item) ? item->reference_size : item->instance_size;
    const size_t size = sizeof(struct yalx_value_array) + (nitems * item_size);
    struct allocate_result result = yalx_heap_allocate(h, array_class, size, 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    struct yalx_value_array *bundle = (struct yalx_value_array *)result.object;
    bundle->len = (u32_t)nitems;
    bundle->item = item;
    return bundle;
}

struct yalx_value_multi_dims_array *
yalx_new_multi_dims_array(struct heap *h, const struct yalx_class *item, u32_t dims, const u32_t *caps) {
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
    struct allocate_result result = yalx_heap_allocate(h, multi_dims_array_class, size, 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    
    struct yalx_value_multi_dims_array *bundle = (struct yalx_value_multi_dims_array *)result.object;
    bundle->len = (u32_t)nitems;
    bundle->item = item;
    bundle->rank = dims;
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

struct yalx_value_array_header *yalx_array_clone(struct heap *heap, struct yalx_value_array_header *origin) {
    if (!origin) {
        return NULL;
    }
    if (CLASS(origin) == array_class) {
        struct yalx_value_array *ar = (struct yalx_value_array *)origin;
        if (yalx_is_ref_type(ar->item)) {
            return yalx_new_refs_array_with_data(heap, ar->item, 1, &ar->len, (yalx_ref_t *)ar->data, ar->len);
        } else {
            return yalx_new_vals_array_with_data(heap, ar->item, 1, &ar->len, (const void *)ar->data, ar->len);
        }
    } else {
        DCHECK(CLASS(origin) == multi_dims_array_class);
        struct yalx_value_multi_dims_array *ar = (struct yalx_value_multi_dims_array *)origin;
        void *data = yalx_multi_dims_array_data(ar);
        if (yalx_is_ref_type(ar->item)) {
            return yalx_new_refs_array_with_data(heap, ar->item, ar->rank, ar->caps, (yalx_ref_t *)data, ar->len);
        } else {
            return yalx_new_vals_array_with_data(heap, ar->item, ar->rank, ar->caps, data, ar->len);
        }
    }
}

void *yalx_array_location2(struct yalx_value_multi_dims_array *ar, int d0, int d1) {
    const size_t item_size_in_bytes = yalx_is_ref_type(ar->item) ? ar->item->reference_size : ar->item->instance_size;
    DCHECK(ar->rank == 2);
    DCHECK(d0 >= 0 && d0 < ar->caps[0]);
    DCHECK(d1 >= 0 && d1 < ar->caps[1]);
    return yalx_multi_dims_array_data(ar) + (d0 * ar->caps[1] + d1) * item_size_in_bytes;
}

/* int[2,2,3]
at [1,1,1] == 11 => at[x * 6 + (y * 3) + z] = at[10]
at [1,1,2] == 12 => at[x * 6 + (y * 3) + z] = at[11]
at [0,1,2] == 6  => at[x * 6 + (y * 3) + z] = at[5]
 [0] 1
 [1] 2
 [2] 3
 [3] 4
 [4] 5
 [5] 6
 ....
{
    {
        {
            1, 2, 3
        }, {
            4, 5, 6
        }
    }, {
        {
            7, 8, 9
        }, {
            10, 11, 12
        }
    }
}
 */
void *yalx_array_location3(struct yalx_value_multi_dims_array *ar, int d0, int d1, int d2) {
    const size_t item_size_in_bytes = yalx_is_ref_type(ar->item) ? ar->item->reference_size : ar->item->instance_size;
    DCHECK(ar->rank == 3);
    DCHECK(d0 >= 0 && d0 < ar->caps[0]);
    DCHECK(d1 >= 0 && d1 < ar->caps[1]);
    DCHECK(d2 >= 0 && d2 < ar->caps[2]);
    const size_t index = (d0 * (ar->caps[1] * ar->caps[2]) + d1 * ar->caps[2] + d2);
    return yalx_multi_dims_array_data(ar) + index * item_size_in_bytes;
}

void *yalx_array_location_more(struct yalx_value_multi_dims_array *ar, const int *indices) {
    const size_t item_size_in_bytes = yalx_is_ref_type(ar->item) ? ar->item->reference_size : ar->item->instance_size;
    
#ifndef NDEBUG
    for (int i = 0; i < ar->rank; i++) {
        DCHECK(indices[i] >= 0 && indices[i] < ar->caps[i]);
    }
#endif

    size_t offset = 0;
    for (int i = 0; i < ar->rank; i++) {
        size_t quantity = indices[i];
        for (int j = i + 1; j < ar->rank; j++) {
            quantity *= ar->caps[j];
        }
        offset += quantity;
    }
    return yalx_multi_dims_array_data(ar) + offset * item_size_in_bytes;
}


void yalx_Zplang_Zolang_ZdmultiDimsArrayGetLength_stub(yalx_ref_handle self, i32_t dim) {
    DCHECK(self != NULL);
    DCHECK(*self != NULL);
    
    struct yalx_value_multi_dims_array *ar = (struct yalx_value_multi_dims_array *)(*self);
    DCHECK(CLASS(ar) == multi_dims_array_class);
    DCHECK(dim >= 0 && dim < ar->rank);
    
    //printf("[%d]=%d\n", dim, ar->caps[dim]);
    yalx_return_i32(ar->caps[dim]);
}
