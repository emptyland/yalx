#include "runtime/object/arrays.h"
#include "runtime/object/type.h"
#include "runtime/heap/heap.h"
#include "runtime/checking.h"


struct yalx_value_refs_array *yalx_new_refs_array(struct heap *heap, const struct yalx_class *item, yalx_ref_t *data,
                                                  size_t nitems) {
    size_t size = sizeof(struct yalx_value_refs_array) + (nitems * sizeof(yalx_ref_t));
    struct allocate_result result = yalx_heap_allocate(heap, refs_array_class, size, 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    struct yalx_value_refs_array *bundle = (struct yalx_value_refs_array *)result.object;
    bundle->len = (u32_t)nitems;
    bundle->item = item;
    memcpy(bundle->data, data, (nitems * sizeof(yalx_ref_t)));
    post_write_barrier_batch(&heap, bundle, data, nitems);
    return bundle;
}