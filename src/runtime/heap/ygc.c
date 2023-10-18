#include "runtime/heap/ygc.h"
#include "runtime/heap/ygc-forwarding.h"
#include "runtime/heap/heap.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/object/any.h"
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

size_t YGC_ADDRESS_OFFSET_MAX = (1L << YGC_METADATA_SHIFT);


uintptr_t YGC_ADDRESS_GOOD_MASK;
uintptr_t YGC_ADDRESS_BAD_MASK;
uintptr_t YGC_ADDRESS_WEAK_BAD_MASK;

enum ygc_phase ygc_global_phase = YGC_PHASE_RELOCATE;
uint32_t ygc_global_tick = 1;

void ygc_set_good_mask(uintptr_t mask) {
    YGC_ADDRESS_GOOD_MASK = mask;
    YGC_ADDRESS_BAD_MASK = YGC_ADDRESS_GOOD_MASK ^ YGC_METADATA_MASK;
    YGC_ADDRESS_WEAK_BAD_MASK = (YGC_ADDRESS_GOOD_MASK | YGC_METADATA_REMAPPED) ^ YGC_METADATA_MASK;
}

void ygc_flip_to_marked(void) {
    YGC_METADATA_MARKED ^= (YGC_METADATA_MARKED0 | YGC_METADATA_MARKED1);
    ygc_set_good_mask(YGC_METADATA_MARKED);
}

void ygc_flip_to_remapped(void) {
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
    if (virtual_memory_management_init(&ygc->vmm, capacity) < 0) {
        memory_backing_final(&ygc->backing);
        physical_memory_management_final(&ygc->pmm);
        return -1;
    }
    ygc->rss = 0;
    ygc->medium_page = NULL;
    ygc->small_page = per_cpu_storage_new();
    ygc->pages.next = &ygc->pages;
    ygc->pages.prev = &ygc->pages;

    granule_map_init(&ygc->page_granules);
    granule_map_init(&ygc->forwarding_table);
    ygc_mark_init(&ygc->mark);
    ygc_relocate_init(&ygc->relocate);
    yalx_mutex_init(&ygc->mutex);

    ygc_flip_to_remapped(); // Initialize state
    ygc_global_phase = YGC_PHASE_RELOCATE;
    ygc_global_tick = 1;

    return 0;
}

void ygc_final(struct ygc_core *ygc) {
    // TODO:

    while (!QUEUE_EMPTY(&ygc->pages)) {
        struct ygc_page *page = ygc->pages.next;
        QUEUE_REMOVE(page);
        ygc_page_free(ygc, page);
    }
    yalx_mutex_final(&ygc->mutex);
    ygc_mark_final(&ygc->mark);
    ygc_relocate_final(&ygc->relocate);
    granule_map_final(&ygc->forwarding_table);
    granule_map_final(&ygc->page_granules);

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

struct ygc_core *ygc_heap_of(struct heap *h) {
    DCHECK(h != NULL);
    DCHECK(h->gc == GC_YGC);
    return (struct ygc_core *)(h + 1);
}

static uintptr_t allocate_object_from_shared_page(struct ygc_core *ygc,
                                                  struct ygc_page *_Atomic *shared_page,
                                                  size_t page_size,
                                                  size_t size,
                                                  uintptr_t alignment_in_bytes) {
    static struct ygc_page *const PAGE_BUSY = (struct ygc_page *)0x1;

    for (;;) {
        struct ygc_page *page;
        do {
            page = atomic_load_explicit(shared_page, memory_order_acquire);
        } while (page == PAGE_BUSY);
        if (page) {
            uintptr_t chunk = ygc_page_atomic_allocate(page, size, alignment_in_bytes);
            if (chunk != UINTPTR_MAX) {
                return chunk;
            }
        }

        if (atomic_compare_exchange_strong(shared_page, &page, PAGE_BUSY)) {
            page = ygc_page_new(ygc, page_size);
            if (!page) {
                atomic_store_explicit(shared_page, NULL, memory_order_release);
                return UINTPTR_MAX;
            }
            uintptr_t chunk = ygc_page_allocate(page, size, alignment_in_bytes);
            atomic_store_explicit(shared_page, page, memory_order_release);
            return chunk;
        }
    }
}


static uintptr_t allocate_small_object(struct ygc_core *ygc, size_t size, uintptr_t alignment_in_bytes) {
    struct ygc_page *_Atomic *shared_page = per_cpu_at(struct ygc_page *_Atomic *, ygc->small_page);
    return allocate_object_from_shared_page(ygc, shared_page, SMALL_PAGE_SIZE, size, alignment_in_bytes);
}


static uintptr_t allocate_medium_object(struct ygc_core *ygc, size_t size, uintptr_t alignment_in_bytes) {
    return allocate_object_from_shared_page(ygc, &ygc->medium_page, MEDIUM_PAGE_SIZE, size,
                                            alignment_in_bytes);
}

uintptr_t allocate_large_object(struct ygc_core *ygc, size_t size, uintptr_t alignment_in_bytes) {
    DCHECK(alignment_in_bytes != 0);
    DCHECK(alignment_in_bytes % 2 == 0);

    const size_t request_in_bytes = ROUND_UP(size + alignment_in_bytes, YGC_GRANULE_SIZE);
    struct ygc_page *const page = ygc_page_new(ygc, request_in_bytes);
    if (!page) {
        DLOG(ERROR, "Failed to allocating large page, size=%zd", request_in_bytes);
        return UINTPTR_MAX;
    }
    return ygc_page_allocate(page, size, alignment_in_bytes);
}

address_t ygc_allocate_object(struct ygc_core *ygc, size_t size, uintptr_t alignment_in_bytes) {
    uintptr_t chunk = UINTPTR_MAX;
    if (size < SMALL_OBJECT_SIZE_LIMIT) {
        chunk = allocate_small_object(ygc, size, alignment_in_bytes);
    } else if (size < MEDIUM_OBJECT_SIZE_LIMIT) {
        chunk = allocate_medium_object(ygc, size, alignment_in_bytes);
    } else {
        chunk = allocate_large_object(ygc, size, alignment_in_bytes);
    }
    if (chunk == UINTPTR_MAX) {
        return NULL;
    }
    address_t addr = (address_t)ygc_good_address(chunk);
    if (size < MEDIUM_OBJECT_SIZE_LIMIT) {
        dbg_init_zag(addr, size);
    }
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

static void page_granule_insert(struct ygc_granule_map *map, struct ygc_page *page) {
    for (uintptr_t addr = page->virtual_addr.addr; addr < page->limit; addr += YGC_GRANULE_SIZE) {
        granule_map_put(map, addr, (uintptr_t)page);
    }
}

static void page_granule_remove(struct ygc_granule_map *map, struct ygc_page *page) {
    for (uintptr_t addr = page->virtual_addr.addr; addr < page->limit; addr += YGC_GRANULE_SIZE) {
        granule_map_put(map, addr, 0);
    }
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


    page->tick = ygc_global_tick;
    page->virtual_addr = virtual_mem;
    page->top = page->virtual_addr.addr;
    page->limit = page->top + page->virtual_addr.size;

    live_map_init(&page->live_map, yalx_log2(512));

    yalx_mutex_lock(&ygc->mutex);
    QUEUE_INSERT_TAIL(&ygc->pages, page);
    page_granule_insert(&ygc->page_granules, page);
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
    page_granule_remove(&ygc->page_granules, page);
    yalx_mutex_unlock(&ygc->mutex);

    atomic_fetch_sub(&ygc->rss, page->virtual_addr.size);

    live_map_final(&page->live_map);

    const uintptr_t addr = page->virtual_addr.addr;
    const size_t size = page->virtual_addr.size;
    memory_backing_unmap(&ygc->backing, ygc_marked0(addr), size);
    memory_backing_unmap(&ygc->backing, ygc_marked1(addr), size);
    memory_backing_unmap(&ygc->backing, ygc_remapped(addr), size);

    free_virtual_memory(&ygc->vmm, page->virtual_addr);
    free_physical_memory(&ygc->pmm, &page->physical_memory);

    free(page);
}

static uintptr_t barrier_mark(struct ygc_core *ygc, uintptr_t addr) {
    uintptr_t good_addr = 0;

    if (ygc_is_marked(addr)) {
        // Already marked, but try to mark though anyway
        good_addr = ygc_good_address(addr);
    } else if (ygc_is_remapped(addr)) {
        // Already remapped, but also needs to be marked
        good_addr = ygc_good_address(addr);
    } else {
        // Needs to be both remapped and marked
        DCHECK(!ygc_is_good(addr));
        DCHECK(!ygc_is_weak_good(addr));
        good_addr = ygc_remap_object(ygc, addr);
    }

    if (ygc_global_phase == YGC_PHASE_MARK) {
        ygc_mark_object(ygc, good_addr);
    }
    return good_addr;
}

static inline void root_barrier_field(struct ygc_core *ygc, struct yalx_value_any *o, struct yalx_value_any **p) {
    uintptr_t addr = (uintptr_t)o;
    if (ygc_is_good(addr) || addr == 0) {
        return;
    }
    uintptr_t good_addr = barrier_mark(ygc, addr);
    *p = (struct yalx_value_any *)good_addr;
}

static void visit_root_pointer(struct yalx_root_visitor *v, yalx_ref_t *p) {
    struct ygc_core *ygc = (struct ygc_core *)v->ctx;
//    struct ygc_page *owns_page = ygc_addr_in_page(ygc, (uintptr_t) *p);
//    DCHECK(owns_page != NULL);
    root_barrier_field(ygc, *p, p);
}

static void visit_root_pointers(struct yalx_root_visitor *v, yalx_ref_t *begin, yalx_ref_t *end) {
    for (yalx_ref_t *x = begin; x < end; x++) {
        visit_root_pointer(v, x);
    }
}

void ygc_mark_start(struct heap *h) {
    // should at safepoint
    struct ygc_core *ygc = ygc_heap_of(h);

    ygc_flip_to_marked();

    // Retire allocating pages
    for (int i = 0; i < ygc->small_page->n; i++) {
        ygc->small_page->items[i] = NULL;
    }
    ygc->medium_page = NULL;

    // Enter mark phase
    ygc_global_phase = YGC_PHASE_MARK;

    // Increment global sequence number to invalidate
    // marking information for all pages.
    ygc_global_tick++;

    // Mark roots objects
    struct yalx_root_visitor visitor = {
        ygc,
        0,
        0,
        visit_root_pointers,
        visit_root_pointer,
    };
    yalx_heap_visit_root(h, &visitor);
    yalx_global_visit_root(&visitor);
}

void ygc_mark(struct heap *h, int initial) {
    // should at safepoint
    struct ygc_core *ygc = ygc_heap_of(h);

    if (initial) {
        // TODO: mark roots
    }

    UNREACHABLE();
}

uintptr_t ygc_remap_object(struct ygc_core *ygc, uintptr_t addr) {
    DCHECK(ygc_global_phase == YGC_PHASE_MARK || ygc_global_phase == YGC_PHASE_MARK_COMPLETE);

    struct forwarding *fwd = forwarding_table_get(&ygc->forwarding_table, addr);
    if (!fwd) {
        // Not forwarding
        return ygc_good_address(addr);
    }
    return ygc_relocate_forward_object(&ygc->relocate, fwd, addr);
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

    uintptr_t addr = page->top;
    uintptr_t new_top;
    do {
        new_top = ROUND_UP(addr, alignment_in_bytes) + size;
    } while (!atomic_compare_exchange_strong(&page->top, &addr, new_top));
    uintptr_t chunk = ROUND_UP(addr, alignment_in_bytes);
    return chunk + size >= page->limit ? UINTPTR_MAX : chunk;
}

void ygc_page_mark_object(struct ygc_page *page, struct yalx_value_any *obj) {
    const uintptr_t offset = ygc_offset(obj);
    DCHECK(offset >= page->virtual_addr.addr && offset < page->top);
    DCHECK(offset % YGC_ALLOCATION_ALIGNMENT_SIZE == 0);

    live_map_increase_obj(&page->live_map, 1, yalx_object_size_in_bytes(obj));
    int index = (int)((offset - page->virtual_addr.addr) >> pointer_shift_in_bytes);
    live_map_set(&page->live_map, index);
}

void ygc_page_visit_objects(struct ygc_page *page, struct yalx_heap_visitor *visitor) {
    uintptr_t addr = ROUND_UP(page->virtual_addr.addr, YGC_ALLOCATION_ALIGNMENT_SIZE);
    while (addr < atomic_load_explicit(&page->top, memory_order_acquire)) {
        yalx_ref_t obj = (yalx_ref_t) ygc_good_address(addr);
        visitor->visit_pointer(visitor, obj);
        uintptr_t next = addr + yalx_object_size_in_bytes(obj);
        addr = ROUND_UP(next, YGC_ALLOCATION_ALIGNMENT_SIZE);
    }
}
