#pragma once
#ifndef YALX_RUNTIME_HEAP_YGC_H
#define YALX_RUNTIME_HEAP_YGC_H

#include "runtime/locks.h"
#include "runtime/runtime.h"
#include "runtime/locks.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SMALL_PAGE_SHIFT 21 // 2MB
#define SMALL_PAGE_SIZE (1L << SMALL_PAGE_SHIFT)

#define PAGE_GRANULE_SIZE SMALL_PAGE_SIZE


extern uintptr_t YGC_MARKED0_BIT; // 0x0000100000000000
extern uintptr_t YGC_MARKED1_BIT; // 0x0000200000000000
extern uintptr_t YGC_REMAPPED_BIT; // 0x0000400000000000
extern uintptr_t YGC_ADDRESS_OFFSET_MASK;
extern size_t YGC_VIRTUAL_ADDRESS_SPACE_LEN;

#define ygc_offset(addr) ((uintptr_t)(addr) & YGC_ADDRESS_OFFSET_MASK)
#define ygc_marked0(addr) ((uintptr_t)(addr) | YGC_MARKED0_BIT)
#define ygc_marked1(addr) ((uintptr_t)(addr) | YGC_MARKED1_BIT)
#define ygc_remapped(addr) ((uintptr_t)(addr) | YGC_REMAPPED_BIT)

struct per_cpu_storage;

struct linear_address {
    uintptr_t addr;
    size_t    size;
};

typedef struct linear_address linear_address_t;

struct memory_backing {
    size_t size;
#if defined(YALX_OS_LINUX)
    int fd; // only linux has fd
#endif // defined(YALX_OS_LINUX)
    int refs;
};

int memory_backing_init(struct memory_backing *backing, size_t capacity);
void memory_backing_final(struct memory_backing *backing);
void memory_backing_map(struct memory_backing *backing, uintptr_t addr, size_t size, uintptr_t offset);
void memory_backing_unmap(struct memory_backing *backing, uintptr_t addr, size_t size);


struct memory_segment {
    QUEUE_HEADER(struct memory_segment);
    uintptr_t addr;
    size_t    size;
};

struct physical_memory {
    size_t size_in_bytes;
    size_t n_segments;
    struct memory_segment segments;
};

struct physical_memory_management {
    size_t capacity;
    size_t unused_in_bytes;
    uintptr_t base_addr;
    uintptr_t limit_addr;
    struct memory_segment free;
    struct yalx_mutex mutex;
};

int physical_memory_management_init(struct physical_memory_management *self, uintptr_t base_addr, size_t capacity);

void physical_memory_management_final(struct physical_memory_management *self);

int allocate_physical_memory(struct physical_memory_management *self,
                             struct physical_memory *mem, size_t size);

void free_physical_memory(struct physical_memory_management *self, struct physical_memory *mem);

void debug_physical_memory_management(struct physical_memory_management *self);


struct virtual_memory_management {
    size_t limit_in_bytes;
    size_t unused_in_bytes;
    struct memory_segment free;
    struct yalx_mutex mutex;
};

int virtual_memory_management_init(struct virtual_memory_management *self, size_t capacity);
void virtual_memory_management_final(struct virtual_memory_management *self);

linear_address_t allocate_virtual_memory(struct virtual_memory_management *self, size_t size, int force_from_end);
void free_virtual_memory(struct virtual_memory_management *self, linear_address_t addr);
void release_virtual_memory(struct virtual_memory_management *self, uintptr_t addr, size_t size);


struct ygc_page {
    struct ygc_page *prev;
    struct ygc_page *next;
    linear_address_t virtual_addr;
    struct physical_memory physical_memory;
};

struct ygc_core {
    struct memory_backing backing;
    struct physical_memory_management pmm;
    struct virtual_memory_management vmm;
    struct per_cpu_storage *small_page;
    struct ygc_page volatile *medium_page;
    _Atomic size_t rss;
    struct ygc_page pages;
    struct yalx_mutex mutex;
};


int ygc_init(struct ygc_core *ygc, size_t capacity);
void ygc_final(struct ygc_core *ygc);

struct ygc_page *ygc_page_new(struct ygc_core *ygc, size_t size);
void ygc_page_free(struct ygc_core *ygc, struct ygc_page *pg);


#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_HEAP_YGC_H
