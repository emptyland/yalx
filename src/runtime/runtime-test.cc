#include "runtime/runtime.h"
#include "runtime/process.h"
#include <gtest/gtest.h>

void *stup(void *p) {
    //printf("%d\n", p == thread_local_mach);
    return thread_local_mach + 1;
}

TEST(RuntimeTest, Sanity) {
    //ASSERT_EQ(0, yalx_runtime_init());
    
    ASSERT_NE(0, ncpus);
    ASSERT_EQ(0, os_page_size % (4 * KB));
    
    ASSERT_EQ(0, asm_stub1(0, 0));
    ASSERT_EQ(1, asm_stub1(1, 0));
    ASSERT_EQ(2, asm_stub1(1, 1));
    
    ASSERT_EQ(1, asm_stub3());
    
    ASSERT_TRUE(strstr(yalx_version.z, "yalx-lang"));
    ASSERT_EQ(16, asm_stub2(&yalx_version));
    
    stup(0);
    ASSERT_EQ(static_cast<void *>(thread_local_mach), asm_stub5());
}


TEST(RuntimeTest, FillMemoryZag) {
    uint8_t b4[4] = {0};
    
    fill_memory_zag(b4, sizeof(b4), MEM_INIT_ZAG);
    ASSERT_EQ(0xcc, b4[0]);
    ASSERT_EQ(0xcc, b4[1]);
    ASSERT_EQ(0xcc, b4[2]);
    ASSERT_EQ(0xcc, b4[3]);
    fill_memory_zag(b4, sizeof(b4), MEM_FREE_ZAG);
    ASSERT_EQ(0xed, b4[0]);
    ASSERT_EQ(0xfe, b4[1]);
    ASSERT_EQ(0xed, b4[2]);
    ASSERT_EQ(0xfe, b4[3]);
    
    uint8_t b5[5] = {0};
    fill_memory_zag(b5, sizeof(b5), MEM_INIT_ZAG);
    ASSERT_EQ(0xcc, b5[0]);
    ASSERT_EQ(0xcc, b5[4]);
    fill_memory_zag(b5, sizeof(b5), MEM_FREE_ZAG);
    ASSERT_EQ(0xed, b5[0]);
    ASSERT_EQ(0xed, b5[4]);
}

TEST(RuntimeTest, C0M0) {
    ASSERT_EQ(m0.owns, &procs[0]);
    ASSERT_EQ(thread_local_mach, &m0);
    ASSERT_TRUE(yalx_is_m0(&m0));
}


TEST(RuntimeTest, MockRt0Sanity) {
    //yalx_rt0(0, NULL);
}

TEST(RuntimeTest, PkgInitOnce) {
    ASSERT_EQ(0, pkg_initialized_count());
    pkg_init_once(nullptr, "testing/test:test");
    ASSERT_EQ(1, pkg_initialized_count());
    ASSERT_TRUE(pkg_has_initialized("testing/test:test"));
}

TEST(RuntimeTest, CallReturningVals) {
    int buf[4] = {0};
    call_returning_vals(buf, sizeof(buf), reinterpret_cast<void *>(asm_stub6));
    ASSERT_EQ(220, buf[0]);
    ASSERT_EQ(199, buf[3]);
}
