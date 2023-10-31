#include "runtime/heap/ygc.h"
#include <gtest/gtest.h>

class YGCPlatformTest : public ::testing::Test {

};

TEST_F(YGCPlatformTest, Sanity) {
    memory_backing backing{};
    ASSERT_EQ(0, memory_backing_init(&backing, 512 * MB));

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

    memory_backing_unmap(&backing, ygc_marked0(addr), SMALL_PAGE_SIZE);
    memory_backing_unmap(&backing, ygc_marked1(addr), SMALL_PAGE_SIZE);
    memory_backing_unmap(&backing, ygc_remapped(addr), SMALL_PAGE_SIZE);
    memory_backing_final(&backing);
}

TEST_F(YGCPlatformTest, PhysicalAddress) {
    physical_memory_management pmm{};
    physical_memory_management_init(&pmm, 0, 4 * GB);

    physical_memory mem{};
    ASSERT_EQ(0, allocate_physical_memory(&pmm, &mem, SMALL_PAGE_SIZE));

    ASSERT_EQ(1, mem.n_segments);
    ASSERT_EQ(SMALL_PAGE_SIZE, mem.size_in_bytes);
    memory_segment *segment = mem.segments.next;
    ASSERT_EQ(0, segment->addr);
    ASSERT_EQ(SMALL_PAGE_SIZE, segment->size);

    free_physical_memory(&pmm, &mem);
    ASSERT_EQ(pmm.capacity, pmm.free.next->size);

    physical_memory_management_final(&pmm);
}

TEST_F(YGCPlatformTest, PhysicalAddressAllocate) {
    physical_memory_management pmm{};
    physical_memory_management_init(&pmm, 0, 4 * GB);

    physical_memory mem[3];
    for (int i = 0; i < arraysize(mem); i++) {
        ASSERT_EQ(0, allocate_physical_memory(&pmm, &mem[i], SMALL_PAGE_SIZE));
    }
    ASSERT_EQ(pmm.unused_in_bytes, pmm.capacity - SMALL_PAGE_SIZE * 3);
    free_physical_memory(&pmm, &mem[1]);
    //debug_physical_memory_management(&pmm);
    auto chunk = pmm.free.next;
    ASSERT_EQ(0x200000, chunk->addr);
    ASSERT_EQ(SMALL_PAGE_SIZE, chunk->size);

    chunk = chunk->next;
    ASSERT_EQ(0x600000, chunk->addr);

    ASSERT_EQ(&pmm.free, chunk->next);

    physical_memory_management_final(&pmm);
}

TEST_F(YGCPlatformTest, VirtualAddress) {
    virtual_memory_management vmm{};
    ASSERT_EQ(0, virtual_memory_management_init(&vmm, 512 * MB));
    
    memory_backing backing{};
    ASSERT_EQ(0, memory_backing_init(&backing, 512 * MB));

    auto mem = allocate_virtual_memory(&vmm, SMALL_PAGE_SIZE, 0);
    //ASSERT_EQ(0, mem.addr);
    ASSERT_EQ(SMALL_PAGE_SIZE, mem.size);

    memory_backing_map(&backing, ygc_marked0(mem.addr), SMALL_PAGE_SIZE, 0);
    memory_backing_map(&backing, ygc_marked1(mem.addr), SMALL_PAGE_SIZE, 0);
    memory_backing_map(&backing, ygc_remapped(mem.addr), SMALL_PAGE_SIZE, 0);


    memory_backing_unmap(&backing, ygc_marked0(mem.addr), SMALL_PAGE_SIZE);
    memory_backing_unmap(&backing, ygc_marked1(mem.addr), SMALL_PAGE_SIZE);
    memory_backing_unmap(&backing, ygc_remapped(mem.addr), SMALL_PAGE_SIZE);
    virtual_memory_management_final(&vmm);
}

