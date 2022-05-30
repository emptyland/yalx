#include "runtime/lxr/block.h"
#include <gtest/gtest.h>

class LxrBlockTest : ::testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

int lxr_log2(size_t n) {
    int j = 0;
    for (size_t i = 1; i < n; i <<= 1) {
        j++;
    }
    return j;
}

TEST(LxrBlockTest, Log2) {
    ASSERT_EQ(1, lxr_log2(2));
    ASSERT_EQ(2, lxr_log2(4));
    ASSERT_EQ(3, lxr_log2(8));
    ASSERT_EQ(4, lxr_log2(16));
    ASSERT_EQ(4, lxr_log2(15));
}


