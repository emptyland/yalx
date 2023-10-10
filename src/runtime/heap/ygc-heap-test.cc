#include "runtime/heap/ygc.h"
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

TEST_F(YGCHeapTest, ObjectMarkingInPage) {
    auto hello = yalx_new_string_direct(heap_, "hello", 5);
    auto world = yalx_new_string_direct(heap_, "world", 5);
    auto doom = yalx_new_string_direct(heap_, "doom", 4);
    auto ygc = ygc_heap_of(heap_);

    auto page = ygc_addr_in_page(ygc, reinterpret_cast<uintptr_t>(hello));
    ASSERT_EQ(page, ygc_addr_in_page(ygc, reinterpret_cast<uintptr_t>(world)));
    ASSERT_EQ(page, ygc_addr_in_page(ygc, reinterpret_cast<uintptr_t>(doom)));

    //printf("%p\n", world->klass);
    ygc_page_mark_object(page, reinterpret_cast<yalx_value_any *>(world));

}