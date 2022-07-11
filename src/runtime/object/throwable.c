#include "runtime/object/throwable.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/type.h"
#include "runtime/heap/heap.h"
#include "runtime/checking.h"
#include "runtime/runtime.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void yalx_Zplang_Zolang_ZdThrowable_ZdprintBacktrace_stub(yalx_throwable_handle handle) {
    struct yalx_value_throwable *self = *handle;
    printf("%s: %s\n", CLASS(self)->location.z, self->message->bytes);
    const u32_t n = self->backtrace->len;
    const struct backtrace_frame **frames = (const struct backtrace_frame **)self->backtrace->data;
    for (int i = 0; i < n; i++) {
        const struct backtrace_frame *frame = frames[i];
        printf("  > %s@%p\n", frame->function->bytes, frame->address);
    }
}

static void throw_exception(const struct yalx_class *ty,
                            struct yalx_value_str *const message,
                            struct yalx_value_throwable *const cause) {
    struct yalx_value_exception *ex = (struct yalx_value_exception *)heap_alloc(ty);
    assert(ex != NULL);
    ex->linked = cause;
    init_write_barrier(&heap, (yalx_ref_t *)&ex->linked);
    ex->message = message;
    init_write_barrier(&heap, (yalx_ref_t *)&ex->message);
    
    size_t n = 0;
    struct backtrace_frame **frames = yalx_unwind(&n, 2/*dummy*/);
    struct yalx_value_array *ar = (struct yalx_value_array *)yalx_new_refs_array_with_data(&heap, backtrace_frame_class,
                                                                                           1, NULL, (yalx_ref_t *)frames,
                                                                                           n);
    free(frames);
    
    ex->backtrace = ar;
    init_write_barrier(&heap, (yalx_ref_t *)&ex->backtrace);
    
    throw_it((yalx_ref_t)ex);
    assert(!"Unreachable");
}

void throw_bad_casting_exception(const struct yalx_class *const from, const struct yalx_class *const to) {
    char *buf = (char *)malloc(1024);
    snprintf(buf, 1024, "%s -> %s", from->location.z, to->location.z);
    struct yalx_value_str *message = yalx_new_string(&heap, buf, strlen(buf));
    free(buf);
    throw_exception(bad_casting_exception_class, message, NULL);
}

void throw_array_index_out_of_bounds_exception(const struct yalx_value_array_header *obj, int dim, int index) {
    DCHECK(obj != NULL);
    const struct yalx_class *const klass = CLASS(obj);
    
    
    char *buf = (char *)malloc(1024);
    if (klass == array_class) {
        snprintf(buf, 1024, "Size is %u, but index at %u", obj->len, index);
    } else {
        DCHECK(klass == multi_dims_array_class);
        const struct yalx_value_multi_dims_array *ar = (const struct yalx_value_multi_dims_array *)obj;
        DCHECK(dim < ar->rank);
        snprintf(buf, 1024, "Dimension %d size is %u, but index at %u", dim, ar->caps[dim], index);
    }
    
    struct yalx_value_str *message = yalx_new_string(&heap, buf, strlen(buf));
    free(buf);
    
    throw_exception(array_index_out_of_bounds_exception_class, message, NULL);
}
