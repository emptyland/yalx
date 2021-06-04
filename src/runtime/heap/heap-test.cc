#include "runtime/heap/heap.h"
#include "runtime/object/yalx-string.h"
#include <gtest/gtest.h>

class HeapTest : public ::testing::Test {
public:
    void SetUp() override {
        yalx_init_heap(&heap_);
        string_pool_init(&kpool_, 4);
    }
    
    void TearDown() override {
        string_pool_free(&kpool_);
        yalx_free_heap(&heap_);
    }
    
    struct heap heap_;
    struct string_pool kpool_;
};

TEST_F(HeapTest, Sanity) {
    auto space = string_pool_ensure_space(&kpool_, "a", 1);
    ASSERT_EQ(nullptr, space->value);
    
    auto str = yalx_new_string_direct(&heap_, "a", 1);
    ASSERT_NE(nullptr, str);
    ASSERT_STREQ("a", str->bytes);
    
    space->value = str;
    space = string_pool_ensure_space(&kpool_, "a", 1);
    ASSERT_EQ(space->value, str);
}

TEST_F(HeapTest, StringPoolRehash) {
    ASSERT_EQ(4, kpool_.slots_shift);
    
    char buf[16];
    for (int i = 0; i < 32; i++) {
        sprintf(buf, "%04x", i);
        auto str = yalx_new_string_direct(&heap_, buf, strlen(buf));
        auto space = string_pool_ensure_space(&kpool_, str->bytes, str->len);
        space->value = str;
    }
    
    ASSERT_EQ(6, kpool_.slots_shift);
}
