#include "runtime/heap/ygc.h"
#include "runtime/heap/heap.h"
#include <gtest/gtest.h>

class YGCPlatformTest : public ::testing::Test {

};

TEST_F(YGCPlatformTest, Sanity) {
    auto page = allocate_os_page(SMALL_PAGE_SIZE);
    ASSERT_TRUE(page != nullptr);

    ASSERT_EQ(SMALL_PAGE_SIZE, page->size);
    ASSERT_EQ(page, page->prev);
    ASSERT_EQ(page, page->next);
    ASSERT_TRUE(page->addr != nullptr);

    free_os_page(page);
}