#include "runtime/heap/ygc.h"
#include "runtime/thread.h"
#include <gtest/gtest.h>
#include <thread>

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

    auto addr = ygc_page_allocate(page, 4, 4);
    auto ptr = reinterpret_cast<int *>(ygc_good_address(addr));
    *ptr = 996;

    ASSERT_EQ(4, ygc_page_used_in_bytes(page));

    addr = ygc_page_atomic_allocate(page, 4, 4);
    ptr = reinterpret_cast<int *>(ygc_good_address(addr));
    *ptr = 700;

    ASSERT_EQ(8, ygc_page_used_in_bytes(page));

    ygc_page_free(&ygc_, page);
}

TEST_F(YGCTest, AllocateSmallObject) {
    ASSERT_TRUE(ygc_.medium_page == nullptr);
    ASSERT_TRUE(per_cpu_get(ygc_page*, ygc_.small_page) == nullptr);

    auto ptr = reinterpret_cast<int *>(ygc_allocate_object(&ygc_, 4, 4));
    *ptr = 996;
    ASSERT_EQ(996, *ptr);
    ASSERT_EQ(0, reinterpret_cast<uintptr_t>(ptr) % 4);

    auto page = per_cpu_get(ygc_page *, ygc_.small_page);
    ASSERT_TRUE(page != nullptr);
    ASSERT_EQ(4, ygc_page_used_in_bytes(page));

    ptr = reinterpret_cast<int *>(ygc_allocate_object(&ygc_, 4, 8));
    *ptr = 770;
    ASSERT_EQ(770, *ptr);
    ASSERT_EQ(0, reinterpret_cast<uintptr_t>(ptr) % 8);

    for (auto i = 0; i < 16384; i++) {
        ptr = reinterpret_cast<int *>(ygc_allocate_object(&ygc_, 128, 8));
        *ptr = i;
        ASSERT_EQ(i, *ptr);
        ASSERT_EQ(0, reinterpret_cast<uintptr_t>(ptr) % 8);
    }

    ASSERT_NE(page, per_cpu_get(ygc_page *, ygc_.small_page));
}

TEST_F(YGCTest, MediumObjectAllocate) {
    ASSERT_TRUE(ygc_.medium_page == nullptr);
    ASSERT_TRUE(per_cpu_get(ygc_page*, ygc_.small_page) == nullptr);

    ASSERT_EQ(4194304, MEDIUM_OBJECT_SIZE_LIMIT);

    auto ptr = reinterpret_cast<int *>(ygc_allocate_object(&ygc_, 4194300, 4));
    *ptr = 996;
    ASSERT_EQ(996, *ptr);
    ASSERT_EQ(0, reinterpret_cast<uintptr_t>(ptr) % 4);

    ASSERT_TRUE(ygc_.medium_page != nullptr);
    ASSERT_TRUE(per_cpu_get(ygc_page*, ygc_.small_page) == nullptr);
}

TEST_F(YGCTest, LargeObjectAllocate) {
    ASSERT_TRUE(ygc_.medium_page == nullptr);
    ASSERT_TRUE(per_cpu_get(ygc_page*, ygc_.small_page) == nullptr);

    auto ptr = reinterpret_cast<int *>(ygc_allocate_object(&ygc_, 4194304, 4));
    ASSERT_TRUE(ptr != nullptr);
    *ptr = 996;
    ASSERT_EQ(996, *ptr);
    ASSERT_EQ(0, reinterpret_cast<uintptr_t>(ptr) % 4);

    auto page = ygc_addr_in_page(&ygc_, reinterpret_cast<uintptr_t>(ptr));
    ASSERT_TRUE(page != nullptr);
    ASSERT_TRUE(ygc_is_large_page(page));

    ASSERT_TRUE(ygc_.medium_page == nullptr);
    ASSERT_TRUE(per_cpu_get(ygc_page*, ygc_.small_page) == nullptr);
}

TEST_F(YGCTest, ThreadingSafeSmallAllocation) {
    std::thread workers[8];

    for (auto & worker : workers) {
        worker = std::thread([this] {
            for (auto j = 0; j < 100000; j++) {
                auto ptr = reinterpret_cast<int *>(ygc_allocate_object(&ygc_, 32, 8));
                *ptr = j;
                ASSERT_EQ(j, *ptr);
                ASSERT_EQ(0, reinterpret_cast<uintptr_t>(ptr) % 8);
                ASSERT_TRUE(ygc_addr_in_heap(&ygc_, reinterpret_cast<uintptr_t>(ptr)));
            }
        });
    }

    for (auto & worker : workers) {
        worker.join();
    }
    ASSERT_GE(ygc_.rss, 20971520);
}
