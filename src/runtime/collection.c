#include "runtime/collection.h"
#include "runtime/runtime.h"
#include "runtime/checking.h"

void *yalx_new_array(size_t len, size_t elem_size_in_bytes) {
    DCHECK(elem_size_in_bytes > 0);

    size_t capacity_shift = yalx_log2(len);
    if (capacity_shift < 3) {
        capacity_shift = 3;
    }
    while (((size_t)1u << capacity_shift) < len) {
        capacity_shift++;
    }

    const size_t capacity = (size_t)1u << capacity_shift;
    DCHECK(capacity >= len);
    const size_t total_in_bytes = sizeof(struct yalx_array_header) + capacity * elem_size_in_bytes;

    struct yalx_array_header *header = (struct yalx_array_header *) malloc(total_in_bytes);
    dbg_init_zag(header + 1, capacity * elem_size_in_bytes);
    header->capacity = capacity;
    header->len = (uint32_t)len;
    header->elem_size_in_bytes = (uint32_t)elem_size_in_bytes;

    DLOG(INFO, "header: %p", header);
    return (void *)(header + 1);
}

void yalx_free_array(void *body) {
    if (body) {
        free(((struct yalx_array_header *)body) - 1);
    }
}

void *yalx_array_add_location(void **addr) {
    address_t body = (address_t)yalx_array_extend_room(*addr, 1);
    struct yalx_array_header *header = ((struct yalx_array_header *)body) - 1;
    header->len++;
    DCHECK(header->len <= header->capacity);
    *addr = (void *)body;

    return body + (header->len - 1) * header->elem_size_in_bytes;
}

void *yalx_array_extend_room(void *addr, size_t count_of_extended) {
    struct yalx_array_header *header = ((struct yalx_array_header *)addr) - 1;
    if (header->len + count_of_extended > header->capacity) {
        header->capacity <<= 1;

        const size_t total_in_bytes = sizeof(struct yalx_array_header) + header->capacity * header->elem_size_in_bytes;
        header = realloc(header, total_in_bytes);
        return (void *)(header - 1);
    }
    return addr;
}