#include "runtime/heap/ygc.h"
#include "runtime/thread.h"
#include "runtime/checking.h"

uintptr_t YGC_METADATA_MARKED0  = 1L << YGC_METADATA_SHIFT; // 0x0000100000000000;
uintptr_t YGC_METADATA_MARKED1  = 1L << (YGC_METADATA_SHIFT + 1); // 0x0000200000000000;
uintptr_t YGC_METADATA_REMAPPED = 1L << (YGC_METADATA_SHIFT + 2); // 0x0000400000000000;
uintptr_t YGC_METADATA_MARKED   = 1L << YGC_METADATA_SHIFT; // 0x0000100000000000; // initialize to mark0

uintptr_t YGC_METADATA_MASK = (1L << YGC_METADATA_SHIFT) |
                              (1L << (YGC_METADATA_SHIFT + 1)) |
                              (1L << (YGC_METADATA_SHIFT + 2));

uintptr_t YGC_ADDRESS_OFFSET_MASK = (1L << YGC_METADATA_SHIFT) - 1;

size_t YGC_VIRTUAL_ADDRESS_SPACE_LEN = (1L << YGC_METADATA_SHIFT);


uintptr_t YGC_ADDRESS_GOOD_MASK;
uintptr_t YGC_ADDRESS_BAD_MASK;
uintptr_t YGC_ADDRESS_WEAK_BAD_MASK;

void ygc_set_good_mask(uintptr_t mask) {
    YGC_ADDRESS_GOOD_MASK = mask;
    YGC_ADDRESS_BAD_MASK = YGC_ADDRESS_GOOD_MASK ^ YGC_METADATA_MASK;
    YGC_ADDRESS_WEAK_BAD_MASK = (YGC_ADDRESS_GOOD_MASK | YGC_METADATA_REMAPPED) ^ YGC_METADATA_MASK;
}

void ygc_flip_to_marked() {
    YGC_METADATA_MARKED ^= (YGC_METADATA_MARKED0 | YGC_METADATA_MARKED1);
    ygc_set_good_mask(YGC_METADATA_MARKED);
}

void ygc_flip_to_remapped() {
    ygc_set_good_mask(YGC_METADATA_REMAPPED);
}

int ygc_init(struct ygc_core *ygc, size_t capacity) {
    if (memory_backing_init(&ygc->backing, capacity) < 0) {
        return -1;
    }
    if (physical_memory_management_init(&ygc->pmm, 0, capacity) < 0) {
        memory_backing_final(&ygc->backing);
        return -1;
    }
    if (virtual_memory_management_init(&ygc->vmm, YGC_VIRTUAL_ADDRESS_SPACE_LEN) < 0) {
        memory_backing_final(&ygc->backing);
        physical_memory_management_final(&ygc->pmm);
        return -1;
    }
    ygc->rss = 0;
    ygc->medium_page = NULL;
    ygc->small_page = per_cpu_storage_new();
    ygc->pages.next = &ygc->pages;
    ygc->pages.prev = &ygc->pages;
    yalx_mutex_init(&ygc->mutex);

    ygc_flip_to_remapped(); // Initialize state
    return 0;
}

void ygc_final(struct ygc_core *ygc) {
    // TODO:
    yalx_mutex_final(&ygc->mutex);
    while (!QUEUE_EMPTY(&ygc->pages)) {
        struct ygc_page *page = ygc->pages.next;
        QUEUE_REMOVE(page);
        ygc_page_free(ygc, page);
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

static uintptr_t allocate_small_object(struct ygc_core *ygc, size_t size, uintptr_t alignment_in_bytes) {
    struct ygc_page *page = per_cpu_get(struct ygc_page*, ygc->small_page);
    if (page) {
        uintptr_t chunk = ygc_page_atomic_allocate(page, size, alignment_in_bytes);
        if (chunk != UINTPTR_MAX) {
            return chunk;
        }
    }

    page = ygc_page_new(ygc, SMALL_PAGE_SIZE);
    if (!page) {
        return UINTPTR_MAX;
    }
    per_cpu_set(ygc->small_page, page);

    return ygc_page_allocate(page, size, alignment_in_bytes);
}


static uintptr_t allocate_medium_object(struct ygc_core *ygc, size_t size, uintptr_t alignment_in_bytes) {
    static struct ygc_page *const page_busy = (struct ygc_page *)0x1;
    for (;;) {
        struct ygc_page *page;
        do {
            page = atomic_load(&ygc->medium_page);
        } while (page != page_busy);
        if (page) {
            uintptr_t chunk = ygc_page_atomic_allocate(page, size, alignment_in_bytes);
            if (chunk != UINTPTR_MAX) {
                return chunk;
            }
        }

        if (atomic_compare_exchange_strong(&ygc->medium_page, &page, page_busy)) {
            page = ygc_page_new(ygc, MEDIUM_PAGE_SIZE);
            if (!page) {
                atomic_store(&ygc->medium_page, NULL);
                return UINTPTR_MAX;
            }
            atomic_store(&ygc->medium_page, page);
            return ygc_page_allocate(page, size, alignment_in_bytes);
        }
    }
}

address_t ygc_allocate_object(struct ygc_core *ygc, size_t size, uintptr_t alignment_in_bytes) {
    uintptr_t chunk = UINTPTR_MAX;
    if (size < SMALL_OBJECT_SIZE_LIMIT) {
        chunk = allocate_small_object(ygc, size, alignment_in_bytes);
    } else if (size < MEDIUM_OBJECT_SIZE_LIMIT) {
        chunk = allocate_medium_object(ygc, size, alignment_in_bytes);
    } else {
        NOREACHABLE();
    }
    if (chunk == UINTPTR_MAX) {
        return NULL;
    }
    address_t addr = (address_t)ygc_good_address(chunk);
    dbg_init_zag(addr, size);
    return addr;
}

static void map_page_all_views(struct memory_backing *backing, linear_address_t virtual_mem,
                               struct physical_memory *physical_mem) {
    uintptr_t addr = virtual_mem.addr;
    for (struct memory_segment *s = physical_mem->segments.next; s != &physical_mem->segments; s = s->next) {
        memory_backing_map(backing, ygc_marked0(addr), s->size, s->addr);
        memory_backing_map(backing, ygc_marked1(addr), s->size, s->addr);
        memory_backing_map(backing, ygc_remapped(addr), s->size, s->addr);
        addr += s->size;
    }
    DCHECK(addr - virtual_mem.addr == virtual_mem.size);
}

struct ygc_page *ygc_page_new(struct ygc_core *ygc, size_t size) {
    if (size == 0) {
        return NULL;
    }
    size_t required_in_bytes = ROUND_UP(size, YGC_GRANULE_SIZE);
    linear_address_t virtual_mem = allocate_virtual_memory(&ygc->vmm, required_in_bytes,
                                                           required_in_bytes > MEDIUM_PAGE_SIZE);
    if (virtual_mem.addr == UINTPTR_MAX) {
        DLOG(WARN, "Failed to allocate virtual memory, required size: %zd", required_in_bytes);
        return NULL;
    }

    struct ygc_page *page = MALLOC(struct ygc_page);
    page->next = page;
    page->prev = page;

    if (allocate_physical_memory(&ygc->pmm, &page->physical_memory, required_in_bytes) < 0) {
        free_virtual_memory(&ygc->vmm, virtual_mem);
        free(page);
        DLOG(WARN, "Failed to allocate physical memory, required size: %zd", required_in_bytes);
        return NULL;
    }

    map_page_all_views(&ygc->backing, virtual_mem, &page->physical_memory);


    page->virtual_addr = virtual_mem;
    page->top = page->virtual_addr.addr;
    page->limit = page->top + page->virtual_addr.size;

    yalx_mutex_lock(&ygc->mutex);
    QUEUE_INSERT_TAIL(&ygc->pages, page);
    yalx_mutex_unlock(&ygc->mutex);

    atomic_fetch_add(&ygc->rss, page->virtual_addr.size);

    return page;
}

void ygc_page_free(struct ygc_core *ygc, struct ygc_page *page) {
    if (!page) {
        return;
    }

    yalx_mutex_lock(&ygc->mutex);
    QUEUE_REMOVE(page);
    yalx_mutex_unlock(&ygc->mutex);

    atomic_fetch_sub(&ygc->rss, page->virtual_addr.size);

    const uintptr_t addr = page->virtual_addr.addr;
    const size_t size = page->virtual_addr.size;
    memory_backing_unmap(&ygc->backing, ygc_marked0(addr), size);
    memory_backing_unmap(&ygc->backing, ygc_marked1(addr), size);
    memory_backing_unmap(&ygc->backing, ygc_remapped(addr), size);

    free_virtual_memory(&ygc->vmm, page->virtual_addr);
    free_physical_memory(&ygc->pmm, &page->physical_memory);

    free(page);
}

uintptr_t ygc_page_allocate(struct ygc_page *page, size_t size, uintptr_t alignment_in_bytes) {
    DCHECK(size != 0);
    DCHECK(alignment_in_bytes != 0 && alignment_in_bytes % 2 == 0);

    const uintptr_t chunk = ROUND_UP(page->top, alignment_in_bytes);
    if (chunk + size >= page->limit) {
        return UINTPTR_MAX; // invalid address!
    }

    page->top = chunk + size;
    return chunk;
}

uintptr_t ygc_page_atomic_allocate(struct ygc_page *page, size_t size, uintptr_t alignment_in_bytes) {
    DCHECK(size != 0);
    DCHECK(alignment_in_bytes != 0 && alignment_in_bytes % 2 == 0);

    uintptr_t chunk = UINTPTR_MAX;
    uintptr_t desired = 0;
    do {
        chunk = atomic_load(&page->top);
        desired = ROUND_UP(chunk, alignment_in_bytes) + size;
    } while (!atomic_compare_exchange_strong(&page->top, &chunk, desired));

    chunk = ROUND_UP(chunk, alignment_in_bytes);
    return chunk + size >= page->limit ? UINTPTR_MAX : chunk;
}