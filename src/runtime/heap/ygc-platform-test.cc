#include "runtime/heap/ygc.h"
#include "runtime/heap/heap.h"
#include <gtest/gtest.h>

class YGCPlatformTest : public ::testing::Test {

};

TEST_F(YGCPlatformTest, Sanity) {
    memory_backing backing{};
    ASSERT_EQ(0, memory_backing_init(&backing, 4 * GB));

    uintptr_t addr = 0;
    memory_backing_map(&backing, ygc_marked0(addr), SMALL_PAGE_SIZE, addr);
    memory_backing_map(&backing, ygc_marked1(addr), SMALL_PAGE_SIZE, addr);
    memory_backing_map(&backing, ygc_remapped(addr), SMALL_PAGE_SIZE, addr);

    int *ptr2 = reinterpret_cast<int *>(ygc_marked0(addr));
    int *ptr1 = reinterpret_cast<int *>(ygc_marked1(addr));
    int *ptr0 = reinterpret_cast<int *>(ygc_remapped(addr));

    ptr0[0] = 999;
    ptr0[1] = 996;
    ptr0[2] = 700;

    ASSERT_EQ(ptr1[0], 999);
    ASSERT_EQ(ptr2[1], 996);
    ASSERT_EQ(ptr2[2], 700);

    memory_backing_final(&backing);
}