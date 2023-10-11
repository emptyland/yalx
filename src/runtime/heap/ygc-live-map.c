#include "runtime/heap/ygc-live-map.h"
#include "runtime/heap/ygc.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/object/any.h"
#include "runtime/locks.h"
#include "runtime/checking.h"
#include "runtime/runtime.h"
#include <stdlib.h>

struct ygc_live_bucket {
    size_t item_size;
    uintptr_t *bits;
};

void live_map_init(struct ygc_live_map *map, size_t shift_per_bucket) {
    map->live_objs = 0;
    map->live_objs_in_bytes = 0;
    map->bucket_size = 0;
    map->shift_per_bucket = shift_per_bucket;
    map->mask_per_bucket = (1 << map->shift_per_bucket) - 1;
    map->buckets = NULL;
}

void live_map_final(struct ygc_live_map *map) {
    for (size_t i = 0; i < map->bucket_size; i++) {
        free(map->buckets[i].bits);
    }
    free(map->buckets);
    memset(map, 0, sizeof(*map));
}

static void bucket_set(struct ygc_live_map *map, struct ygc_live_bucket *bucket, int index) {
    DCHECK(sizeof(uintptr_t) == pointer_size_in_bytes);

    if (!bucket->bits) {
        const size_t bits_per_bucket = (1U << map->shift_per_bucket);
        const size_t segments_size = (bits_per_bucket + pointer_size_in_bits - 1) >> pointer_shift_in_bits;

        bucket->bits = (uintptr_t *)yalx_zalloc(sizeof(uintptr_t) * segments_size);
        bucket->item_size = segments_size;
    }

    bucket->bits[index >> pointer_shift_in_bits] |= ((uintptr_t)1 << (index & pointer_mask_in_bits));
}

void live_map_set(struct ygc_live_map *map, int index) {
    DCHECK(index >= 0);
    //DLOG(INFO, "set index=%d", index);
    const size_t bucket = (size_t)index >> map->shift_per_bucket;
    if (bucket >= map->bucket_size) {
        map->buckets = (struct ygc_live_bucket *) realloc(map->buckets, (bucket + 1) * sizeof(struct ygc_live_bucket));
        memset(map->buckets + map->bucket_size, 0, (bucket + 1 - map->bucket_size) * sizeof(struct ygc_live_bucket));
        map->bucket_size = bucket + 1;
    }
    bucket_set(map, &map->buckets[bucket], index & map->mask_per_bucket);
}

static int bucket_get(struct ygc_live_map *map, struct ygc_live_bucket *bucket, int index) {
    DCHECK(sizeof(uintptr_t) == pointer_size_in_bytes);
    if (!bucket->bits) {
        return 0;
    }
    return (bucket->bits[index >> pointer_shift_in_bits] & ((uintptr_t)1 << (index & pointer_mask_in_bits))) != 0;
}

int live_map_get(struct ygc_live_map *map, int index) {
    DCHECK(index >= 0);
    const size_t bucket = (size_t)index >> map->shift_per_bucket;
    if (bucket >= map->bucket_size) {
        return 0;
    }
    return bucket_get(map, &map->buckets[bucket], index & map->mask_per_bucket);
}

void live_map_increase_obj(struct ygc_live_map *map, size_t objs, size_t objs_in_bytes) {
    atomic_fetch_add(&map->live_objs, objs);
    atomic_fetch_add(&map->live_objs_in_bytes, objs_in_bytes);
}

static
void visit_objects_in_bucket(size_t base, struct ygc_live_bucket *bucket, struct ygc_page *page,
                             struct yalx_heap_visitor *visitor) {
    for (int i = 0; i < bucket->item_size; i++) {
        if (!bucket->bits[i]) {
            continue;
        }
        for (int j = 0; j < pointer_size_in_bits; j++) {
            const uintptr_t mask = ((uintptr_t)1) << j;
            if (bucket->bits[i] & mask) {
                size_t index = base + (i << pointer_shift_in_bits) + j;
                uintptr_t offset = page->virtual_addr.addr + (index << pointer_shift_in_bytes);
                yalx_ref_t obj = (yalx_ref_t)ygc_good_address(offset);
                //DLOG(INFO, "obj=%p, index=%zd, bits=%p, test=%x", obj, index, bucket->bits[i], mask);
                visitor->visit_pointer(visitor, obj);
            }
        }
    }
}

void live_map_visit_objects(struct ygc_live_map *map, struct ygc_page *page, struct yalx_heap_visitor *visitor) {
    for (int i = 0; i < map->bucket_size; i++) {
        if (!map->buckets[i].bits) {
            continue;
        }
        struct ygc_live_bucket *bucket = &map->buckets[i];
        visit_objects_in_bucket(i << map->shift_per_bucket, bucket, page, visitor);
    }
}