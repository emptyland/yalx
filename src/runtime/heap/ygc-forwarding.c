#include "runtime/heap/ygc-forwarding.h"
#include "runtime/heap/ygc.h"
#include "runtime/checking.h"

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
    struct forwarding *fwd = MALLOC(struct forwarding);
    fwd->page = page;
    fwd->virtual_addr.addr = page->virtual_addr.addr;
    fwd->virtual_addr.size = page->virtual_addr.size;
    fwd->n_entries = round_up_power_of_2(page->live_map.live_objs * 2);
    fwd->entries = MALLOC_N(_Atomic struct forwarding_entry, fwd->n_entries);
    memset(fwd->entries, 0, sizeof(struct forwarding_entry) * fwd->n_entries);
    fwd->refs = 1;
    fwd->pinned = 0;
    return fwd;
}

void forwarding_free(struct forwarding *fwd) {
    free(fwd->entries);
    free(fwd);
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