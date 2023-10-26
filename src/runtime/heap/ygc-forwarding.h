#pragma once
#ifndef YALX_YGC_RUNTIME_HEAP_FORWARDING_H
#define YALX_YGC_RUNTIME_HEAP_FORWARDING_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ygc_core;
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
    volatile _Atomic struct forwarding_entry *entries;
    size_t n_entries;
    struct ygc_page *page;
    volatile _Atomic int refs;
    volatile _Atomic int pinned;
};


struct forwarding *forwarding_new(struct ygc_page *page);
void forwarding_free(struct forwarding *fwd);

static inline int forwarding_contains_addr(struct forwarding const *fwd, uintptr_t offset) {
    return offset >= fwd->virtual_addr.addr && offset < fwd->virtual_addr.addr + fwd->virtual_addr.size;
}

int forwarding_grab_page(struct forwarding *fwd);
int forwarding_drop_page(struct forwarding *fwd, struct ygc_core *ygc);
void forwarding_set_pinned(struct forwarding *fwd, int pinned);

uintptr_t forwarding_insert(struct forwarding *fwd, uintptr_t from_index, uintptr_t to_offset, size_t *pos);
struct forwarding_entry forwarding_find(struct forwarding *fwd, uintptr_t from_index, size_t *pos);
struct forwarding_entry forwarding_first(struct forwarding *fwd, uintptr_t from_index, size_t *pos);

struct forwarding_entry forwarding_next(struct forwarding *fwd, size_t *pos);

void forwarding_table_insert(struct ygc_granule_map *map, struct forwarding *fwd);
void forwarding_table_remove(struct ygc_granule_map *map, struct forwarding *fwd);
struct forwarding *forwarding_table_get(struct ygc_granule_map *map, uintptr_t addr);


#ifdef __cplusplus
}
#endif

#endif //YALX_YGC_RUNTIME_HEAP_FORWARDING_H
