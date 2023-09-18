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


struct linear_address {
    uintptr_t addr;
    size_t    size;
};

typedef struct linear_address linear_address_t;

struct os_page {
    struct os_page *prev;
    struct os_page *next;
    size_t size;
    address_t addr;
#if defined(YALX_OS_LINUX)
    int fd; // only linux has fd
#endif // defined(YALX_OS_LINUX)
    int refs;
};

struct page {
    struct page *prev;
    struct page *next;
    linear_address_t virtual_addr;
    struct os_page *os;
};

struct os_page *allocate_os_page(size_t size);
void free_os_page(struct os_page *page);

address_t map_virtual_addr(struct os_page *page, linear_address_t virtual_addr);

struct ygc_core {
    struct os_page os_pages;
    size_t rss;
};


int ygc_init(struct ygc_core *ygc);
void ygc_final(struct ygc_core *ygc);

#ifdef __cplusplus
}
#endif

#endif // YALX_RUNTIME_HEAP_YGC_H
