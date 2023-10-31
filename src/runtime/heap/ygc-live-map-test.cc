#include "runtime/heap/ygc-live-map.h"
#include "runtime/runtime.h"
#include "gtest/gtest.h"

class YGCLiveMapTest : public ::testing::Test {
public:
    void SetUp() override {
        live_map_init(&map_, yalx_log2(256));
    }

    void TearDown() override {
        live_map_final(&map_);
    }

    ygc_live_map map_{};
};

TEST_F(YGCLiveMapTest, Sanity) {
    ASSERT_EQ(8, pointer_size_in_bytes);
    ASSERT_EQ(3, pointer_shift_in_bytes);
    ASSERT_EQ(64, pointer_size_in_bits);
    ASSERT_EQ(6, pointer_shift_in_bits);

    ASSERT_EQ(0x3F, pointer_mask_in_bits);

    ASSERT_EQ(1, 65 >> pointer_shift_in_bits);
    ASSERT_EQ(1, 65 & pointer_mask_in_bits);
}

TEST_F(YGCLiveMapTest, SetBits) {
    ASSERT_FALSE(live_map_get(&map_, 0));
    ASSERT_FALSE(live_map_get(&map_, 1));
    ASSERT_FALSE(live_map_get(&map_, 3));
    ASSERT_FALSE(live_map_get(&map_, 255));

    live_map_set(&map_, 0);
    live_map_set(&map_, 1);
    live_map_set(&map_, 3);
    live_map_set(&map_, 255);

    ASSERT_TRUE(live_map_get(&map_, 0));
    ASSERT_TRUE(live_map_get(&map_, 1));
    ASSERT_TRUE(live_map_get(&map_, 3));
    ASSERT_TRUE(live_map_get(&map_, 255));
}