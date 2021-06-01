#include "runtime/runtime.h"
#include <gtest/gtest.h>

TEST(RuntimeTest, Sanity) {
    //ASSERT_EQ(0, yalx_runtime_init());
    
    ASSERT_NE(0, ncpus);
    ASSERT_EQ(0, os_page_size % (4 * KB));
    
    ASSERT_EQ(0, entry());
    ASSERT_EQ(0, asm_stub1(0, 0));
    ASSERT_EQ(1, asm_stub1(1, 0));
    ASSERT_EQ(2, asm_stub1(1, 1));
    
    ASSERT_TRUE(strstr(yalx_version.z, "yalx-lang"));
    ASSERT_EQ(16, asm_stub2(&yalx_version));
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
