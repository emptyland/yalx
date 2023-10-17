#pragma once
#ifndef YALX_RUNTIME_HEAP_YGC_H
#define YALX_RUNTIME_HEAP_YGC_H

#include "runtime/heap/ygc-live-map.h"
#include "runtime/heap/ygc-mark.h"
#include "runtime/locks.h"
#include "runtime/runtime.h"
#include "runtime/locks.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YGC_GRANULE_SHIFT 21
#define YGC_GRANULE_SIZE (1L << YGC_GRANULE_SHIFT)

#define SMALL_PAGE_SHIFT YGC_GRANULE_SHIFT // 2MB
#define SMALL_PAGE_SIZE (1L << SMALL_PAGE_SHIFT)

#define MEDIUM_PAGE_SHIFT (YGC_GRANULE_SHIFT + 4) // x16
#define MEDIUM_PAGE_SIZE (1L << MEDIUM_PAGE_SHIFT)

#define SMALL_OBJECT_SIZE_LIMIT (1L << (SMALL_PAGE_SHIFT - 3))  // SMALL_PAGE_SIZE / 8 12.5%
#define MEDIUM_OBJECT_SIZE_LIMIT (1L << (MEDIUM_PAGE_SHIFT - 3))  // MEDIUM_PAGE_SIZE / 8 12.5%

#define YGC_ALLOCATION_ALIGNMENT_SIZE 8

#define YGC_METADATA_SHIFT 44

enum ygc_phase {
    YGC_PHASE_MARK,
    YGC_PHASE_MARK_COMPLETE,
    YGC_PHASE_RELOCATE,
};

extern uintptr_t YGC_METADATA_MARKED0; // 0x0000100000000000
extern uintptr_t YGC_METADATA_MARKED1; // 0x0000200000000000
extern uintptr_t YGC_METADATA_REMAPPED; // 0x0000400000000000
extern uintptr_t YGC_METADATA_MARKED;

extern uintptr_t YGC_METADATA_MASK;
extern uintptr_t YGC_ADDRESS_OFFSET_MASK;
extern size_t YGC_ADDRESS_OFFSET_MAX;

extern uintptr_t YGC_ADDRESS_GOOD_MASK;
extern uintptr_t YGC_ADDRESS_BAD_MASK;
extern uintptr_t YGC_ADDRESS_WEAK_BAD_MASK;

extern enum ygc_phase ygc_global_phase;
extern uint32_t ygc_global_tick;

void ygc_set_good_mask(uintptr_t mask);
void ygc_flip_to_remapped(void);
void ygc_flip_to_marked(void);

#define ygc_offset(addr) ((uintptr_t)(addr) & YGC_ADDRESS_OFFSET_MASK)
#define ygc_marked0(addr) ((uintptr_t)(addr) | YGC_METADATA_MARKED0)
#define ygc_marked1(addr) ((uintptr_t)(addr) | YGC_METADATA_MARKED1)
#define ygc_remapped(addr) ((uintptr_t)(addr) | YGC_METADATA_REMAPPED)

#define ygc_good_address(addr) (ygc_offset(addr) | YGC_ADDRESS_GOOD_MASK)

#define ygc_is_bad(addr)       ((uintptr_t)(addr) & YGC_ADDRESS_BAD_MASK)
#define ygc_is_weak_bad(addr)  ((uintptr_t)(addr) & YGC_ADDRESS_WEAK_BAD_MASK)
#define ygc_is_good(addr)      !(ygc_is_bad(addr))
#define ygc_is_weak_good(addr) !(ygc_is_weak_bad(addr))
#define ygc_is_marked(addr)    ((uintptr_t)(addr) & YGC_METADATA_MARKED)
#define ygc_is_remapped(addr)  ((uintptr_t)(addr) & YGC_METADATA_REMAPPED)

struct per_cpu_storage;
struct yalx_heap_visitor;
struct heap;

struct linear_address {
    uintptr_t addr;
    size_t    size;
};

typedef struct linear_address linear_address_t;

struct memory_backing {
    size_t size;
#if defined(YALX_OS_LINUX)
    int fd; // only linux has fd
#endif // defined(YALX_OS_LINUX)
    
#if defined(YALX_OS_DARWIN)
    uintptr_t base;
#endif // defined(YALX_OS_DARWIN)
    int refs;
};

int memory_backing_init(struct memory_backing *backing, size_t capacity);
void memory_backing_final(struct memory_backing *backing);
void memory_backing_map(struct memory_backing *backing, uintptr_t addr, size_t size, uintptr_t offset);
void memory_backing_unmap(struct memory_backing *backing, uintptr_t addr, size_t size);


struct memory_segment {
    QUEUE_HEADER(struct memory_segment);
    uintptr_t addr;
    size_t    size;
};

struct physical_memory {
    size_t size_in_bytes;
    size_t n_segments;
    struct memory_segment segments;
};

struct physical_memory_management {
    size_t capacity;
    size_t unused_in_bytes;
    uintptr_t base_addr;
    uintptr_t limit_addr;
    struct memory_segment free;
    struct yalx_mutex mutex;
};

int physical_memory_management_init(struct physical_memory_management *self, uintptr_t base_addr, size_t capacity);

void physical_memory_management_final(struct physical_memory_management *self);

int allocate_physical_memory(struct physical_memory_management *self,
                             struct physical_memory *mem, size_t size);

void free_physical_memory(struct physical_memory_management *self, struct physical_memory *mem);

void debug_physical_memory_management(struct physical_memory_management *self);


struct virtual_memory_management {
    size_t capacity;
    size_t unused_in_bytes;
    struct memory_segment free;
    struct yalx_mutex mutex;
};

int virtual_memory_management_init(struct virtual_memory_management *self, size_t capacity);
void virtual_memory_management_final(struct virtual_memory_management *self);

linear_address_t allocate_virtual_memory(struct virtual_memory_management *self, size_t size, int force_from_end);
void free_virtual_memory(struct virtual_memory_management *self, linear_address_t addr);
void release_virtual_memory(struct virtual_memory_management *self, uintptr_t addr, size_t size);

struct ygc_page;

struct ygc_granule_map {
    size_t size;
    uintptr_t *bucket;
};

static inline void granule_map_init(struct ygc_granule_map *map) {
    map->size = 0;
    map->bucket = NULL;
}

static inline void granule_map_final(struct ygc_granule_map *map) {
    map->size = 0;
    free(map->bucket);
}

static inline uintptr_t granule_map_get(const struct ygc_granule_map *map, uintptr_t key) {
    size_t index = (size_t)(key >> YGC_GRANULE_SHIFT);
    return index >= map->size ? 0 : map->bucket[index];
}

static inline void granule_map_put(struct ygc_granule_map *map, uintptr_t key, uintptr_t value) {
    size_t index = (size_t)(key >> YGC_GRANULE_SHIFT);
    if (index >= map->size) {
        map->bucket = (uintptr_t *)realloc(map->bucket, (index + 1) * sizeof(uintptr_t));
        memset(map->bucket + map->size, 0, (index + 1 - map->size) * sizeof(uintptr_t));
        map->size = index + 1;
    }
    map->bucket[index] = value;
}

struct ygc_page {
    struct ygc_page *next;
    struct ygc_page *prev;
    uint32_t tick;
    linear_address_t virtual_addr;
    struct physical_memory physical_memory;
    _Atomic uintptr_t top;
    uintptr_t limit;
    struct ygc_live_map live_map;
};

struct ygc_core {
    struct memory_backing backing;
    struct physical_memory_management pmm;
    struct virtual_memory_management vmm;
    struct per_cpu_storage *small_page;
    struct ygc_page *_Atomic medium_page;
    _Atomic size_t rss;
    struct ygc_page pages;
    struct ygc_granule_map page_granules;

    struct ygc_mark mark;
    struct yalx_mutex mutex;
};


int ygc_init(struct ygc_core *ygc, size_t capacity);
void ygc_final(struct ygc_core *ygc);

struct ygc_core *ygc_heap_of(struct heap *h);

address_t ygc_allocate_object(struct ygc_core *ygc, size_t size, uintptr_t alignment_in_bytes);

struct ygc_page *ygc_page_new(struct ygc_core *ygc, size_t size);
void ygc_page_free(struct ygc_core *ygc, struct ygc_page *page);

// Allocate memory chunk from page
uintptr_t ygc_page_allocate(struct ygc_page *page, size_t size, uintptr_t alignment_in_bytes);
uintptr_t ygc_page_atomic_allocate(struct ygc_page *page, size_t size, uintptr_t alignment_in_bytes);

static inline int ygc_is_small_page(const struct ygc_page *page) {
    return page->virtual_addr.size == SMALL_PAGE_SIZE;
}

static inline int ygc_is_medium_page(const struct ygc_page *page) {
    return page->virtual_addr.size == MEDIUM_PAGE_SIZE;
}

static inline int ygc_is_large_page(const struct ygc_page *page) {
    return !ygc_is_small_page(page) && !ygc_is_medium_page(page);
}

static inline int ygc_page_is_allocating(const struct ygc_page *page) {
    return page->tick == ygc_global_tick;
}

static inline int ygc_page_is_relocatable(const struct ygc_page *page) {
    return page->tick < ygc_global_tick;
}

void ygc_page_mark_object(struct ygc_page *page, struct yalx_value_any *obj);

void ygc_page_visit_objects(struct ygc_page *page, struct yalx_heap_visitor *visitor);

static inline struct ygc_page *ygc_addr_in_page(const struct ygc_core *ygc, uintptr_t addr) {
    return (struct ygc_page *)granule_map_get(&ygc->page_granules, ygc_offset(addr));
}

static inline int ygc_addr_in_heap(const struct ygc_core *ygc, uintptr_t addr) {
    const struct ygc_page *page = ygc_addr_in_page(ygc, addr);
    const uintptr_t offset = ygc_offset(addr);
    return page != NULL && offset >= page->virtual_addr.addr && offset < page->limit;
}

static inline size_t ygc_page_used_in_bytes(const struct ygc_page *page) {
    return (size_t)page->top - page->virtual_addr.addr;
}

void ygc_mark_start(struct heap *h);
void ygc_mark(struct heap *h, int initial);

uintptr_t ygc_remap_object(struct ygc_core *ygc, uintptr_t addr);

static inline void ygc_mark_object(struct ygc_core *ygc, uintptr_t addr) {
    struct ygc_page *page = ygc_addr_in_page(ygc, addr);
    ygc_page_mark_object(page, (struct yalx_value_any *)addr);
    ygc_marking_mark_object(&ygc->mark, addr);
}

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_HEAP_YGC_H
