#include "base/arena.h"
#include <gtest/gtest.h>

namespace yalx::base {

TEST(ArenaTest, Sanity) {
    Arena arean;
    ASSERT_TRUE(arean.Allocate(1) != nullptr);
    ASSERT_EQ(1, arean.CountBlocks());
    ASSERT_EQ(0, arean.CountLargeBlocks());
}

class Foo {
public:
    Foo(int a, int b): a_(a), b_(b) {}
    
    int a_;
    int b_;
};

TEST(ArenaTest, New) {
    Arena arean;
    auto foo = arean.New<Foo>(1, 2);
    ASSERT_EQ(1, foo->a_);
    ASSERT_EQ(2, foo->b_);
}

TEST(ArenaTest, MutilBlocks) {
    Arena arean;
    for (int i = 0; i < 1024; i++) {
        auto p = arean.Allocate(kKB);
        ASSERT_TRUE(nullptr != p);
    }
    ASSERT_EQ(2, arean.CountBlocks());
    ASSERT_EQ(0, arean.CountLargeBlocks());
}

TEST(ArenaTest, ContinuousAllocation) {
    Arena arean;
    auto x = static_cast<int *>(arean.Allocate(4));
    *x = 0xcccccccc;
    auto p = static_cast<int *>(arean.Allocate(4));
    *p = 0xeeeeeeee;
    ASSERT_EQ(*x, 0xcccccccc);
    ASSERT_EQ(*p, 0xeeeeeeee);
}

} // namespace yalx
