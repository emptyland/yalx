#include "runtime/heap/ygc-relocate.h"
#include "runtime/heap/ygc-forwarding.h"
#include "runtime/heap/ygc.h"
#include "runtime/heap/heap.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/object/any.h"
#include "runtime/runtime.h"
#include "runtime/checking.h"
#include <math.h>

static void group_init(struct relocation_set_selector_group *group, const char *name,
                       int page_type,
                       size_t page_size,
                       size_t object_size_limit,
                       int fragmentation_limit) {
    memset(group, 0, sizeof(struct relocation_set_selector_group));
    group->name = name;
    DCHECK(page_type == YGC_PAGE_TYPE_SMALL || page_type == YGC_PAGE_TYPE_MEDIUM || page_type == YGC_PAGE_TYPE_LARGE);
    group->page_type = page_type;
    DCHECK(fragmentation_limit >= 0 && fragmentation_limit <= 100);
    group->fragmentation_limit = fragmentation_limit;
    group->fragmentation_limit_in_bytes = page_size * (fragmentation_limit / 100);
    group->page_size_in_bytes = page_size;
    group->object_size_limit_in_bytes = object_size_limit;
}

static void group_final(struct relocation_set_selector_group *group) {
    free(group->pages);
    free(group->ordered_pages);
}

static void group_make_room_for_pages(struct relocation_set_selector_group *group, size_t new_size) {
    if (new_size >= group->capacity_of_pages) {
        if (!group->pages) {
            group->capacity_of_pages = 16;
        } else {
            group->capacity_of_pages = new_size << 1; // 2 times
        }
        group->pages = realloc(group->pages, group->capacity_of_pages * sizeof(struct ygc_page *));
    }
}

static void group_register_live_page(struct relocation_set_selector_group *group, struct ygc_page *page) {
    size_t garbage_size_in_bytes = page->virtual_addr.size - page->live_map.live_objs_in_bytes;

    if (garbage_size_in_bytes > group->fragmentation_limit_in_bytes) {
        group_make_room_for_pages(group, group->n_pages + 1);
        group->pages[group->n_pages++] = page;
    }

    group->page_count++;
    group->total_in_bytes += page->virtual_addr.size;
    group->live_in_bytes += page->live_map.live_objs_in_bytes;
    group->garbage_in_bytes += garbage_size_in_bytes;
}

static inline void group_register_garbage_page(struct relocation_set_selector_group *group, struct ygc_page *page) {
    group->page_count++;
    group->total_in_bytes += page->virtual_addr.size;
    group->garbage_in_bytes += page->virtual_addr.size;
    group->empty_in_bytes += page->virtual_addr.size;
}

static inline int group_is_disabled(struct relocation_set_selector_group const *group) {
    // Medium pages are disabled when their page size is zero
    return group->page_type == YGC_PAGE_TYPE_MEDIUM && group->page_size_in_bytes == 0;
}

static inline int group_is_selectable(struct relocation_set_selector_group const *group) {
    // Large pages are not selectable
    return group->page_type != YGC_PAGE_TYPE_LARGE;
}

static void group_semi_sort(struct relocation_set_selector_group *group) {
    const size_t partitions_count_shift = 11;
    const size_t partitions_count = ((size_t)1u) << partitions_count_shift;
    const size_t partition_size_in_bytes = group->page_size_in_bytes >> partitions_count_shift;
    const size_t partition_size_shift = yalx_log2(partition_size_in_bytes);

    // Partition slots/fingers
    size_t partitions[partitions_count];
    memset(partitions, 0, sizeof(partitions));

    // Allocate destination array
    DCHECK(group->ordered_pages == NULL);
    group->ordered_pages = MALLOC_N(struct ygc_page *, group->n_pages);

    // Calculate partition slots
    for (size_t i = 0; i < group->n_pages; i++) {
        const size_t index = group->pages[i]->live_map.live_objs_in_bytes >> partition_size_shift;
        partitions[index]++;
    }

    // Calculate partition fingers
    {
        size_t finger = 0;
        for (size_t i = 0; i < partitions_count; i++) {
            const size_t slots = partitions[i];
            partitions[i] = finger;
            finger += slots;
        }
    }

    // Sort pages into partitions
    for (size_t i = 0; i < group->n_pages; i++) {
        const size_t index = group->pages[i]->live_map.live_objs_in_bytes >> partition_size_shift;
        const size_t finger = partitions[index]++;
        DCHECK(group->ordered_pages[finger] == NULL);
        group->ordered_pages[finger] = group->pages[i];
    }
    UNREACHABLE();
}

static inline double percent_of(size_t a, size_t b) {
    return b != 0 ? ((double)a / (double)b) * 100.0 : 0.0;
}

static void group_select_detail(struct relocation_set_selector_group *group) {
    // Calculate the number of pages to relocate by successively including pages in
    // a candidate relocation set and calculate the maximum space requirement for
    // their live objects.
    const size_t n_pages = group->n_pages;
    size_t selected_from = 0;
    size_t selected_to = 0;
    size_t from_size_in_bytes = 0;

    group_semi_sort(group);

    for (size_t from = 1; from <= n_pages; from++) {
        // Add page to the candidate relocation set
        from_size_in_bytes += group->ordered_pages[from - 1]->live_map.live_objs_in_bytes;

        // Calculate the maximum number of pages needed by the candidate relocation set.
        // By subtracting the object size limit from the pages size we get the maximum
        // number of pages that the relocation set is guaranteed to fit in, regardless
        // of in which order the objects are relocated.
        const size_t to = ceil((double)from_size_in_bytes /
                (double)(group->page_size_in_bytes - group->object_size_limit_in_bytes));

        // Calculate the relative difference in reclaimable space compared to our
        // currently selected final relocation set. If this number is larger than the
        // acceptable fragmentation limit, then the current candidate relocation set
        // becomes our new final relocation set.
        const size_t diff_from = from - selected_from;
        const size_t diff_to = to - selected_to;
        const double diff_reclaimable = 100 - percent_of(diff_to, diff_from);
        if (diff_reclaimable > group->fragmentation_limit) {
            selected_from = from;
            selected_to = to;
        }
    }

    group->n_selected = selected_from;

    group->compacting_from_in_bytes = selected_from * group->page_size_in_bytes;
    group->compacting_to_in_bytes = selected_to * group->page_size_in_bytes;
}

static void group_select(struct relocation_set_selector_group *group) {
    if (group_is_disabled(group)) {
        return; // Ignore it
    }

    if (group_is_selectable(group)) {
        group_select_detail(group);
    }
}


void relocation_set_selector_init(struct relocation_set_selector *self, int fragmentation_limit) {
    group_init(&self->small, "Small", YGC_PAGE_TYPE_SMALL, SMALL_PAGE_SIZE, SMALL_OBJECT_SIZE_LIMIT, fragmentation_limit);
    group_init(&self->medium, "Medium", YGC_PAGE_TYPE_MEDIUM, MEDIUM_PAGE_SIZE, MEDIUM_OBJECT_SIZE_LIMIT, fragmentation_limit);
    group_init(&self->large, "Large", YGC_PAGE_TYPE_LARGE, 0, 0, fragmentation_limit);
}

void relocation_set_selector_final(struct relocation_set_selector *self) {
    group_final(&self->large);
    group_final(&self->medium);
    group_final(&self->small);
}

void relocation_set_selector_register_live_page(struct relocation_set_selector *self, struct ygc_page *page) {
    if (ygc_is_small_page(page)) {
        group_register_live_page(&self->small, page);
    } else if (ygc_is_medium_page(page)) {
        group_register_live_page(&self->medium, page);
    } else {
        DCHECK(ygc_is_large_page(page));
        group_register_live_page(&self->large, page);
    }
}

void relocation_set_selector_register_garbage_page(struct relocation_set_selector *self, struct ygc_page *page) {
    if (ygc_is_small_page(page)) {
        group_register_garbage_page(&self->small, page);
    } else if (ygc_is_medium_page(page)) {
        group_register_garbage_page(&self->medium, page);
    } else {
        DCHECK(ygc_is_large_page(page));
        group_register_garbage_page(&self->large, page);
    }
}

void relocation_set_selector_select(struct relocation_set_selector *self, struct ygc_relocation_set *set) {
    group_select(&self->small);
    group_select(&self->medium);
    group_select(&self->large);

    ygc_relocation_set_populate(set, self->medium.ordered_pages, self->medium.n_selected,
                                self->small.ordered_pages, self->small.n_selected);
}

void ygc_relocation_set_init(struct ygc_relocation_set *set) {
    set->size = 0;
    set->forwards = NULL;
}

void ygc_relocation_set_final(struct ygc_relocation_set *set) {
    ygc_relocation_set_reset(set);
    free(set->forwards);
    set->size = 0;
}

void ygc_relocation_set_populate(struct ygc_relocation_set *set, struct ygc_page *const *group0, size_t size0,
                                 struct ygc_page *const *group1, size_t size1) {
    DCHECK(set->size == 0);
    DCHECK(set->forwards == NULL);

    set->size = size0 + size1;
    set->forwards = realloc(set->forwards, set->size * sizeof(struct forwarding *));

    size_t x = 0;
    for (int i = 0; i < size0; i++) {
        set->forwards[x++] = forwarding_new(group0[i]);
    }
    for (int i = 0; i < size1; i++) {
        set->forwards[x++] = forwarding_new(group1[i]);
    }
}

void ygc_relocation_set_reset(struct ygc_relocation_set *set) {
    for (int i = 0; i < set->size; i++) {
        if (set->forwards[i]) {
            forwarding_free(set->forwards[i]);
            set->forwards[i] =NULL;
        }
    }
}

void ygc_relocate_init(struct ygc_relocate *relocate, struct ygc_core *owns) {
    relocate->owns = owns;
    yalx_job_init(&relocate->job, "yalx-relocating", ncpus / 2 < 4 ? 4 : ncpus / 2);
}

void ygc_relocate_final(struct ygc_relocate *relocate) {
    yalx_job_final(&relocate->job);
}

uintptr_t ygc_relocating_forward_object(struct ygc_relocate *relocate, struct forwarding *fwd, uintptr_t from_addr) {
    const uintptr_t from_offset = ygc_offset(from_addr);
    DCHECK(forwarding_contains_addr(fwd, from_offset));
    const uintptr_t from_index = (from_offset - fwd->virtual_addr.addr) >> pointer_shift_in_bytes;
    size_t dummy = 0;
    const struct forwarding_entry entry = forwarding_find(fwd, from_index, &dummy);

    DCHECK(entry.populated && "Should be forwarded");
    DCHECK(entry.from_index == from_index && "Should be forwarded");

    return ygc_good_address(entry.to_offset);
}

static uintptr_t relocate_object(const struct ygc_relocate *relocate, struct forwarding *fwd, uintptr_t from_index,
                                 uintptr_t from_offset) {
    size_t pos = 0;

    struct forwarding_entry const entry = forwarding_find(fwd, from_index, &pos);
    if (entry.populated && entry.from_index == from_index) {
        // Already relocated, return new address
        return entry.to_offset;
    }

    // TODO: check `from_offset' is live

    if (atomic_load(&fwd->pinned)) {
        // In-place forward
        forwarding_insert(fwd, from_index, from_offset, &pos);
    }

    // Allocate object
    const uintptr_t from_good = ygc_good_address(from_offset);
    const size_t size_in_bytes = yalx_object_size_in_bytes((yalx_ref_t)from_good);
    const uintptr_t to_good = (uintptr_t)ygc_allocate_object(relocate->owns, size_in_bytes,
                                                             pointer_size_in_bytes);
    if (!to_good) {
        // Failed, in-place forward
        forwarding_insert(fwd, from_index, from_offset, &pos);
    }

    memcpy((void *)to_good, (void *)from_good, size_in_bytes);

    const uintptr_t to_offset = ygc_offset(to_good);
    const uintptr_t to_offset_final = forwarding_insert(fwd, from_index, to_offset, &pos);
    if (to_offset == to_offset_final) {
        // Ok
        return to_offset;
    }

    DLOG(INFO, "Relocation: from_good=%p, size=%zd", from_good, size_in_bytes);

    return to_offset_final;
}

uintptr_t ygc_relocating_relocate_object(struct ygc_relocate *relocate, struct forwarding *fwd, uintptr_t from_addr) {
    const uintptr_t from_offset = ygc_offset(from_addr);
    DCHECK(forwarding_contains_addr(fwd, from_offset));
    const uintptr_t from_index = (from_offset - fwd->virtual_addr.addr) >> pointer_shift_in_bytes;
    const uintptr_t to_offset = relocate_object(relocate, fwd, from_index, from_offset);

    if (from_offset == to_offset) {
        // In-place forwarding, pin page
        forwarding_set_pinned(fwd, 1);
    }

    return ygc_good_address(to_offset);
}

static void visit_root_pointer(struct yalx_root_visitor *v, yalx_ref_t *p) {
    struct ygc_core *ygc = (struct ygc_core *)v->ctx;
    ygc_barrier_relocate_on_root(ygc, *p, p);
}

static void visit_root_pointers(struct yalx_root_visitor *v, yalx_ref_t *begin, yalx_ref_t *end) {
    struct ygc_core *ygc = (struct ygc_core *)v->ctx;
    for (yalx_ref_t *x = begin; x < end; x++) {
        ygc_barrier_relocate_on_root(ygc, *x, x);
    }
}

void ygc_relocating_start(struct ygc_relocate *relocate, struct heap *h) {
    // Mark roots objects
    struct yalx_root_visitor visitor = {
            relocate->owns,
            0,
            0,
            visit_root_pointers,
            visit_root_pointer,
    };
    yalx_heap_visit_root(h, &visitor);
    yalx_global_visit_root(&visitor);

    DLOG(WARN, "Visit coroutines and others...");
}