#include "runtime/heap/ygc.h"
#include <gtest/gtest.h>

class YGCTest : public ::testing::Test {
public:
    void SetUp() override {
        ASSERT_EQ(0, ygc_init(&ygc_, 4 * GB));
    }

    void TearDown() override {
        ygc_final(&ygc_);
    }

    ygc_core ygc_{};
};

TEST_F(YGCTest, Sanity) {
    ASSERT_EQ(0x0000100000000000, YGC_METADATA_MARKED0);
    ASSERT_EQ(0x0000200000000000, YGC_METADATA_MARKED1);
    ASSERT_EQ(0x0000400000000000, YGC_METADATA_REMAPPED);
    ASSERT_EQ(YGC_METADATA_MARKED0 | YGC_METADATA_MARKED1 | YGC_METADATA_REMAPPED, YGC_METADATA_MASK);

    ygc_flip_to_remapped();
    ASSERT_EQ(YGC_METADATA_REMAPPED, YGC_METADATA_REMAPPED);
    ASSERT_EQ(YGC_METADATA_REMAPPED, YGC_ADDRESS_GOOD_MASK);
}

TEST_F(YGCTest, PageNew) {
    auto page = ygc_page_new(&ygc_, SMALL_PAGE_SIZE);
    ASSERT_TRUE(page != nullptr);
    ASSERT_EQ(SMALL_PAGE_SIZE, page->virtual_addr.size);

    page = ygc_page_new(&ygc_, MEDIUM_PAGE_SIZE);
    ASSERT_TRUE(page != nullptr);
    ASSERT_EQ(MEDIUM_PAGE_SIZE, page->virtual_addr.size);
}

TEST_F(YGCTest, PageAllocate) {
    auto page = ygc_page_new(&ygc_, SMALL_PAGE_SIZE);
    ASSERT_TRUE(page != nullptr);
    ASSERT_EQ(SMALL_PAGE_SIZE, page->virtual_addr.size);

    auto addr = ygc_page_allocate(page, 4);
    auto ptr = reinterpret_cast<int *>(ygc_good_address(addr));
    *ptr = 996;

    ASSERT_EQ(4, ygc_page_used_in_bytes(page));

    addr = ygc_page_atomic_allocate(page, 4);
    ptr = reinterpret_cast<int *>(ygc_good_address(addr));
    *ptr = 700;

    ASSERT_EQ(8, ygc_page_used_in_bytes(page));

    ygc_page_free(&ygc_, page);
}