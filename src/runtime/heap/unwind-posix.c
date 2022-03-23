#include "runtime/runtime.h"
#include "runtime/checking.h"
#include "runtime/heap/heap.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/type.h"
#include "runtime/object/throwable.h"
#include "runtime/object/arrays.h"
#include <libunwind.h>

void yalx_Zplang_Zolang_Zdunwind_stub() {
    //struct back heap_alloc(backtrace_frame_class);
    unw_cursor_t cursor;
    unw_context_t context;
    
    // Initialize cursor to current frame for local unwinding.
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    
    struct backtrace_frame **frames = NULL;
    size_t capacity = 16;
    size_t size = 0;
    frames = realloc(frames, capacity * sizeof(*frames));
    assert(frames);

    // Unwind frames one by one, going up the frame stack.
    while (unw_step(&cursor) > 0) {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {
            break;
        }
        struct backtrace_frame *frame = (struct backtrace_frame *)heap_alloc(backtrace_frame_class);
        frame->address = (address_t)pc;

        char name[256];
        if (unw_get_proc_name(&cursor, name, arraysize(name), &offset) == 0) {
            frame->function = yalx_new_string(&heap, name, strlen(name));
        } else {
            frame->function = yalx_new_string(&heap, "<unknown>", 9);
        }
        frame->line = 0;
        frame->file = yalx_new_string(&heap, "<unknown>", 9);
        
        if (size + 1 > capacity) {
            capacity <<= 1;
            frames = realloc(frames, capacity * sizeof(*frames));
            assert(frames);
        }
        frames[size++] = frame;
    }
    
    struct yalx_value_refs_array *array = yalx_new_refs_array(&heap, backtrace_frame_class, (yalx_ref_t *)frames, size);
    free(frames);
    yalx_return_ref((yalx_ref_t)array);
}
