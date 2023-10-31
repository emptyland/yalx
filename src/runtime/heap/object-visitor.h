#pragma once
#ifndef YALX_RUNTIME_HEAP_OBJECT_VISITOR_H_
#define YALX_RUNTIME_HEAP_OBJECT_VISITOR_H_

#include <stddef.h>
#include <stdint.h>

typedef struct yalx_value_any *yalx_ref_t;

struct yalx_root_visitor {
    void *ctx;
    intptr_t reserved1;
    intptr_t reserved2;
    void (*visit_pointers)(struct yalx_root_visitor *v, yalx_ref_t *begin, yalx_ref_t *end);
    void (*visit_pointer)(struct yalx_root_visitor *v, yalx_ref_t *p);
}; // struct yalx_root_visitor


struct yalx_object_visitor {
    void *ctx;
    intptr_t reserved1;
    intptr_t reserved2;
    void (*visit_pointers)(struct yalx_object_visitor *v, yalx_ref_t host, yalx_ref_t *begin, yalx_ref_t *end);
    void (*visit_pointer)(struct yalx_object_visitor *v, yalx_ref_t host, yalx_ref_t *p);
}; // struct yalx_object_visitor

struct yalx_heap_visitor {
    void *ctx;
    intptr_t reserved1;
    intptr_t reserved2;
    void (*visit_pointer)(struct yalx_heap_visitor *v, yalx_ref_t o);
};

#endif // YALX_RUNTIME_HEAP_OBJECT_VISITOR_H_
