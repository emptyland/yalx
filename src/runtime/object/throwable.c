#include "runtime/object/throwable.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/type.h"
#include "runtime/heap/heap.h"
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


void throw_bad_casting_exception(const struct yalx_class *const from, const struct yalx_class *const to) {
    char *buf = (char *)malloc(1024);
    snprintf(buf, 1024, "%s -> %s", from->location.z, to->location.z);
    struct yalx_value_str *message = yalx_new_string(&heap, buf, strlen(buf));
    free(buf);
    
    struct yalx_value_exception *ex = heap_alloc(bad_casting_exception_class);
    assert(ex != NULL);
    ex->linked = NULL;
    ex->message = message;
    post_write_barrier(&heap, ex, message);
    
    size_t n = 0;
    struct backtrace_frame **frames = yalx_unwind(&n, 2/*dummy*/);
    struct yalx_value_refs_array *bt = yalx_new_refs_array(&heap, backtrace_frame_class, frames, n);
    free(frames);
    
    ex->backtrace = bt;
    post_write_barrier(&heap, ex, bt);
    
    throw_it(ex);
    assert(!"Unreachable");
}
