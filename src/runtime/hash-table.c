#include "runtime/hash-table.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define hash_table_capacity(map) (1 << (map)->capacity_shift)
//#define hash_table_slot_index(map, hash_code) ((hash_code) & ((1 << ((map)->capacity_shift)) - 1))
//#define hash_table_slot_at(map, hash_code) (&(map)->slots[hash_table_slot_index(map, hash_code)])

static const hash_table_value_span_t hash_table_value = {NULL, 0};

static inline size_t hash_table_key_placement_size(size_t key_size, size_t value_size) {
    size_t header_size = sizeof(struct hash_table_slot) + key_size;
    header_size = ROUND_UP(header_size, 4);
    return header_size + value_size;
}

static inline hash_table_value_span_t hash_table_key_value(struct hash_table_slot *node) {
    hash_table_value_span_t span;
    span.value = &node->key[(ROUND_UP(node->key_size, 4))];
    span.size  = node->value_size;
    return span;
}

static uint32_t js_hash(const char *p, size_t n) {
    int hash = 1315423911;
    for (const char *s = p; s < p + n; s++) {
        hash ^= ((hash << 5) + (*s) + (hash >> 2));
    }
    return hash;
}

static inline struct hash_table_slot *new_hash_table_key(const void *key,
                                                         size_t key_size,
                                                         size_t value_size) {
    size_t size_in_bytes = hash_table_key_placement_size(key_size, value_size);
    struct hash_table_slot *slot = malloc(size_in_bytes);
    if (!slot) {
        return NULL;
    }
    dbg_init_zag(slot, size_in_bytes);
    slot->next = slot;
    slot->prev = slot;
    slot->hash_code = js_hash(key, key_size);
    slot->key_size = key_size;
    slot->value_size = value_size;
    memcpy(slot->key, key, key_size);
    return slot;
}

static inline struct hash_table_slot *hash_table_slot_at_hash_code(struct hash_table *map, uint32_t hash_code) {
    const size_t index = hash_code & ((1 << (map->capacity_shift)) - 1);
    return &map->slots[index];
}

static inline struct hash_table_slot *hash_table_slot_at(struct hash_table *map, void *key, size_t len) {
    return hash_table_slot_at_hash_code(map, js_hash(key, len));
}

static void rehash_if_needed(struct hash_table *map) {
    const float rate = ((float)map->size) / ((float)hash_table_capacity(map));
    if (rate <= map->rehash_factor) {
        return;
    }
    struct hash_table_slot **linear_nodes = (struct hash_table_slot **)malloc(map->size * sizeof(struct hash_table_slot *));
    int k = 0;
    for (int j = 0; j < hash_table_capacity(map); j++){
        struct hash_table_slot *slot = &map->slots[j];
        while (!QUEUE_EMPTY(slot)) {
            struct hash_table_slot *node = slot->next;
            linear_nodes[k++] = node;
            QUEUE_REMOVE(node);
            //free(node);
        }
    }
    map->capacity_shift++;
    map->slots = (struct hash_table_slot *)realloc(map->slots,
                                                   hash_table_capacity(map) * sizeof(struct hash_table_slot));
    for (int j = 0; j < hash_table_capacity(map); j++) {
        struct hash_table_slot *slot = &map->slots[j];
        slot->next = slot;
        slot->prev = slot;
    }
    for (int i = 0; i < k; i++) {
        struct hash_table_slot *node = linear_nodes[i];
        struct hash_table_slot *slot = hash_table_slot_at_hash_code(map, node->hash_code);
        QUEUE_INSERT_TAIL(slot, node);
    }
    free(linear_nodes);
}

void yalx_init_hash_table(struct hash_table *map, float rehash_factor) {
    //assert(rehash_factor > 0);
    map->size = 0;
    map->capacity_shift = 2;
    map->rehash_factor = rehash_factor < 1.1 ? 1.1 : rehash_factor;
    map->slots = (struct hash_table_slot *)malloc(hash_table_capacity(map) * sizeof(struct hash_table_slot));
    if (map->slots) {
        for (int i = 0; i < hash_table_capacity(map); i++) {
            struct hash_table_slot *slot = &map->slots[i];
            slot->prev = slot;
            slot->next = slot;
            slot->key_size   = 0;
            slot->value_size = 0;
        }
    }
}

void yalx_free_hash_table(struct hash_table *map) {
    for (int i = 0; i < hash_table_capacity(map); i++) {
        struct hash_table_slot *slot = &map->slots[i];
        while (slot->next != slot) {
            struct hash_table_slot *key = slot->next;
            QUEUE_REMOVE(key);
            free(key);
        }
    }
    free(map->slots);
    map->slots = NULL;
}

hash_table_value_span_t yalx_hash_table_put(struct hash_table *map, const void *key, size_t key_size, size_t value_size) {
    struct hash_table_slot *const slot = hash_table_slot_at(map, key, key_size);
    for (struct hash_table_slot *node = slot->next; node != slot; node = node->next) {
        if (node->key_size == key_size && memcmp(node->key, key, key_size) == 0) {
            if (node->value_size >= value_size) {
                return hash_table_key_value(node);
            }
            QUEUE_REMOVE(node);
            node = realloc(node, hash_table_key_placement_size(key_size, value_size));
            QUEUE_INSERT_HEAD(slot, node);
            return hash_table_key_value(node);
        }
    }
    struct hash_table_slot *node = new_hash_table_key(key, key_size, value_size);
    QUEUE_INSERT_HEAD(slot, node);
    map->size++;
    rehash_if_needed(map);
    return hash_table_key_value(node);
}

hash_table_value_span_t yalx_hash_table_get(struct hash_table *map, const void *key, size_t len) {
    struct hash_table_slot *const slot = hash_table_slot_at(map, key, len);
    for (struct hash_table_slot *node = slot->next; node != slot; node = node->next) {
        if (node->key_size == len && memcmp(node->key, key, len) == 0) {
            return hash_table_key_value(node);
        }
    }
    return hash_table_value;
}

void yalx_hash_table_remove(struct hash_table *map, const void *key, size_t len) {
    struct hash_table_slot *const slot = hash_table_slot_at(map, key, len);
    for (struct hash_table_slot *node = slot->next; node != slot; node = node->next) {
        if (node->key_size == len && memcmp(node->key, key, len) == 0) {
            QUEUE_REMOVE(node);
            free(node);
            map->size--;
            break;
        }
    }
}
