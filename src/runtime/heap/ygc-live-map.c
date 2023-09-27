#include "runtime/heap/ygc-live-map.h"
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

    bucket->bits[index >> pointer_shift_in_bits] |= (1U << (index & pointer_mask_in_bits));
}

void live_map_set(struct ygc_live_map *map, int index) {
    DCHECK(index >= 0);
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
    return (bucket->bits[index >> pointer_shift_in_bits] & (1U << (index & pointer_mask_in_bits))) != 0;
}

int live_map_get(struct ygc_live_map *map, int index) {
    DCHECK(index >= 0);
    const size_t bucket = (size_t)index >> map->shift_per_bucket;
    if (bucket >= map->bucket_size) {
        return 0;
    }
    return bucket_get(map, &map->buckets[bucket], index & map->mask_per_bucket);
}

void live_map_increase_obj(struct ygc_live_map *map, struct yalx_value_any *obj) {
    atomic_fetch_add(&map->live_objs, 1);
    atomic_fetch_add(&map->live_objs_in_bytes, yalx_object_size_in_bytes(obj));
}