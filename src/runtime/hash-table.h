#pragma once
#ifndef YALX_RUNTIME_HASH_TABLE_H_
#define YALX_RUNTIME_HASH_TABLE_H_

#include "runtime/runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

struct hash_table_slot {
    struct hash_table_slot *prev;
    struct hash_table_slot *next;
    uint32_t hash_code;
    int key_size;
    int value_size;
    char key[1];
};

typedef struct hash_table_value_span {
    void   *value;
    size_t  size;
} hash_table_value_span_t;

struct hash_table {
    int size;     // size of key-pairs
    int capacity_shift; // capacity of slots
    float rehash_factor;
    struct hash_table_slot *slots;
};

void yalx_init_hash_table(struct hash_table *map, float rehash_factor);
void yalx_free_hash_table(struct hash_table *map);

#define yalx_put_string_key(map, key, value_size) \
    yalx_hash_table_put(map, key, strlen(key), value_size)
#define yalx_get_string_key(map, key) \
    yalx_hash_table_get(map, key, strlen(key))
#define yalx_remove_string_key(map, key) \
    yalx_hash_table_remove(map, key, strlen(key))

hash_table_value_span_t yalx_hash_table_put(struct hash_table *map, const void *key, size_t key_size, size_t value_size);
hash_table_value_span_t yalx_hash_table_get(struct hash_table *map, const void *key, size_t len);
void yalx_hash_table_remove(struct hash_table *map, const void *key, size_t len);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_HASH_TABLE_H_
