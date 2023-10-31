#include "runtime/heap/ygc.h"
#include "runtime/heap/ygc-forwarding.h"
#include "runtime/heap/heap.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/object/any.h"
#include "runtime/root-handles.h"
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

const char *ygc_desc_address(uintptr_t addr) {
    if (addr & YGC_METADATA_REMAPPED) {
        return "remapped";
    } else if (addr & YGC_METADATA_MARKED0) {
        return "marked0";
    } else if (addr & YGC_METADATA_MARKED1) {
        return "marked1";
    } else {
        return "unknown";
    }
}

int ygc_init(struct ygc_core *ygc, size_t capacity, int fragmentation_limit) {
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
    DCHECK(fragmentation_limit >= 0 && fragmentation_limit <= 100);
    ygc->fragmentation_limit = fragmentation_limit;
    ygc->medium_page = NULL;
    ygc->small_page = per_cpu_storage_new();
    ygc->pages.next = &ygc->pages;
    ygc->pages.prev = &ygc->pages;

    granule_map_init(&ygc->page_granules);
    granule_map_init(&ygc->forwarding_table);
    ygc_mark_init(&ygc->mark, ygc);
    ygc_relocate_init(&ygc->relocate, ygc);
    ygc_relocation_set_init(&ygc->relocation_set);
    yalx_mutex_init(&ygc->mutex);

    ygc_flip_to_remapped(); // Initialize state
    ygc_global_phase = YGC_PHASE_RELOCATE;
    ygc_global_tick = 1;

    return 0;
}

void ygc_final(struct ygc_core *ygc) {
    // TODO:

    while (!QUEUE_EMPTY(&ygc->pages)) {
        struct ygc_page *const page = ygc->pages.next;
        QUEUE_REMOVE(page);
        DLOG(INFO, "<Free Page> Cause ygc heap final: [%p, %p) (%zd)",
             page->virtual_addr.addr,
             page->virtual_addr.addr + page->virtual_addr.size,
             page->virtual_addr.size);
        ygc_page_free(ygc, page, 0/*dont should locking*/);
    }
    yalx_mutex_final(&ygc->mutex);
    ygc_mark_final(&ygc->mark);
    ygc_relocation_set_final(&ygc->relocation_set);
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
    //DCHECK(h->gc == GC_YGC);
    GUARANTEE(h->gc == GC_YGC, "Heap mut be YGC, actual is %d", h->gc);
    return (struct ygc_core *)(h + 1);
}

void ygc_thread_enter(struct heap *h, struct yalx_os_thread *thread) {
    DCHECK(h != NULL);
    //DCHECK(h->gc == GC_YGC);
    DCHECK(sizeof(struct ygc_tls_struct) <= sizeof(thread->gc_data));

    struct ygc_tls_struct *tls = (struct ygc_tls_struct *)thread->gc_data;
    tls->bad_mask = YGC_ADDRESS_BAD_MASK;
    for (int i = 0; i < YGC_MAX_MARKING_STRIPES; i++) {
        tls->stacks[i] = NULL;
    }

    // TODO:
}

void ygc_thread_exit(struct heap *h, struct yalx_os_thread *thread) {
    // TODO:
    if (h->gc != GC_YGC) {
        return; // For testing: ignore non-ygc...
    }
    struct ygc_core *ygc = ygc_heap_of(h);
    struct ygc_tls_struct *tls = (struct ygc_tls_struct *)thread->gc_data;
    for (int i = 0; i < YGC_MAX_MARKING_STRIPES; i++) {
        if (tls->stacks[i]) {
            struct ygc_marking_stripe *stripe = &ygc->mark.stripes[i];
            ygc_marking_stripe_commit(stripe, tls->stacks[i]);
            tls->stacks[i] = NULL;
        }
    }
}

struct ygc_tls_struct *ygc_tls_data() {
    return (struct ygc_tls_struct *)yalx_os_thread_self()->gc_data;
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

void ygc_page_free(struct ygc_core *ygc, struct ygc_page *page, int should_locking) {
    if (!page) {
        return;
    }

    if (should_locking) { yalx_mutex_lock(&ygc->mutex); }
    QUEUE_REMOVE(page);
    page_granule_remove(&ygc->page_granules, page);
    if (should_locking) { yalx_mutex_unlock(&ygc->mutex); }

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

uintptr_t ygc_barrier_mark(struct ygc_core *ygc, uintptr_t addr) {
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

struct yalx_value_any *ygc_barrier_load(struct yalx_value_any *_Atomic volatile *p) {
    return atomic_load_explicit(p, memory_order_relaxed);
}

void ygc_barrier_self_heal_fast_on_good_or_null(struct yalx_value_any *_Atomic volatile *p, uintptr_t addr,
                                                uintptr_t heal_addr) {
    if (heal_addr == 0) {
        // Never heal with null since it interacts badly with reference processing.
        // A mutator clearing an oop would be similar to calling Reference.clear(),
        // which would make the reference non-discoverable or silently dropped
        // by the reference processor.
        return;
    }

    DCHECK(!ygc_is_good(addr) && addr != 0);
    DCHECK(ygc_is_good(heal_addr) || heal_addr == 0);

    for (;;) {
        if (atomic_compare_exchange_strong((volatile _Atomic uintptr_t *)p, &addr, heal_addr)) {
            // Ok
            return;
        }

        if (ygc_is_good(addr) || addr == 0) {
            // Must not self heal
            return;
        }

        // The oop location was healed by another barrier, but still needs upgrading.
        // Re-apply healing to make sure the oop is not left with weaker (remapped or
        // finalizable) metadata bits than what this barrier tried to apply.
        DCHECK(ygc_offset(addr) == ygc_offset(heal_addr));
    }
}

static void visit_root_pointer(struct yalx_root_visitor *v, yalx_ref_t *p) {
    struct ygc_core *ygc = (struct ygc_core *)v->ctx;
    ygc_barrier_mark_on_root(ygc, *p, p);
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
    yalx_root_handles_visit(&visitor);

    DLOG(WARN, "Visit coroutines and others for ygc_mark_start()...");
}

void ygc_mark(struct heap *h, int initial) {
    struct ygc_core *ygc = ygc_heap_of(h);

    if (initial) {
        // TODO: mark roots: Threads/Coroutines stacks
    }

    ygc_marking_concurrent_mark(&ygc->mark);
}

void ygc_reset_relocation_set(struct heap *h) {
    struct ygc_core *ygc = ygc_heap_of(h);

    for (size_t i = 0; i < ygc->relocation_set.size; i++) {
        forwarding_table_remove(&ygc->forwarding_table, ygc->relocation_set.forwards[i]);
    }

    ygc_relocation_set_reset(&ygc->relocation_set);
}

void ygc_select_relocation_set(struct ygc_core *ygc) {
    struct relocation_set_selector selector;
    relocation_set_selector_init(&selector, ygc->fragmentation_limit);

    yalx_mutex_lock(&ygc->mutex);
    for (struct ygc_page *page = ygc->pages.next; page != &ygc->pages; page = page->next) {
        if (!ygc_page_is_relocatable(page)) {
            continue;
        }

        if (ygc_page_is_marked(page)) {
            relocation_set_selector_register_live_page(&selector, page);
        } else {
            relocation_set_selector_register_garbage_page(&selector, page);

            DLOG(INFO, "Free page: [%p, %p) (%zd)",
                 page->virtual_addr.addr,
                 page->virtual_addr.addr + page->virtual_addr.size,
                 page->virtual_addr.size);
            ygc_page_free(ygc, page, 0/*dont should lock*/);
        }
    }
    yalx_mutex_unlock(&ygc->mutex);

    relocation_set_selector_select(&selector, &ygc->relocation_set);
    // Setup forwarding table
    for (size_t i = 0; i < ygc->relocation_set.size; i++) {
        forwarding_table_insert(&ygc->forwarding_table, ygc->relocation_set.forwards[i]);
    }

    relocation_set_selector_final(&selector);
}

void ygc_relocate_start(struct heap *h) {
    struct ygc_core *ygc = ygc_heap_of(h);

    ygc_flip_to_remapped();

    ygc_global_phase = YGC_PHASE_RELOCATE;

    // Remap/Relocate roots
    ygc_relocating_start(&ygc->relocate, h);
}

void ygc_relocate(struct heap *h) {
    struct ygc_core *ygc = ygc_heap_of(h);
    ygc_relocating_relocate(&ygc->relocate);
}

uintptr_t ygc_remap_object(struct ygc_core *ygc, uintptr_t addr) {
    DCHECK(ygc_global_phase == YGC_PHASE_MARK || ygc_global_phase == YGC_PHASE_MARK_COMPLETE);

    struct forwarding *fwd = forwarding_table_get(&ygc->forwarding_table, addr);
    if (!fwd) {
        // Not forwarding
        return ygc_good_address(addr);
    }
    return ygc_relocating_forward_object(&ygc->relocate, fwd, addr);
}

uintptr_t ygc_relocate_object(struct ygc_core *ygc, uintptr_t addr) {
    struct forwarding *const fwd = forwarding_table_get(&ygc->forwarding_table, addr);
    if (!fwd) {
        return ygc_good_address(addr);
    }

    const int retained = forwarding_grab_page(fwd);
    const uintptr_t new_addr = ygc_relocating_relocate_object(&ygc->relocate, fwd, addr);
    if (retained) {
        forwarding_drop_page(fwd, ygc);
    }

    return new_addr;
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

size_t ygc_page_object_max_count(struct ygc_page const *page) {
    if (ygc_is_large_page(page)) {
        return 1;
    }
    return page->virtual_addr.size >> pointer_shift_in_bytes;
}

void ygc_page_mark_object(struct ygc_page *page, struct yalx_value_any *obj) {
    const uintptr_t offset = ygc_offset(obj);
    DCHECK(offset >= page->virtual_addr.addr && offset < page->top);
    DCHECK(offset % YGC_ALLOCATION_ALIGNMENT_SIZE == 0);

    int index = (int)((offset - page->virtual_addr.addr) >> pointer_shift_in_bytes);
    live_map_set(&page->live_map, index);
    live_map_increase_obj(&page->live_map, 1, yalx_object_size_in_bytes(obj));
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
