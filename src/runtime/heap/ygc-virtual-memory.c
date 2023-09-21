#include "runtime/heap/ygc.h"
#include "runtime/checking.h"
#include "runtime/runtime.h"

int reserve_continuous_memory(uintptr_t addr, size_t size);
int probe_continuous_memories(struct virtual_memory_management *vmm, uintptr_t addr, size_t size);
void release_mapping(uintptr_t addr, size_t size);
struct memory_segment *new_segment(uintptr_t addr, size_t size);

int virtual_memory_management_init(struct virtual_memory_management *self, size_t capacity) {
    DCHECK(capacity > 0);
    self->limit_in_bytes = ROUND_UP(capacity, PAGE_GRANULE_SIZE);
    self->unused_in_bytes = 0;
    self->free.next = &self->free;
    self->free.prev = &self->free;
    yalx_mutex_init(&self->mutex);

    return probe_continuous_memories(self, 0, self->limit_in_bytes);
}

void virtual_memory_management_final(struct virtual_memory_management *self) {
    yalx_mutex_final(&self->mutex);

    while (!QUEUE_EMPTY(&self->free)) {
        struct memory_segment *segment = self->free.next;
        QUEUE_REMOVE(segment);
        self->unused_in_bytes += segment->size;

        release_mapping(segment->addr, segment->size);
        free(segment);
    }
}

static struct memory_segment *find_fit_from_front(struct virtual_memory_management *self, size_t size) {
    for (struct memory_segment *s = self->free.next; s != &self->free; s = s->next) {
        if (s->size == size) {
            QUEUE_REMOVE(s);
            return s;
        }
        if (size < s->size) {
            struct memory_segment *fit = new_segment(s->addr, size);
            s->addr += size;
            s->size -= size;
            return fit;
        }
    }
    return NULL;
}

static struct memory_segment *find_fit_from_end(struct virtual_memory_management *self, size_t size) {
    for (struct memory_segment *s = self->free.prev; s != &self->free; s = s->prev) {
        if (s->size == size) {
            QUEUE_REMOVE(s);
            return s;
        }
        if (size < s->size) {
            struct memory_segment *fit = new_segment(s->addr + size, size);
            s->size -= size;
            return fit;
        }
    }
    return NULL;
}

linear_address_t allocate_virtual_memory(struct virtual_memory_management *self, size_t size, int force_from_end) {
    size_t required_in_bytes = ROUND_UP(size, PAGE_GRANULE_SIZE);

    struct memory_segment *fit = NULL;
    if (force_from_end) {
        fit = find_fit_from_end(self, required_in_bytes);
    } else {
        fit = find_fit_from_front(self, required_in_bytes);
    }
    linear_address_t rv;
    if (!fit) {
        rv.addr = UINTPTR_MAX;
        rv.size = 0;
    } else {
        rv.addr = fit->addr;
        rv.size = fit->size;
    }
    free(fit);
    return rv;
}

void free_virtual_memory(struct virtual_memory_management *self, linear_address_t addr) {
    yalx_mutex_lock(&self->mutex);
    release_virtual_memory(self, addr.addr, addr.size);
    yalx_mutex_unlock(&self->mutex);
}

void release_virtual_memory(struct virtual_memory_management *self, uintptr_t addr, size_t size) {
    DLOG(INFO, "release [0x%lx,0x%lx)", addr, addr + size);

    for (struct memory_segment *s = self->free.next; s != &self->free; s = s->next) {
        if (addr + size == s->addr) { // Can be shrink
            s->addr = addr;
            s->size += size;
            return;
        }

        if (s->addr > addr) { // Fit position
            struct memory_segment *fit = new_segment(addr, size);
            QUEUE_INSERT_TAIL(s, fit);
            return;
        }
    }

    struct memory_segment *fit = new_segment(addr, size);
    QUEUE_INSERT_TAIL(&self->free, fit); // No fit position, insert into free list tail.
}

int probe_continuous_memories(struct virtual_memory_management *vmm, uintptr_t addr, size_t size) {
    DCHECK(size % PAGE_GRANULE_SIZE == 0);
    if (reserve_continuous_memory(addr, size)) {
        release_virtual_memory(vmm, addr, size);
        return 0;
    }
    if (size <= PAGE_GRANULE_SIZE) {
        return 0;
    }
    return probe_continuous_memories(vmm, addr, size / 2) &&
           probe_continuous_memories(vmm, addr + size / 2, size / 2);
}