#include "runtime/heap/ygc-forwarding.h"
#include "runtime/heap/ygc.h"
#include "runtime/checking.h"
#include <stdatomic.h>

static int count_leading_zeros_32(uint32_t x) {
    static const int zval[16] = {
            4, /* 0 */ 3, /* 1 */ 2, /* 2 */ 2, /* 3 */
            1, /* 4 */ 1, /* 5 */ 1, /* 6 */ 1, /* 7 */
            0, /* 8 */ 0, /* 9 */ 0, /* A */ 0, /* B */
            0, /* C */ 0, /* D */ 0, /* E */ 0  /* F */
    };

    int base = 0;
    if ((x & 0xFFFF0000) == 0) {base  = 16; x <<= 16;} else {base = 0;}
    if ((x & 0xFF000000) == 0) {base +=  8; x <<=  8;}
    if ((x & 0xF0000000) == 0) {base +=  4; x <<=  4;}
    return base + zval[x >> (32-4)];
}

static inline int count_leading_zeros_64(uint64_t x) {
    if ((x & 0xFFFFFFFF00000000ULL) == 0) {
        return 32 + count_leading_zeros_32((uint32_t)x);
    } else {
        x = (x & 0xFFFFFFFF00000000ULL) >> 32;
        return count_leading_zeros_32((uint32_t)x);
    }
}

static size_t round_up_power_of_2(size_t input) {
    DCHECK(input != 0);
    if (yalx_u64_is_power_of_2(input)) {
        return input;
    }
    uint32_t lz = count_leading_zeros_64(input);
    DCHECK(lz < sizeof(input) * 8);
    DCHECK(lz > 0);

    return ((size_t)1) << (sizeof(input) * 8 - lz);
}

static inline uint32_t hash_uint32_to_uint32(uint32_t key) {
    key = ~key + (key << 15);
    key = key ^ (key >> 12);
    key = key + (key << 2);
    key = key ^ (key >> 4);
    key = key * 2057;
    key = key ^ (key >> 16);
    return key;
}

//static inline uint32_t hash_address_to_uint32(uintptr_t key) {
//    return hash_uint32_to_uint32((uint32_t)(key >> 3));
//}

struct forwarding *forwarding_new(struct ygc_page *page) {
    DCHECK(sizeof(struct forwarding_entry) == sizeof(uint64_t) && "struct forwarding_entry too small");

    struct forwarding *fwd = MALLOC(struct forwarding);
    fwd->page = page;
    fwd->virtual_addr.addr = page->virtual_addr.addr;
    fwd->virtual_addr.size = page->virtual_addr.size;
    fwd->n_entries = round_up_power_of_2(page->live_map.live_objs * 2);
    fwd->entries = MALLOC_N(_Atomic struct forwarding_entry, fwd->n_entries);
    memset((void *)fwd->entries, 0, sizeof(struct forwarding_entry) * fwd->n_entries);
    fwd->refs = 1;
    fwd->pinned = 0;
    return fwd;
}

void forwarding_free(struct forwarding *fwd) {
    free((void *)fwd->entries);
    free(fwd);
}

int forwarding_grab_page(struct forwarding *fwd) {
    int refs = atomic_load(&fwd->refs);
    while (refs > 0) {
        if (atomic_compare_exchange_strong(&fwd->refs, &refs, refs + 1)) {
            return 1;
        }
    }
    return 0;
}

int forwarding_drop_page(struct forwarding *fwd, struct ygc_core *ygc) {
    DCHECK(fwd->refs > 0);
    if (atomic_fetch_sub(&fwd->refs, 1) == 1) {
        struct ygc_page *const page = fwd->page;
        DLOG(INFO, "<Free Page> Cause forwarding drop page: [%p, %p) (%zd)",
             page->virtual_addr.addr,
             page->virtual_addr.addr + page->virtual_addr.size,
             page->virtual_addr.size);
        ygc_page_free(ygc, page, 1);
        fwd->page = NULL;
        return 1;
    }
    return 0;
}

void forwarding_set_pinned(struct forwarding *fwd, int pinned) {
    atomic_store(&fwd->pinned, pinned);
}

void forwarding_verify(struct forwarding const *fwd) {
    GUARANTEE(fwd->refs > 0, "Invalid refs count: %d", fwd->refs);
    GUARANTEE(fwd->page != NULL, "Invalid page");

    size_t live_objs_count = 0;
    for (size_t i = 0; i < fwd->n_entries; i++) {
        struct forwarding_entry const entry = atomic_load(&fwd->entries[i]);
        if (!entry.populated) {
            continue;
        }

        // Check from index:
        GUARANTEE(entry.from_index < ygc_page_object_max_count(fwd->page), "Invalid from index: %zd", entry.from_index);

        // Check duplicates:
        for (size_t j = i + 1; j < fwd->n_entries; j++) {
            struct forwarding_entry const other = atomic_load(&fwd->entries[j]);
            if (!other.populated) {
                continue;
            }

            GUARANTEE(entry.from_index != other.from_index, "Duplicated from index: %p", entry.from_index);
            GUARANTEE(entry.to_offset != other.to_offset, "Duplicated to offset: %p", entry.to_offset);
        }

        live_objs_count++;
    }

    GUARANTEE(live_objs_count == fwd->page->live_map.live_objs, "Invalid number of entries, expected: %zd; actual is %zd",
              fwd->page->live_map.live_objs, live_objs_count);
}

uintptr_t forwarding_insert(struct forwarding *fwd, uintptr_t from_index, uintptr_t to_offset, size_t *pos) {
    struct forwarding_entry new_entry = {1, to_offset, from_index};
    struct forwarding_entry old_entry = { 0, 0, 0};

    for(;;) {
        struct forwarding_entry prev_entry = old_entry;
        if (atomic_compare_exchange_strong(fwd->entries + *pos, &prev_entry, new_entry)) {
            return to_offset;
        }
        // Find next empty or matching entry
        struct forwarding_entry entry = atomic_load_explicit(fwd->entries + *pos, memory_order_acquire);
        while (entry.populated) {
            if (entry.from_index == from_index) {
                // Match found, return already inserted address
                return entry.to_offset;
            }
            entry = forwarding_next(fwd, pos);
        }
    }
}

struct forwarding_entry forwarding_find(struct forwarding *fwd, uintptr_t from_index, size_t *pos) {
    // Reading entries in the table races with the atomic CAS done for
    // insertion into the table. This is safe because each entry is at
    // most updated once (from zero to something else).
    struct forwarding_entry entry = forwarding_first(fwd, from_index, pos);
    while (entry.populated) {
        if (entry.from_index == from_index) {
            // Match found, return already inserted address
            return entry;
        }
        entry = forwarding_next(fwd, pos);
    }
    // Match not found, return empty entry
    return entry;
}

struct forwarding_entry forwarding_first(struct forwarding *fwd, uintptr_t from_index, size_t *pos) {
    const size_t mask = fwd->n_entries - 1;
    const size_t hash = hash_uint32_to_uint32((uint32_t)from_index);
    *pos = hash & mask;
    return atomic_load_explicit(fwd->entries + *pos, memory_order_acquire);
}

struct forwarding_entry forwarding_next(struct forwarding *fwd, size_t *pos) {
    const size_t mask = fwd->n_entries - 1;
    *pos = (*pos + 1) & mask;
    return atomic_load_explicit(fwd->entries + *pos, memory_order_acquire);
}

void forwarding_table_insert(struct ygc_granule_map *map, struct forwarding *fwd) {
    granule_map_put(map, fwd->virtual_addr.addr, (uintptr_t)fwd);
}

void forwarding_table_remove(struct ygc_granule_map *map, struct forwarding *fwd) {
    granule_map_put(map, fwd->virtual_addr.addr, 0);
    //forwarding_free(fwd);
}

struct forwarding *forwarding_table_get(struct ygc_granule_map *map, uintptr_t addr) {
    return (struct forwarding *)granule_map_get(map, ygc_offset(addr));
}
