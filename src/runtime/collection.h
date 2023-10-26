#pragma once
#ifndef YALX_RUNTIME_COLLECTION_H
#define YALX_RUNTIME_COLLECTION_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct yalx_array_header {
    size_t capacity;
    uint32_t len;
    uint32_t elem_size_in_bytes;
};

#define YALX_ARRAY_TY(type) type *
#define YALX_ARRAY(len, type) ((type *)yalx_new_array((len), sizeof(type)))
#define YALX_ARRAY_ADD(body, elem, type) *((type *)yalx_array_add_location(&(body))) = elem

void *yalx_new_array(size_t len, size_t elem_size_in_bytes);
void yalx_free_array(void *body);

static inline struct yalx_array_header const *yalx_array_head(const void *body) {
    return ((struct yalx_array_header const *)body) - 1;
}

static inline size_t yalx_array_len(const void *body) {
    return yalx_array_head(body)->len;
}

static inline size_t yalx_array_capacity(const void *body) {
    return yalx_array_head(body)->capacity;
}

void *yalx_array_add_location(void **addr);
void *yalx_array_extend_room(void *body, size_t count_of_extended);

#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_COLLECTION_H
