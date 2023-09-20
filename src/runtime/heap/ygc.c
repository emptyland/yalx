#include "runtime/heap/ygc.h"
#include "runtime/thread.h"
#include "runtime/checking.h"

uintptr_t YGC_MARKED0_BIT         = 0x0000100000000000;
uintptr_t YGC_MARKED1_BIT         = 0x0000200000000000;
uintptr_t YGC_REMAPPED_BIT        = 0x0000400000000000;

uintptr_t YGC_ADDRESS_OFFSET_MASK = 0x00000FFFFFFFFFFF;

int ygc_init(struct ygc_core *ygc, size_t capacity) {
    if (memory_backing_init(&ygc->backing, capacity) < 0) {
        return -1;
    }
    ygc->rss = 0;
    ygc->medium_page = NULL;
    ygc->small_page = per_cpu_storage_new();
    return 0;
}

void ygc_final(struct ygc_core *ygc) {
    // TODO:
    per_cpu_storage_free(ygc->small_page);
    memory_backing_final(&ygc->backing);
    ygc->rss = 0;
}

struct ygc_page *ygc_page_new(struct ygc_core *ygc, size_t size) {
    NOREACHABLE();
    return NULL;
}

void ygc_page_free(struct ygc_page *pg) {
    NOREACHABLE();
}