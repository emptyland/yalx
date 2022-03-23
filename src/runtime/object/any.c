#include "runtime/object/any.h"
#include "runtime/process.h"
#include <assert.h>


struct yalx_value_str *yalx_any_to_string(struct yalx_value_any *any) {
    
    return NULL;
}

void yalx_Zplang_Zolang_ZdAny_ZdhashCode_stub(yalx_ref_handle self) {
    uintptr_t val = (uintptr_t)*self;
    yalx_return_i32(CURRENT_COROUTINE->returning_vals, (val >> 2) & 0xffffffff);
}

void yalx_Zplang_Zolang_ZdAny_Zdid_stub(yalx_ref_handle self) {
    yalx_return_i32(CURRENT_COROUTINE->returning_vals, (*self)->oid);
}

void yalx_Zplang_Zolang_ZdAny_ZdisEmpty_stub(yalx_ref_handle self) {
    yalx_return_i32(CURRENT_COROUTINE->returning_vals, 1);
}

void yalx_Zplang_Zolang_ZdAny_ZdtoString_stub(yalx_ref_handle self) {
    char buf[64] = {0};
    snprintf(arraysize(buf), buf, "any@%p", *self);
    yalx_return_cstring(CURRENT_COROUTINE->returning_vals, buf, strlen(buf));
}

