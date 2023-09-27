#include "runtime/heap/heap.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/type.h"
#include <gtest/gtest.h>

class YGCHeapTest : public ::testing::Test {
public:
    void SetUp() override {
        ASSERT_EQ(0, yalx_init_heap(GC_YGC, &heap_));
    }

    void TearDown() override {
        yalx_free_heap(heap_);
        heap_ = nullptr;
    }

    struct heap *heap_ = nullptr;
};

TEST_F(YGCHeapTest, Sanity) {
    auto str = yalx_new_string_direct(heap_, "hello", 5);
    ASSERT_STREQ("hello", str->bytes);

    ASSERT_TRUE(heap_->is_in(heap_, reinterpret_cast<uintptr_t>(str)));
    ASSERT_FALSE(heap_->is_in(heap_, 0));
}