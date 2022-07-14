#include "runtime/object/yalx-string.h"
#include "runtime/object/type.h"
#include "runtime/heap/heap.h"
#include "runtime/checking.h"

static const char digit_table[] = "0123456789abcdefghijklmnopqrstuvwxyz";

struct yalx_value_str *yalx_uint_to_string(struct heap *heap, uint64_t value, int base) {
    DCHECK(base > 1);
    DCHECK(base <= 32);
    
    char buf[128] = {0};
    char *z = buf + sizeof(buf) - 1;
    *z-- = '\0';
    do {
        *z-- = digit_table[value % base];
        value /= base;
    } while (value);
    
    
    return yalx_new_string(heap, z + 1, (buf + sizeof(buf) - 1) - z);
}

struct yalx_value_str *yalx_int_to_string(struct heap *heap, int64_t value, int base) {
    DCHECK(base > 1);
    DCHECK(base <= 32);
    
    int sign = value < 0;
    if (sign) {
        value = -value;
    }
    
    char buf[128] = {0};
    char *z = buf + sizeof(buf) - 1;
    *z-- = '\0';
    do {
        *z-- = digit_table[value % base];
        value /= base;
    } while (value);
    if (sign) {
        *z-- = '-';
    }
    
    return yalx_new_string(heap, z + 1, (buf + sizeof(buf) - 1) - z);
}

struct yalx_value_str *yalx_new_string(struct heap *heap, const char *z, size_t n) {
    struct string_pool_entry *space = yalx_ensure_space_kpool(heap, z, n);
    if (space && space->value) {
        return space->value;
    }
    struct yalx_value_str *str = yalx_new_string_direct(heap, z, n);
    if (space) {
        space->value = str;
    }
    return str;
}

struct yalx_value_str *yalx_new_string_direct(struct heap *heap, const char *z, size_t n) {
    size_t size = yalx_reserve_string_bytes(z, n);
    struct allocate_result result = yalx_heap_allocate(heap, string_class, size, 0);
    if (result.status != ALLOCATE_OK) {
        return NULL;
    }
    struct yalx_value_str *str = (struct yalx_value_str *)result.object;
    str->hash_code = yalx_str_hash(z, n);
    str->len = (u32_t)n;
    memcpy(str->bytes, z, n);
    str->bytes[n] = '\0';
    return str;
}

