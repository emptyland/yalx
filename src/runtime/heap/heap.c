#include "runtime/heap/heap.h"
#include "runtime/heap/ygc.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/number.h"
#include "runtime/object/any.h"
#include "runtime/object/type.h"
#include "runtime/checking.h"
#include <stdlib.h>
#include <string.h>


struct heap *heap = NULL;

struct no_gc_heap {
    struct heap heap;
    struct one_time_memory_pool pool;
};

struct ygc_heap {
    struct heap heap;
    struct ygc_core ygc;
};

static void prefix_write_barrier_no_op(struct heap *h, struct yalx_value_any *host, struct yalx_value_any *mutator) {}
static void prefix_write_barrier_batch_no_op(struct heap *h, struct yalx_value_any *host, struct yalx_value_any **mutators,
                                             size_t nitems) {}

static void post_write_barrier_no_op(struct heap *h, struct yalx_value_any **field, struct yalx_value_any *mutator) {}
static void post_write_barrier_batch_no_op(struct heap *h, struct yalx_value_any **fields, struct yalx_value_any **mutators,
                                           size_t nitems) {}
static void init_write_barrier_no_op(struct heap *h, struct yalx_value_any **field) {}
static void init_write_barrier_batch_no_op(struct heap *h, struct yalx_value_any **fields, size_t nitems) {}

static struct barrier_set barrier_no_op = {
    prefix_write_barrier_no_op,
    prefix_write_barrier_batch_no_op,
    post_write_barrier_no_op,
    post_write_barrier_batch_no_op,
    init_write_barrier_no_op,
    init_write_barrier_batch_no_op,
};

static struct allocate_result allocate_from_pool(struct heap *h, size_t size, u32_t flags) {
    USE(flags);
    struct one_time_memory_pool *pool = &((struct no_gc_heap *)h)->pool;

    //yalx_mutex_lock(&h->mutex); // TODO: used atomic ops
    DCHECK(pool->free >= pool->chunk);
    size_t used_in_bytes = pool->free - pool->chunk;
    DCHECK(used_in_bytes <= pool->size);
    size_t free_in_bytes = pool->size - used_in_bytes;
    size_t n = ROUND_UP(size, OBJECT_ALIGNMENT_IN_BYTES);
    
    struct allocate_result rv = {NULL, ALLOCATE_NOTHING};
    if (n > free_in_bytes) {
        rv.status = ALLOCATE_NOT_ENOUGH_MEMORY;
        //yalx_mutex_unlock(&h->mutex);
        return rv;
    }
    rv.object = (yalx_ref_t)pool->free;
    rv.status = ALLOCATE_OK;
    pool->free += n;
    //yalx_mutex_unlock(&h->mutex);

    dbg_init_zag(rv.object, n);
    return rv;
}

static void finalize_for_pool(struct heap *h) {
    struct one_time_memory_pool *pool = &((struct no_gc_heap *)h)->pool;

    dbg_free_zag(pool->chunk, pool->size);
    free(pool->chunk);
}

static int is_in_pool(const struct heap *h, uintptr_t addr) {
    const struct one_time_memory_pool *pool = &((const struct no_gc_heap *)h)->pool;
    const address_t ptr = (address_t)addr;
    return ptr >= pool->chunk && ptr < pool->free;
}

static struct allocate_result allocate_from_ygc(struct heap *h, size_t size, u32_t flags) {
    USE(flags);
    struct ygc_core *ygc = &((struct ygc_heap *)h)->ygc;

    struct allocate_result rs = {NULL, ALLOCATE_NOTHING};
    address_t chunk = ygc_allocate_object(ygc, size, OBJECT_ALIGNMENT_IN_BYTES);
    if (!chunk) {
        rs.status = ALLOCATE_NOT_ENOUGH_MEMORY;
        return rs;
    }

    rs.object = (yalx_ref_t)chunk;
    rs.status = ALLOCATE_OK;
    return rs;
}

static void finalize_for_ygc(struct heap *h) {
    struct ygc_core *ygc = &((struct ygc_heap *)h)->ygc;
    ygc_final(ygc);
}

static int is_in_ygc_heap(const struct heap *h, uintptr_t addr) {
    const struct ygc_core *ygc = &((const struct ygc_heap *)h)->ygc;
    return (addr & YGC_METADATA_MASK) && ygc_addr_in_heap(ygc, addr);
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

static int boxing_number_pool_init(struct heap *h, struct boxing_number_pool *pool) {
    pool->bool_values[0] = yalx_new_small_boxing_number(h, Bool_class);
    pool->bool_values[1] = yalx_new_small_boxing_number(h, Bool_class);
    pool->bool_values[0]->box.u32 = 0;
    pool->bool_values[1]->box.u32 = 1;
    
    for (int i = 0; i < 256; i++) {
        pool->u8_values[i] = yalx_new_small_boxing_number(h, U8_class);
        pool->u8_values[i]->box.u32 = i;
        
        pool->i8_values[i] = yalx_new_small_boxing_number(h, I8_class);
        pool->i8_values[i]->box.i8 = ((int8_t)(i - 128));
    }
    for (int i = 0; i < 201; i++) {
        pool->u16_values[i] = yalx_new_small_boxing_number(h, U16_class);
        pool->u16_values[i]->box.u32 = i;
        
        pool->i16_values[i] = yalx_new_small_boxing_number(h, I16_class);
        pool->i16_values[i]->box.i16 = ((int16_t)(i - 100));
        
        pool->u32_values[i] = yalx_new_small_boxing_number(h, U32_class);
        pool->u32_values[i]->box.u32 = i;
        
        pool->i32_values[i] = yalx_new_small_boxing_number(h, I32_class);
        pool->i32_values[i]->box.i32 = i - 100;
        
        pool->u64_values[i] = yalx_new_big_boxing_number(h, U64_class);
        pool->u64_values[i]->box.u64 = i;
        
        pool->i64_values[i] = yalx_new_big_boxing_number(h, I64_class);
        pool->i64_values[i]->box.i64 = i - 100;
    }
    
    for (int i = 0; i < arraysize(fast_boxing_f32_table); i++) {
        pool->f32_values[i] = yalx_new_small_boxing_number(h, F32_class);
        pool->f32_values[i]->box.f32 = fast_boxing_f32_table[i];
    }
    for (int i = 0; i < arraysize(fast_boxing_f64_table); i++) {
        pool->f64_values[i] = yalx_new_big_boxing_number(h, F64_class);
        pool->f64_values[i]->box.f64 = fast_boxing_f64_table[i];
    }
    return 0;
}

void string_pool_init(struct string_pool *pool, int slots_shift) {
    DCHECK(slots_shift >= 4 && "shift too small");
    
    pool->n_entries = 0;
    pool->slots_shift = slots_shift;
    
    const size_t n_slots = 1 << pool->slots_shift;
    pool->slots = (struct string_pool_entry *)malloc(n_slots * sizeof(struct string_pool_entry));
    for (int i = 0; i < n_slots; i++) {
        struct string_pool_entry *slot = &pool->slots[i];
        slot->next = slot;
        slot->prev = slot;
        slot->value = NULL;
    }
    
    yalx_mutex_init(&pool->mutex);
}

void string_pool_free(struct string_pool *pool) {
    yalx_mutex_final(&pool->mutex);
    
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

int yalx_init_heap(gc_t gc, size_t max_heap_in_bytes, struct heap **receiver) {
    assert(*receiver == NULL); // Make sure has not init.

    switch (gc) {
        case GC_NONE: {
            struct no_gc_heap *h = MALLOC(struct no_gc_heap);
            if (!h) {
                return -1;
            }
            h->pool.size = max_heap_in_bytes;
            h->pool.chunk = (address_t)malloc(h->pool.size);
            if (!h->pool.chunk) {
                return -1;
            }
            h->pool.free = h->pool.chunk;
            h->heap.allocate = allocate_from_pool;
            h->heap.is_in = is_in_pool;
            h->heap.finalize = finalize_for_pool;
            h->heap.barrier_ops = barrier_no_op;
            *receiver = (struct heap *) h;
        } break;

        case GC_YGC: {
            struct ygc_heap *h = MALLOC(struct ygc_heap);
            if (!h) {
                return -1;
            }
            if (ygc_init(&h->ygc, max_heap_in_bytes) < 0) {
                return -1;
            }
            h->heap.allocate = allocate_from_ygc;
            h->heap.is_in = is_in_ygc_heap;
            h->heap.finalize = finalize_for_ygc;
            *receiver = (struct heap *) h;
        } break;

        case GC_LXR:
        default:
            assert(!"unreachable");
            break;
    }

    (*receiver)->gc = gc;
    //string_pool_init(&heap->string_pool, 4);
    for (int i = 0; i < KPOOL_STRIPES_SIZE; i++) {
        string_pool_init(&(*receiver)->kpool_stripes[i], 4);
    }
    
    boxing_number_pool_init((*receiver), &(*receiver)->fast_boxing_numbers);
    yalx_mutex_init(&(*receiver)->mutex);
    return 0;
}

void yalx_free_heap(struct heap *h) {
    h->finalize(h);
    
    yalx_mutex_final(&h->mutex);
    
    for (int i = 0; i < KPOOL_STRIPES_SIZE; i++) {
        string_pool_free(&h->kpool_stripes[i]);
    }

    free(h);
}


struct allocate_result yalx_heap_allocate(struct heap *h, const struct yalx_class *klass, size_t size, u32_t flags) {
    struct allocate_result rv = h->allocate(h, size, flags);
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


struct string_pool_entry *yalx_ensure_space_kpool(struct heap *h, const char *z, size_t n) {
    if (n > IN_POOL_STR_LEN) {
        return NULL;
    }
    
    const u32_t hash_code = yalx_str_hash(z, n);
    struct string_pool *kpool = &h->kpool_stripes[hash_code % KPOOL_STRIPES_SIZE];
    
    yalx_mutex_lock(&kpool->mutex);
    struct string_pool_entry *space =string_pool_ensure_space(kpool, z, n);
    yalx_mutex_unlock(&kpool->mutex);
    return space;
}


void yalx_heap_visit_root(struct heap *h, struct yalx_root_visitor *visitor) {
    {
    #define VISIT(ty)                        \
        visitor->visit_pointers(visitor,     \
            (yalx_ref_t *)pool->ty##_values, \
            (yalx_ref_t *)pool->ty##_values + arraysize(pool->ty##_values))
        
        struct boxing_number_pool *pool = &h->fast_boxing_numbers;
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
    for (int i = 0; i < arraysize(h->kpool_stripes); i++) {
        struct string_pool *pool = &h->kpool_stripes[i];
        for (int j = 0; j < (1u << pool->slots_shift); j++) {
            struct string_pool_entry *slot = &pool->slots[j];
            for (struct string_pool_entry *n = slot->next; n != slot; n = n->next) {
                visitor->visit_pointer(visitor, (yalx_ref_t *)&n->value);
            }
        }
    }
}

void init_typing_write_barrier_if_needed(struct heap *h, const struct yalx_class *item, address_t data) {
    if (item->constraint == K_ENUM) {
        const uint16_t enum_code = *(uint16_t *)data;
        DCHECK(enum_code < item->n_fields);
        const struct yalx_class_field *field = &item->fields[enum_code];
        if (field->type->constraint == K_ENUM) {
            if (field->type->compact_enum) {
                init_write_barrier(h, (yalx_ref_t *)(data + field->offset_of_head));
            } else {
                init_typing_write_barrier_if_needed(h, field->type, data + field->offset_of_head);
            }
            return;
        }
        if (yalx_is_ref_type(field->type)) {
            init_write_barrier(h, (yalx_ref_t *)(data + field->offset_of_head));
            return;
        }
        if (field->type->constraint != K_STRUCT) {
            return;
        }
        item = field->type;
    }
    
    DCHECK(item->constraint == K_STRUCT);
    for (int i = 0; i < item->refs_mark_len; i++) {
        const struct yalx_class *const ty = item->refs_mark[i].ty;
        const size_t offset = item->refs_mark[i].offset;

        if (ty->constraint == K_ENUM) {
            if (ty->compact_enum) {
                init_write_barrier(h, (yalx_ref_t *)(data + offset));
            } else if (ty->refs_mark_len > 0) {
                init_typing_write_barrier_if_needed(h, ty, data + offset);
            }
        } else {
            init_write_barrier(h, (yalx_ref_t *)(data + offset));
        }
    }
}

void post_typing_write_barrier_if_needed(struct heap *h, const struct yalx_class *item, address_t location,
                                         address_t data) {
    if (item->constraint == K_ENUM) {
        const uint16_t enum_code = *(uint16_t *)data;
        DCHECK(enum_code < item->n_fields);
        const struct yalx_class_field *field = &item->fields[enum_code];
        if (field->type->constraint == K_ENUM) {
            if (field->type->compact_enum) {
                post_write_barrier(h, (yalx_ref_t *)(location + field->offset_of_head),
                                   *(yalx_ref_t *)(data + field->offset_of_head));
            } else {
                post_typing_write_barrier_if_needed(h, field->type, location + field->offset_of_head,
                                                    data + field->offset_of_head);
            }
            return;
        }
        if (yalx_is_ref_type(field->type)) {
            init_write_barrier(h, (yalx_ref_t *)(data + field->offset_of_head));
            return;
        }
        if (field->type->constraint != K_STRUCT) {
            return;
        }
        item = field->type;
    }
    
    DCHECK(item->constraint == K_STRUCT);
    for (int i = 0; i < item->refs_mark_len; i++) {
        const struct yalx_class *const ty = item->refs_mark[i].ty;
        const size_t offset = item->refs_mark[i].offset;

        if (ty->constraint == K_ENUM) {
            if (ty->compact_enum) {
                post_write_barrier(h, (yalx_ref_t *)(location + offset), *(yalx_ref_t *)(data + offset));
            } else if (ty->refs_mark_len > 0) {
                post_typing_write_barrier_if_needed(h, ty, location + offset, data + offset);
            }
        } else {
            post_write_barrier(h, (yalx_ref_t *)(location + offset), *(yalx_ref_t *)(data + offset));
        }
    }
}

