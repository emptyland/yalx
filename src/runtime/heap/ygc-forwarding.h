#pragma once
#ifndef YALX_YGC_RUNTIME_HEAP_FORWARDING_H
#define YALX_YGC_RUNTIME_HEAP_FORWARDING_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

struct ygc_page;
struct ygc_granule_map;

//
// Forwarding entry layout
// -----------------------
//
//   6                  4 4
//   3                  6 5                                                1 0
//  +--------------------+--------------------------------------------------+-+
//  |11111111 11111111 11|111111 11111111 11111111 11111111 11111111 1111111|1|
//  +--------------------+--------------------------------------------------+-+
//  |                    |                                                  |
//  |                    |                      0-0 Populated Flag (1-bits) *
//  |                    |
//  |                    * 45-1 To Object Offset (45-bits)
//  |
//  * 63-46 From Object Index (18-bits)
//
struct forwarding_entry {
    uint64_t populated: 1;
    uint64_t to_offset: 45;
    uint64_t from_index: 18;
};

// static_assert(sizeof(struct forwarding_entry) == sizeof(uint64_t), "forwarding_entry too big");

struct forwarding {
    struct {
        uintptr_t addr;
        size_t size;
    } virtual_addr;
    _Atomic struct forwarding_entry *entries;
    size_t n_entries;
    struct ygc_page *page;
    _Atomic int refs;
    _Atomic int pinned;
};

struct forwarding *forwarding_new(struct ygc_page *page);
void forwarding_free(struct forwarding *fwd);

uintptr_t forwarding_insert(struct forwarding *fwd, uintptr_t from_index, uintptr_t to_offset, size_t *pos);
struct forwarding_entry forwarding_find(struct forwarding *fwd, uintptr_t from_index, size_t *pos);
struct forwarding_entry forwarding_first(struct forwarding *fwd, uintptr_t from_index, size_t *pos);

static inline
struct forwarding_entry forwarding_next(struct forwarding *fwd, size_t *pos) {
    const size_t mask = fwd->n_entries - 1;
    *pos = (*pos + 1) & mask;
    return atomic_load_explicit(fwd->entries + *pos, memory_order_acquire);
}

void forwarding_table_insert(struct ygc_granule_map *map, struct forwarding *fwd);
void forwarding_table_remove(struct ygc_granule_map *map, struct forwarding *fwd);

#endif //YALX_YGC_RUNTIME_HEAP_FORWARDING_H
