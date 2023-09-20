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

#define SMALL_PAGE_SIZE (2 * MB)


extern uintptr_t YGC_MARKED0_BIT; // 0x0000100000000000
extern uintptr_t YGC_MARKED1_BIT; // 0x0000200000000000
extern uintptr_t YGC_REMAPPED_BIT; // 0x0000400000000000
extern uintptr_t YGC_ADDRESS_OFFSET_MASK;

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

struct ygc_page {
    struct ygc_page *prev;
    struct ygc_page *next;
    linear_address_t virtual_addr;
    //struct os_page *os;
};

struct ygc_core {
    struct memory_backing backing;
    struct per_cpu_storage *small_page;
    struct ygc_page volatile *medium_page;
    _Atomic size_t rss;
};


int ygc_init(struct ygc_core *ygc, size_t capacity);
void ygc_final(struct ygc_core *ygc);

struct ygc_page *ygc_page_new(struct ygc_core *ygc, size_t size);
void ygc_page_free(struct ygc_page *pg);


#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_HEAP_YGC_H
