#include "runtime/root-handles.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/checking.h"
#include <stdlib.h>

struct yalx_value_any **root_handles = NULL;
size_t n_root_handles = 0;
size_t capacity = 0;

void yalx_free_root_handles() {
    free(root_handles);
    root_handles = NULL;
    n_root_handles = 0;
    capacity = 0;
}

void yalx_add_root_handle(struct yalx_value_any *obj) {
    yalx_add_root_handles(&obj, 1);
}

void yalx_add_root_handles(struct yalx_value_any **begin, size_t n) {
    if (n_root_handles + n > capacity) {
        if (capacity == 0) {
            capacity = 16;
        } else {
            capacity <<= 1;
        }
        root_handles = (struct yalx_value_any **) realloc(root_handles, capacity * sizeof(struct yalx_value_any *));
    }
    for (size_t i = 0; i < n; i++) {
        root_handles[n_root_handles++] = begin[i];
    }
}

struct yalx_value_any **yalx_get_root_handles(size_t *n) {
    *n = n_root_handles;
    return root_handles;
}

void yalx_root_handles_visit(struct yalx_root_visitor *visitor) {
    DLOG(INFO, "Visit root handles for testing, %p (%zd)", root_handles, n_root_handles);
    visitor->visit_pointers(visitor, root_handles, root_handles + n_root_handles);
}