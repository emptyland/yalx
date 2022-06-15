#include "runtime/heap/heap.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/number.h"
#include "runtime/object/any.h"
#include "runtime/object/type.h"
#include "runtime/checking.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


struct heap heap;

static struct allocate_result allocate_from_pool(struct heap *heap, size_t size, u32_t flags) {
    USE(flags);
    struct one_time_memory_pool *pool = &heap->one_time_pool;
    
    DCHECK(pool->free >= pool->chunk);
    size_t used_in_bytes = pool->free - pool->chunk;
    DCHECK(used_in_bytes <= pool->size);
    size_t free_in_bytes = pool->size - used_in_bytes;
    size_t n = ROUND_UP(size, 4);
    
    struct allocate_result rv = {NULL, ALLOCATE_NOTHING};
    if (n > free_in_bytes) {
        rv.status = ALLOCATE_NOT_ENOUGH_MEMORY;
        return rv;
    }
    rv.object = (yalx_ref_t)pool->free;
    rv.status = ALLOCATE_OK;
    pool->free += n;
    dbg_init_zag(rv.object, n);
    return rv;
}

static void finalize_for_pool(struct heap *heap) {
    dbg_free_zag(heap->one_time_pool.chunk, heap->one_time_pool.size);
    free(heap->one_time_pool.chunk);
}

static struct allocate_result allocate_from_lxr(struct heap *heap, size_t size, u32_t flags) {
    USE(flags);
    struct allocate_result rs = {NULL, ALLOCATE_NOTHING};
    void *chunk = lxr_allocate(&heap->lxr_immix, size);
    if (!chunk) {
        rs.status = ALLOCATE_NO_OS_MEMORY;
        return rs;
    }
    rs.object = (struct yalx_value_any *)chunk;
    rs.status = ALLOCATE_OK;
    return rs;
}

static void finalize_for_lxr(struct heap *heap) {
    lxr_free_immix_heap(&heap->lxr_immix);
}

static f32_t fast_boxing_f32_table[] = {
    -1.0,
    0,
    1.0,
};

static f64_t fast_boxing_f64_table[] = {
    -1.0,
    0,
    1.0,
};

static int boxing_number_pool_init(struct heap *heap, struct boxing_number_pool *pool) {
    pool->bool_values[0] = yalx_new_small_boxing_number(heap, Bool_class);
    pool->bool_values[1] = yalx_new_small_boxing_number(heap, Bool_class);
    pool->bool_values[0]->box.u32 = 0;
    pool->bool_values[1]->box.u32 = 1;
    
    for (int i = 0; i < 256; i++) {
        pool->u8_values[i] = yalx_new_small_boxing_number(heap, U8_class);
        pool->u8_values[i]->box.u32 = i;
        
        pool->i8_values[i] = yalx_new_small_boxing_number(heap, I8_class);
        pool->i8_values[i]->box.i8 = ((int8_t)(i - 128));
    }
    for (int i = 0; i < 201; i++) {
        pool->u16_values[i] = yalx_new_small_boxing_number(heap, U16_class);
        pool->u16_values[i]->box.u32 = i;
        
        pool->i16_values[i] = yalx_new_small_boxing_number(heap, I16_class);
        pool->i16_values[i]->box.i16 = ((int16_t)(i - 100));
        
        pool->u32_values[i] = yalx_new_small_boxing_number(heap, U32_class);
        pool->u32_values[i]->box.u32 = i;
        
        pool->i32_values[i] = yalx_new_small_boxing_number(heap, I32_class);
        pool->i32_values[i]->box.i32 = i - 100;
        
        pool->u64_values[i] = yalx_new_small_boxing_number(heap, U64_class);
        pool->u64_values[i]->box.u64 = i;
        
        pool->i64_values[i] = yalx_new_small_boxing_number(heap, I64_class);
        pool->i64_values[i]->box.i64 = i - 100;
    }
    
    for (int i = 0; i < arraysize(fast_boxing_f32_table); i++) {
        pool->f32_values[i] = yalx_new_small_boxing_number(heap, F32_class);
        pool->f32_values[i]->box.f32 = fast_boxing_f32_table[i];
    }
    for (int i = 0; i < arraysize(fast_boxing_f64_table); i++) {
        pool->f64_values[i] = yalx_new_small_boxing_number(heap, F64_class);
        pool->f64_values[i]->box.f64 = fast_boxing_f64_table[i];
    }
    return 0;
}

void string_pool_init(struct string_pool *pool, int slots_shift) {
    DCHECK(slots_shift >= 4 && "shift too small");
    
    pool->n_entries = 0;
    pool->slots_shift = slots_shift;
    
    const n_slots = 1 << pool->slots_shift;
    pool->slots = (struct string_pool_entry *)malloc(n_slots * sizeof(struct string_pool_entry));
    for (int i = 0; i < n_slots; i++) {
        struct string_pool_entry *slot = &pool->slots[i];
        slot->next = slot;
        slot->prev = slot;
        slot->value = NULL;
    }
    
    pthread_mutex_init(&pool->mutex, NULL);
}

void string_pool_free(struct string_pool *pool) {
    pthread_mutex_destroy(&pool->mutex);
    
    for (int i = 0; i < (1 << pool->slots_shift); i++) {
        struct string_pool_entry *slot = &pool->slots[i];
        while (!QUEUE_EMPTY(slot)) {
            struct string_pool_entry *node = slot->next;
            QUEUE_REMOVE(node);
            free(node);
        }
    }
    free(pool->slots);
    pool->n_entries = 0;
    pool->slots_shift = 0;
    pool->slots = NULL;
}

void string_pool_rehash(struct string_pool *pool, int slot_shift) {
    DCHECK(slot_shift >= 4 && "shift too small");
    
    const int new_size = (1 << slot_shift);
    struct string_pool_entry *new_slots = MALLOC_N(struct string_pool_entry, new_size);
    for (int i = 0; i < new_size; i++) {
        new_slots[i].next = &new_slots[i];
        new_slots[i].prev = &new_slots[i];
        new_slots[i].value = NULL;
    }
    
    for (int i = 0; i < (1 << pool->slots_shift); i++) {
        struct string_pool_entry *slot = &pool->slots[i];
        while (!QUEUE_EMPTY(slot)) {
            struct string_pool_entry *node = slot->next;
            struct string_pool_entry *new_slot = &new_slots[node->value->hash_code % new_size];
            QUEUE_REMOVE(node);
            QUEUE_INSERT_TAIL(new_slot, node);
        }
    }
    free(pool->slots);
    pool->slots = new_slots;
    pool->slots_shift = slot_shift;
}

struct string_pool_entry *string_pool_ensure_space(struct string_pool *pool, const char *z, size_t n) {
    const u32_t hash_code = yalx_str_hash(z, n);
    struct string_pool_entry *slot = &pool->slots[hash_code % (1 << pool->slots_shift)];
    for (struct string_pool_entry *i = slot->next; i != slot; i = i->next) {
        if (i->value->hash_code == hash_code &&
            memcmp(i->value->bytes, z, n) == 0) {
            return i;
        }
    }
    
    
    struct string_pool_entry *node = MALLOC(struct string_pool_entry);
    if (!node) {
        return NULL;
    }
    node->next = node;
    node->prev = node;
    node->value = NULL;
    
    const float factor = (float)(1 + pool->n_entries) / (float)(1 << pool->slots_shift);
    if (factor > KPOOL_REHASH_FACTOR) {
        string_pool_rehash(pool, pool->slots_shift + 1);
        
        // After rehash: update slot pointer
        slot = &pool->slots[hash_code % (1 << pool->slots_shift)];
    }
    QUEUE_INSERT_TAIL(slot, node);
    pool->n_entries++;
    return node;
}

int yalx_init_heap(struct heap *heap, gc_t gc) {
    memset(heap, 0, sizeof(*heap));
    heap->gc = gc;
    
    switch (heap->gc) {
        case GC_NONE:
            heap->one_time_pool.size  = 10 * MB;
            heap->one_time_pool.chunk = (address_t)malloc(heap->one_time_pool.size);
            if (!heap->one_time_pool.chunk) {
                return -1;
            }
            heap->one_time_pool.free  = heap->one_time_pool.chunk;
            
            heap->allocate = allocate_from_pool;
            heap->finalize = finalize_for_pool;
            break;
        case GC_LXR:
            lxr_init_immix_heap(&heap->lxr_immix, 16);
            
            heap->allocate = allocate_from_lxr;
            heap->finalize = finalize_for_lxr;
            break;
        default:
            assert(!"unreachable");
            break;
    }
    
    
    //string_pool_init(&heap->string_pool, 4);
    for (int i = 0; i < KPOOL_STRIPES_SIZE; i++) {
        string_pool_init(&heap->kpool_stripes[i], 4);
    }
    
    boxing_number_pool_init(heap, &heap->fast_boxing_numbers);
    pthread_mutex_init(&heap->mutex, NULL);
    return 0;
}

void yalx_free_heap(struct heap *heap) {
    heap->finalize(heap);
    
    pthread_mutex_destroy(&heap->mutex);
    
    for (int i = 0; i < KPOOL_STRIPES_SIZE; i++) {
        string_pool_free(&heap->kpool_stripes[i]);
    }
}


struct allocate_result yalx_heap_allocate(struct heap *heap, const struct yalx_class *klass, size_t size, u32_t flags) {
    struct allocate_result rv = heap->allocate(heap, size, flags);
    if (rv.status != ALLOCATE_OK) {
        return rv;
    }
    memset(rv.object, 0, size);
    rv.object->refs = 0;
    rv.object->tags = 0;
    rv.object->klass = (uintptr_t)klass;
    rv.status = ALLOCATE_OK;
    return rv;
}


struct string_pool_entry *yalx_ensure_space_kpool(struct heap *heap, const char *z, size_t n) {
    if (n > IN_POOL_STR_LEN) {
        return NULL;
    }
    
    const u32_t hash_code = yalx_str_hash(z, n);
    struct string_pool *kpool = &heap->kpool_stripes[hash_code % KPOOL_STRIPES_SIZE];
    
    pthread_mutex_lock(&kpool->mutex);
    struct string_pool_entry *space =string_pool_ensure_space(kpool, z, n);
    pthread_mutex_unlock(&kpool->mutex);
    return space;
}


void yalx_heap_visit_root(struct heap *heap, struct yalx_root_visitor *visitor) {
    {
    #define VISIT(ty)                        \
        visitor->visit_pointers(visitor,     \
            (yalx_ref_t *)pool->ty##_values, \
            (yalx_ref_t *)pool->ty##_values + arraysize(pool->ty##_values))
        
        struct boxing_number_pool *pool = &heap->fast_boxing_numbers;
        VISIT(bool);
        VISIT(i8);
        VISIT(u8);
        VISIT(i16);
        VISIT(u16);
        VISIT(i32);
        VISIT(u32);
        VISIT(i64);
        VISIT(u64);
        VISIT(f32);
        VISIT(f64);
    #undef VISIT
    }
    for (int i = 0; i < arraysize(heap->kpool_stripes); i++) {
        struct string_pool *pool = &heap->kpool_stripes[i];
        for (int j = 0; j < (1u << pool->slots_shift); j++) {
            struct string_pool_entry *slot = &pool->slots[j];
            for (struct string_pool_entry *n = slot->next; n != slot; n = n->next) {
                visitor->visit_pointer(visitor, (yalx_ref_t *)&n->value);
            }
        }
    }
}


void post_write_barrier(struct heap *heap, struct yalx_value_any *host, struct yalx_value_any *mutator) {
    // TODO:
}

void post_write_barrier_batch(struct heap *heap, struct yalx_value_any *host, struct yalx_value_any **mutators,
                              size_t nitems) {
    // TODO:
}
