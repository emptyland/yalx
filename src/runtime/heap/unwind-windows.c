#include "runtime/runtime.h"
#include "runtime/process.h"
#include "runtime/checking.h"
#include "runtime/heap/heap.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/type.h"
#include "runtime/object/throwable.h"
#include "runtime/object/arrays.h"
#include <stdio.h>


struct backtrace_frame **yalx_unwind(size_t *depth, int dummy) {
    return NULL;
}

void yalx_Zplang_Zolang_Zdunwind_stub(void) {

}

void throw_it(struct yalx_value_any *exception) {

}