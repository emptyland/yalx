#include "runtime/runtime.h"
#include "runtime/process.h"
#include "runtime/checking.h"
#include "runtime/heap/heap.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/type.h"
#include "runtime/object/throwable.h"
#include "runtime/object/arrays.h"
#include <libunwind.h>
#include <stdio.h>

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
            size_t n = yalx_symbol_demangle_on_place(name, strlen(name));
            frame->function = yalx_new_string(&heap, name, n);
        } else {
            frame->function = yalx_new_string(&heap, "<unknown>", 9);
        }
        frame->line = 0;
        frame->file = yalx_new_string(&heap, "<unknown>", 9);
        
//        {
//            unw_word_t fp, sp;
//            int r1 = unw_get_reg(&cursor, UNW_ARM64_FP, &fp);
//            int r2 = unw_get_reg(&cursor, UNW_REG_SP, &sp);
//            printf("%s(): %d = %p, %d = %p, size = %d\n", frame->function->bytes, r1, fp, r2, sp, fp - sp);
//        }
        
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

// implements in boot-[Os]-[Arch].s
extern void throw_to(struct coroutine *root, address_t pc, address_t fp, address_t sp);

void throw_it(struct yalx_value_any *exception) {
    struct coroutine *co = CURRENT_COROUTINE;
    assert(co->top_unwind_point != NULL);
    //co->top_unwind_point->
    
    //struct back heap_alloc(backtrace_frame_class);
    unw_cursor_t cursor;
    unw_context_t context;
    
    // Initialize cursor to current frame for local unwinding.
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    while (unw_step(&cursor) > 0) {
        unw_word_t pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {
            break;
        }
        
        unw_word_t offset = 0;
        char name[256];
        unw_get_proc_name(&cursor, name, arraysize(name), &offset);
        
        //printf("%s\n", name);
        if ((address_t)pc - offset == co->top_unwind_point->addr) {
            co->exception = exception;
        #if defined(YALX_ARCH_X64)
            unw_word_t rbp = 0, rsp = 0;
            unw_get_reg(&cursor, UNW_X86_64_RBP, &rbp);
            unw_get_reg(&cursor, UNW_REG_SP, &rsp);
            
            address_t pc = *((address_t *)(rsp - 8));
            throw_to(co, pc, (address_t)rbp, (address_t)rsp);
        #elif defined(YALX_ARCH_X64)
            assert(!"TODO");
        #endif
        }
    }
    assert(!"Unreachable");
}
