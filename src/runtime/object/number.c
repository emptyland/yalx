#include "runtime/object/number.h"
#include "runtime/heap/heap.h"


struct yalx_value_number_l *yalx_new_small_boxing_number(struct heap *heap, const struct yalx_class *klass) {
    struct allocate_result result = yalx_heap_allocate(heap, klass, sizeof(struct yalx_value_number_l), 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    struct yalx_value_number_l *l = (struct yalx_value_number_l *)result.object;
    l->box.u32 = 0;
    return l;
}

struct yalx_value_number_w *yalx_new_big_boxing_number(struct heap *heap, const struct yalx_class *klass) {
    struct allocate_result result = yalx_heap_allocate(heap, klass, sizeof(struct yalx_value_number_w), 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    struct yalx_value_number_w *w = (struct yalx_value_number_w *)result.object;
    w->box.u64 = 0;
    return w;
}
