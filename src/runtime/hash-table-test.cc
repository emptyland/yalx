#include "runtime/hash-table.h"
#include "gtest/gtest.h"


class HashTableTest : public ::testing::Test {
public:
    void SetUp() override {
        yalx_init_hash_table(&htable_, 0);
    }
    
    void TearDown() override {
        yalx_free_hash_table(&htable_);
    }
    
protected:
    hash_table htable_;
}; // class HashTableTest


TEST_F(HashTableTest, Sanity) {
    ASSERT_EQ(0, htable_.size);
    ASSERT_EQ(2, htable_.capacity_shift);
    ASSERT_NEAR(1.1, htable_.rehash_factor, 0.001);
}


TEST_F(HashTableTest, StringKey) {
    *static_cast<int *>(yalx_put_string_key(&htable_, "ok", 4).value) = 1;
    *static_cast<int *>(yalx_put_string_key(&htable_, "ok", 4).value) = 2;
    *static_cast<int *>(yalx_put_string_key(&htable_, "fail", 4).value) = -1;
    
    ASSERT_EQ(2, *static_cast<int *>(yalx_get_string_key(&htable_, "ok").value));
    ASSERT_EQ(-1, *static_cast<int *>(yalx_get_string_key(&htable_, "fail").value));
}
