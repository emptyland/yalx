#include "runtime/object/yalx-string.h"
#include "runtime/runtime.h"
#include <gtest/gtest.h>

extern "C" {

void issue9_stub(i32_t a, yalx_str_handle s) {
    printf("%d\n", a);
    // TODO
}

void issue10_stub() {
    ASSERT_EQ(0, yalx_return_i32(1));
    ASSERT_EQ(0, yalx_return_i32(2));
    ASSERT_EQ(0, yalx_return_i32(3));
    ASSERT_EQ(0, yalx_return_cstring("hello", 5));
}

void assert_string_stub(yalx_str_handle a, yalx_str_handle b) {
    ASSERT_STREQ(yalx_str_bytes(a), yalx_str_bytes(b));
}

void assert_int_stub(i32_t a, i32_t b) {
    ASSERT_EQ(a, b);
}

void yalx_Zplang_Zolang_ZdOptional_Dkissue06_Zoissue06_ZdBar_Dl_ZdunwarpOr() {
    FAIL() << "Just dummy";
}

} // extern "C"
