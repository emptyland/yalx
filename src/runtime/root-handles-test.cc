#include "runtime/root-handles.h"
#include "runtime/heap/heap.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/any.h"
#include "gtest/gtest.h"

class RootHandlesTest : public ::testing::Test {
public:
    void TearDown() override {
        yalx_free_root_handles();
    }
};

TEST_F(RootHandlesTest, Sanity) {
    auto o1 = yalx_new_string(heap, "hello", 5);
    yalx_add_root_handle(reinterpret_cast<yalx_ref_t>(o1));

    auto o2 = yalx_new_string(heap, "world", 5);
    yalx_add_root_handle(reinterpret_cast<yalx_ref_t>(o2));

    size_t n = 0;
    auto objs = yalx_get_root_handles(&n);
    ASSERT_EQ(2, n);
    ASSERT_TRUE(objs != nullptr);
    EXPECT_EQ(reinterpret_cast<yalx_ref_t>(o1), objs[0]);
    EXPECT_EQ(reinterpret_cast<yalx_ref_t>(o2), objs[1]);
}