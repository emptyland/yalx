#pragma once
#ifndef YALX_RUNTIME_HEAP_YGC_LIVE_MAP_H
#define YALX_RUNTIME_HEAP_YGC_LIVE_MAP_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ygc_live_bucket;
struct yalx_value_any;

struct ygc_live_map {
    _Atomic size_t live_objs;
    _Atomic size_t live_objs_in_bytes;
    struct ygc_live_bucket *buckets;
    size_t shift_per_bucket; // bits per bucket;
    int mask_per_bucket;
    size_t bucket_size;
};

void live_map_init(struct ygc_live_map *map, size_t shift_per_bucket);
void live_map_final(struct ygc_live_map *map);

void live_map_set(struct ygc_live_map *map, int index);
int live_map_get(struct ygc_live_map *map, int index);

void live_map_increase_obj(struct ygc_live_map *map, struct yalx_value_any *obj);

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_HEAP_YGC_LIVE_MAP_H
