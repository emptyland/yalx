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

struct backtrace_frame **yalx_unwind(size_t *depth, int dummy) {
    assert(dummy >= 0);
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
        unw_word_t offset = 0, pc = 0;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {
            break;
        }
        if (dummy-- > 0) {
            continue;
        }
        struct backtrace_frame *frame = (struct backtrace_frame *)heap_alloc(backtrace_frame_class);
    #if defined(YALX_ARCH_ARM64)
        pc &= 0xfffffffffffull; // valid address only 48 bits
    #endif
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

        if (size + 1 > capacity) {
            capacity <<= 1;
            frames = realloc(frames, capacity * sizeof(*frames));
            assert(frames);
        }
        frames[size++] = frame;
    }
    *depth = size;
    return frames;
}

void yalx_Zplang_Zolang_Zdunwind_stub() {
    size_t size = 0;
    struct backtrace_frame **frames = yalx_unwind(&size, 1);
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
        unw_word_t pc = 0;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {
            break;
        }
        
        unw_word_t offset = 0;
        char name[256];
        unw_get_proc_name(&cursor, name, arraysize(name), &offset);
        
    #if defined(YALX_ARCH_ARM64)
        pc &= 0xfffffffffffull; // valid address only 48 bits
    #endif
        if (((address_t)pc - offset) == co->top_unwind_point->addr) {
            co->exception = exception;
        #if defined(YALX_ARCH_X64)
            unw_word_t rbp = 0, rsp = 0;
            unw_get_reg(&cursor, UNW_X86_64_RBP, &rbp);
            unw_get_reg(&cursor, UNW_REG_SP, &rsp);
            
            address_t pc = *((address_t *)(rsp - 8));
            throw_to(co, pc, (address_t)rbp, (address_t)rsp);
        #elif defined(YALX_ARCH_ARM64)
            unw_word_t fp = 0, sp = 0;
            unw_get_reg(&cursor, UNW_ARM64_FP, &fp);
            unw_get_reg(&cursor, UNW_REG_SP, &sp);
            
            address_t pc = *((address_t *)(sp - 8));
            throw_to(co, pc, (address_t)fp, (address_t)sp);
        #endif
        }
    }
    assert(!"Unreachable");
}
