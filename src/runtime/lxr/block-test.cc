#include "runtime/lxr/block.h"
#include <gtest/gtest.h>

class LxrBlockTest : public ::testing::Test {
public:
    static constexpr size_t kDefaultAligment = 8;
    
    void SetUp() override {
        block_ = lxr_new_normal_block(NULL);
        ASSERT_NE(nullptr, block_);
    }
    
    void TearDown() override {
        ASSERT_NE(nullptr, block_);
        lxr_delete_block(block_);
    }
    
protected:
    lxr_block_header *block_;
};

int lxr_log2(size_t n) {
    int j = 0;
    for (size_t i = 1; i < n; i <<= 1) {
        j++;
    }
    return j;
}

TEST_F(LxrBlockTest, Log2) {
    ASSERT_EQ(1, lxr_log2(2));
    ASSERT_EQ(2, lxr_log2(4));
    ASSERT_EQ(3, lxr_log2(8));
    ASSERT_EQ(4, lxr_log2(16));
    ASSERT_EQ(4, lxr_log2(15));
}

TEST_F(LxrBlockTest, Sanity) {
    ASSERT_EQ(block_, block_->next);
    ASSERT_EQ(block_, block_->prev);
    
    auto p = lxr_block_allocate(block_, 1, kDefaultAligment);
    // 257 = 0x300000
    ASSERT_EQ(0x900000, block_->bitmap[257]);
    lxr_block_free(block_, p);
    ASSERT_EQ(0, block_->bitmap[257]);
    ASSERT_NE(nullptr, p);
}

TEST_F(LxrBlockTest, Allocation0) {
    auto p1 = lxr_block_allocate(block_, 1, kDefaultAligment);
    auto p2 = lxr_block_allocate(block_, 1, kDefaultAligment);
    
    ASSERT_EQ(0x9900000, block_->bitmap[257]);
    ASSERT_EQ(block_, lxr_owns_block(p1));
    ASSERT_EQ(block_, lxr_owns_block(p2));
    
    lxr_block_free(block_, p1);
    lxr_block_free(block_, p2);
    ASSERT_EQ(0, block_->bitmap[257]);
}

TEST_F(LxrBlockTest, Allocation1) {
    auto p1 = lxr_block_allocate(block_, 16, kDefaultAligment);
    auto p2 = lxr_block_allocate(block_, 32, kDefaultAligment);
    auto p3 = lxr_block_allocate(block_, 64, kDefaultAligment);
    
    ASSERT_EQ(16, lxr_block_marked_size(block_, p1));
    ASSERT_EQ(32, lxr_block_marked_size(block_, p2));
    ASSERT_EQ(64, lxr_block_marked_size(block_, p3));
    //ASSERT_EQ(0xf00000, block_->bitmap[257]);
    
    lxr_block_free(block_, p1);
    lxr_block_free(block_, p2);
    lxr_block_free(block_, p3);
}

TEST_F(LxrBlockTest, AllocatingWithCache) {
    auto p1 = lxr_block_allocate(block_, 1, kDefaultAligment);
    auto p2 = lxr_block_allocate(block_, 1, kDefaultAligment);
    
    lxr_block_free(block_, p1);
    lxr_block_free(block_, p2);
}

