#include "runtime/heap/ygc.h"
#include "runtime/thread.h"
#include "runtime/checking.h"

uintptr_t YGC_MARKED0_BIT         = 0x0000100000000000;
uintptr_t YGC_MARKED1_BIT         = 0x0000200000000000;
uintptr_t YGC_REMAPPED_BIT        = 0x0000400000000000;

uintptr_t YGC_ADDRESS_OFFSET_MASK = 0x00000FFFFFFFFFFF;

size_t YGC_VIRTUAL_ADDRESS_SPACE_LEN = 0x0000100000000000;

int ygc_init(struct ygc_core *ygc, size_t capacity) {
    if (memory_backing_init(&ygc->backing, capacity) < 0) {
        return -1;
    }
    if (physical_memory_management_init(&ygc->pmm, 0, capacity) < 0) {
        return -1;
    }
    if (virtual_memory_management_init(&ygc->vmm, YGC_VIRTUAL_ADDRESS_SPACE_LEN) < 0) {
        return -1;
    }
    ygc->rss = 0;
    ygc->medium_page = NULL;
    ygc->small_page = per_cpu_storage_new();
    ygc->pages.next = &ygc->pages;
    ygc->pages.prev = &ygc->pages;
    yalx_mutex_init(&ygc->mutex);
    return 0;
}

void ygc_final(struct ygc_core *ygc) {
    // TODO:
    yalx_mutex_final(&ygc->mutex);
    while (!QUEUE_EMPTY(&ygc->pages)) {
        struct ygc_page *page = ygc->pages.next;
        QUEUE_REMOVE(page);
        ygc_page_free(ygc,page);
    }

    for (int i = 0; i < ygc->small_page->n; i++) {
        struct ygc_page *page = per_cpu_get(struct ygc_page *, ygc->small_page);
        USE(page);
    }
    per_cpu_storage_free(ygc->small_page);
    virtual_memory_management_final(&ygc->vmm);
    physical_memory_management_final(&ygc->pmm);
    memory_backing_final(&ygc->backing);
    ygc->rss = 0;
}

struct ygc_page *ygc_page_new(struct ygc_core *ygc, size_t size) {
    NOREACHABLE();
    return NULL;
}

void ygc_page_free(struct ygc_core *ygc, struct ygc_page *pg) {
    if (!pg) {
        return;
    }
    NOREACHABLE();
}