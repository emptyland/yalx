#include "runtime/heap/heap.h"


struct heap heap;

int yalx_init_heap(struct heap *heap) {
    // TODO:
    return 0;
}

void yalx_free_heap(struct heap *heap) {
    // TODO:
}


struct allocate_result yalx_heap_allocate(size_t size, u32_t flags) {
    struct allocate_result rv;
    rv.object = NULL;
    rv.status = ALLOCATE_NOTHING;
    return rv;
}
