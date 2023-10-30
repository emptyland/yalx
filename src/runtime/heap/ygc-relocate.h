#pragma once
#ifndef YALX_RUNTIME_HEAP_YGC_RELOCATE_H
#define YALX_RUNTIME_HEAP_YGC_RELOCATE_H

#include "runtime/jobs.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ygc_relocation_set;
struct forwarding;
struct ygc_core;
struct ygc_page;
struct heap;

struct relocation_set_selector_group {
    const char *name;
    int page_type;
    int fragmentation_limit;
    size_t fragmentation_limit_in_bytes;
    size_t page_size_in_bytes;
    size_t object_size_limit_in_bytes;

    struct ygc_page **pages;
    struct ygc_page **ordered_pages;
    size_t n_pages;
    size_t capacity_of_pages;
    size_t n_selected;

    size_t page_count;
    size_t total_in_bytes;
    size_t live_in_bytes;
    size_t garbage_in_bytes;
    size_t empty_in_bytes;
    size_t compacting_from_in_bytes;
    size_t compacting_to_in_bytes;
};

struct relocation_set_selector {
    struct relocation_set_selector_group small;
    struct relocation_set_selector_group medium;
    struct relocation_set_selector_group large;
};

void relocation_set_selector_init(struct relocation_set_selector *self, int fragmentation_limit);
void relocation_set_selector_final(struct relocation_set_selector *self);
void relocation_set_selector_register_live_page(struct relocation_set_selector *self, struct ygc_page *page);
void relocation_set_selector_register_garbage_page(struct relocation_set_selector *self, struct ygc_page *page);
void relocation_set_selector_select(struct relocation_set_selector *self, struct ygc_relocation_set *set);

struct ygc_relocation_set {
    struct forwarding **forwards;
    size_t size;
};

void ygc_relocation_set_init(struct ygc_relocation_set *set);
void ygc_relocation_set_final(struct ygc_relocation_set *set);
void ygc_relocation_set_populate(struct ygc_relocation_set *set, struct ygc_page *const *group0, size_t size0,
                                 struct ygc_page *const *group1, size_t size1);
void ygc_relocation_set_reset(struct ygc_relocation_set *set);


struct ygc_relocation_set_parallel_iter {
    struct ygc_relocate *const relocate;
    struct ygc_relocation_set const *const owns;
    _Atomic volatile size_t next;
};

int ygc_relocation_set_parallel_iter_next(struct ygc_relocation_set_parallel_iter *iter, struct forwarding **fwd);

struct ygc_relocate {
    struct ygc_core *owns;
    struct yalx_job job;
};

void ygc_relocate_init(struct ygc_relocate *relocate, struct ygc_core *owns);
void ygc_relocate_final(struct ygc_relocate *relocate);
uintptr_t ygc_relocating_forward_object(struct ygc_relocate *relocate, struct forwarding *fwd, uintptr_t from_addr);
uintptr_t ygc_relocating_relocate_object(struct ygc_relocate *relocate, struct forwarding *fwd, uintptr_t from_addr);

void ygc_relocating_start(struct ygc_relocate *relocate, struct heap *h);
void ygc_relocating_relocate(struct ygc_relocate *relocate);

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_HEAP_YGC_RELOCATE_H
