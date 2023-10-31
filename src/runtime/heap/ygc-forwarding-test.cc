#include "runtime/heap/ygc-forwarding.h"
#include "runtime/heap/ygc.h"
#include "gtest/gtest.h"

class ForwardingTest : public ::testing::Test {
public:
    void SetUp() override {
        ASSERT_EQ(0, ygc_init(&ygc_, 512 * MB, 25));
        page_ = ygc_page_new(&ygc_, SMALL_PAGE_SIZE);
        ASSERT_TRUE(page_ != nullptr);
        live_map_increase_obj(&page_->live_map, 100, 1024);
        fwd_ = forwarding_new(page_);
        ASSERT_TRUE(fwd_ != nullptr);
    }

    void TearDown() override {
        forwarding_free(fwd_);
        ygc_final(&ygc_);
    }

    ygc_core ygc_{};
    ygc_page *page_ = nullptr;
    forwarding *fwd_ = nullptr;
};

TEST_F(ForwardingTest, Sanity) {
    ASSERT_EQ(256, fwd_->n_entries);
    size_t pos;
    auto entry = forwarding_find(fwd_, 0, &pos);
    ASSERT_FALSE(entry.populated);
    ASSERT_EQ(996, forwarding_insert(fwd_, 0, 996, &pos));
    entry = forwarding_find(fwd_, 0, &pos);
    ASSERT_TRUE(entry.populated);
    ASSERT_EQ(0, entry.from_index);
    ASSERT_EQ(996, entry.to_offset);
}

TEST_F(ForwardingTest, MoreInserting) {
    const size_t N = fwd_->n_entries / 2;
    size_t pos;
    for (int i = 0; i < N; i++) {
        auto entry = forwarding_find(fwd_, i, &pos);
        if (entry.populated) {
            continue;
        }
        ASSERT_EQ(10000 + i, forwarding_insert(fwd_, i, 10000 + i, &pos));
    }

    for (size_t i = 0; i < N; i++) {
        auto entry = forwarding_find(fwd_, i, &pos);
        ASSERT_TRUE(entry.populated);
        ASSERT_EQ(i, entry.from_index);
        ASSERT_EQ(10000 + i, entry.to_offset);
    }
}
