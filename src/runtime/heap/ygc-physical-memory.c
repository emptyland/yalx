#include "runtime/heap/ygc.h"
#include "runtime/checking.h"
#include "runtime/runtime.h"

struct memory_segment *new_segment(uintptr_t addr, size_t size) {
    struct memory_segment *segment = MALLOC(struct memory_segment);
    segment->addr = addr;
    segment->size = size;
    segment->prev = segment;
    segment->next = segment;
    return segment;
}

static struct memory_segment *split_segment(struct memory_segment *origin, size_t required_bytes) {
    //QUEUE_REMOVE(origin);
    DCHECK(origin->size > required_bytes);
    struct memory_segment *to = new_segment(origin->addr, required_bytes);
    origin->addr += required_bytes;
    origin->size -= required_bytes;
    return to;
}

int physical_memory_management_init(struct physical_memory_management *self, uintptr_t base_addr, size_t capacity) {
    DCHECK(capacity > 0);
    capacity = ROUND_UP(capacity, PAGE_GRANULE_SIZE);

    self->capacity = capacity;
    self->unused_in_bytes = capacity;
    self->base_addr = base_addr;
    self->limit_addr = base_addr + capacity;
    self->free.next = &self->free;
    self->free.prev = &self->free;

    struct memory_segment *segment = new_segment(self->base_addr, capacity);
    QUEUE_INSERT_TAIL(&self->free, segment);

    yalx_mutex_init(&self->mutex);
    return 0;
}

void physical_memory_management_final(struct physical_memory_management *self) {
    yalx_mutex_final(&self->mutex);

    while (!QUEUE_EMPTY(&self->free)) {
        struct memory_segment *segment = self->free.next;
        QUEUE_REMOVE(segment);
        self->unused_in_bytes += segment->size;
        free(segment);
    }
}

int allocate_physical_memory(struct physical_memory_management *self, struct physical_memory *mem, size_t size) {
    DCHECK(size > 0);
    mem->segments.next = &mem->segments;
    mem->segments.prev = &mem->segments;
    mem->n_segments = 0;
    mem->size_in_bytes = 0;
    size = ROUND_UP(size, PAGE_GRANULE_SIZE);

    yalx_mutex_lock(&self->mutex);

    if (size >= self->unused_in_bytes) {
        goto fail;
    }
    if (QUEUE_EMPTY(&self->free)) {
        goto fail;
    }

    size_t required_in_bytes = size;
    struct memory_segment *s = self->free.next;
    while (s != &self->free) {
        struct memory_segment *x = s;
        if (required_in_bytes >= x->size) {
            s = x->next;
            QUEUE_REMOVE(x);
        } else {
            s = x->next;
            x = split_segment(x, required_in_bytes);
        }
        required_in_bytes -= x->size;
        QUEUE_INSERT_TAIL(&mem->segments, x);
        mem->n_segments ++;
        if (required_in_bytes == 0) {
            break;
        }
    }

    if (mem->n_segments > 0) {
        mem->size_in_bytes = size;
        self->unused_in_bytes -= mem->size_in_bytes;
    }
    yalx_mutex_unlock(&self->mutex);
    return mem->n_segments > 0 ? 0 : -1;
fail:
    yalx_mutex_unlock(&self->mutex);
    return -1;
}

static void try_shrink_or_free_segment(struct physical_memory_management *self, struct memory_segment *segment) {
    for (struct memory_segment *s = self->free.next; s != &self->free; s = s->next) {
        if (segment->addr + segment->size == s->addr) { // Can be shrink
            s->addr = segment->addr;
            s->size += segment->size;
            free(segment);
            return;
        }

        if (s->addr > segment->addr) { // Fit position
            QUEUE_INSERT_TAIL(s, segment);
            return;
        }
    }
    QUEUE_INSERT_TAIL(&self->free, segment); // No fit position, insert into free list tail.
}

void free_physical_memory(struct physical_memory_management *self, struct physical_memory *mem) {
    yalx_mutex_lock(&self->mutex);

    while (!QUEUE_EMPTY(&mem->segments)) {
        struct memory_segment *x = mem->segments.next;
        QUEUE_REMOVE(x);
        try_shrink_or_free_segment(self, x);
    }

    self->unused_in_bytes += mem->size_in_bytes;
    yalx_mutex_unlock(&self->mutex);

    mem->size_in_bytes = 0;
    mem->n_segments = 0;
}

void debug_physical_memory_management(struct physical_memory_management *self) {
    if (QUEUE_EMPTY(&self->free)) {
        DLOG(WARN, "free list is empty");
    }
    for (struct memory_segment *s = self->free.next; s != &self->free; s = s->next) {
        DLOG(INFO, "[0x%lx,0x%lx)", s->addr, s->addr + s->size);
    }
}