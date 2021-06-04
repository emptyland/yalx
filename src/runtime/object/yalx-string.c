#include "runtime/object/yalx-string.h"
#include "runtime/object/type.h"
#include "runtime/heap/heap.h"


struct yalx_value_str *yalx_new_string(struct heap *heap, const char *z, size_t n) {
    struct yalx_value_str **space = yalx_ensure_space_kpool(heap, z, n);
    if (space && *space) {
        return *space;
    }
    struct yalx_value_str *str = yalx_new_string_direct(heap, z, n);
    if (space) {
        *space = str;
    }
    return str;
}

struct yalx_value_str *yalx_new_string_direct(struct heap *heap, const char *z, size_t n) {
    size_t size = yalx_reserve_string_size(z, n);
    struct allocate_result result = yalx_heap_allocate(heap, &string_class, size, 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    struct yalx_value_str *str = (struct yalx_value_str *)result.object;
    str->hash_code = yalx_str_hash(z, n);
    str->len = n;
    memcpy(str->bytes, z, n);
    str->bytes[n] = '\0';
    return str;
}
