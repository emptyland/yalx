#pragma once
#ifndef YALX_RUNTIME_ROOT_HANDLES_H
#define YALX_RUNTIME_ROOT_HANDLES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct yalx_root_visitor;
struct yalx_value_any;

void yalx_free_root_handles();

void yalx_add_root_handle(struct yalx_value_any *obj);
void yalx_add_root_handles(struct yalx_value_any **begin, size_t n);

struct yalx_value_any **yalx_get_root_handles(size_t *n);
void yalx_root_handles_visit(struct yalx_root_visitor *visitor);


#ifdef __cplusplus
}
#endif

#endif //YALX_RUNTIME_ROOT_HANDLES_H
