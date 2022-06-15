#include "runtime/lxr/immix-heap.h"
#include "runtime/lxr/block.h"
#include <base/base.h>
#include <gtest/gtest.h>
#include <thread>

class LxrImmixHeapTest : public ::testing::Test {
public:

    void SetUp() override {
        srand(time(nullptr));
        lxr_init_immix_heap(&heap_, 16);
        lxr_thread_enter(&heap_);
    }
    
    void TearDown() override {
        lxr_thread_exit(&heap_);
        lxr_free_immix_heap(&heap_);
    }
    
protected:
    lxr_immix_heap heap_;
};


TEST_F(LxrImmixHeapTest, Sanity) {
    auto chunk = lxr_allocate(&heap_, 16);
    ASSERT_NE(nullptr, chunk);
    
    auto rs = lxr_test_addr(&heap_, chunk);
    ASSERT_EQ(LXR_ADDR_CHUNK, rs.kind);
    ASSERT_NE(nullptr, rs.block.normal);
}

TEST_F(LxrImmixHeapTest, BatchAllocating) {
    static const int k = 1000000;
    for (int i = 0; i < k; i++) {
        auto chunk = lxr_allocate(&heap_, 16);
        ASSERT_NE(nullptr, chunk);
        
        auto rs = lxr_test_addr(&heap_, chunk);
        ASSERT_EQ(LXR_ADDR_CHUNK, rs.kind);
        ASSERT_NE(nullptr, rs.block.normal);
    }
}

TEST_F(LxrImmixHeapTest, LargeAllocating) {
    auto chunk = static_cast<int *>(lxr_allocate(&heap_, LXR_LARGE_BLOCK_THRESHOLD_SIZE * sizeof(int)));
    ASSERT_NE(nullptr, chunk);
    for (int i = 0; i < LXR_LARGE_BLOCK_THRESHOLD_SIZE; i++) {
        chunk[i] = i + 1;
    }

    auto rs = lxr_test_addr(&heap_, chunk);
    ASSERT_EQ(LXR_ADDR_LARGE, rs.kind);
    ASSERT_NE(nullptr, rs.block.large);
    
    lxr_free(&heap_, static_cast<void *>(chunk));
    rs = lxr_test_addr(&heap_, chunk);
    ASSERT_EQ(LXR_ADDR_INVALID, rs.kind);
    ASSERT_EQ(nullptr, rs.block.large);
}

TEST_F(LxrImmixHeapTest, FuzzAllocatingThenFree) {
    static const int k = 100000;
    std::vector<void *> chunks;
    size_t total = 0;
    for (int i = 0; i < k; i++) {
        auto size = 16 + std::abs(rand()) % 4096;
        total += size;
        auto chunk = lxr_allocate(&heap_, size);
        if (!chunk) {
            break;
        }
        chunks.push_back(chunk);
    }
    for (auto chunk : chunks) {
        auto rs = lxr_test_addr(&heap_, chunk);
        ASSERT_NE(LXR_ADDR_INVALID, rs.kind);
        ASSERT_NE(nullptr, rs.block.large);
        ASSERT_NE(nullptr, rs.block.normal);
        
        lxr_free(&heap_, chunk);
    }
    // Max = 4.14GB
    //printf("total: %0.2fGB\n", total / (1024.0 * 1024.0 * 1024.0));
}

TEST_F(LxrImmixHeapTest, AllocatingThreadSafe) {
    static const int k = 100000;
    std::thread workers[5];
    for (int i = 0; i < arraysize(workers); i++) {
        workers[i] = std::move(std::thread([this](int id) {
            lxr_thread_enter(&heap_);
            for (int i = 0; i < k; i++) {
                auto chunk = lxr_allocate(&heap_, 16);
                ASSERT_NE(nullptr, chunk);
                
                auto rs = lxr_test_addr(&heap_, chunk);
                ASSERT_EQ(LXR_ADDR_CHUNK, rs.kind);
                ASSERT_NE(nullptr, rs.block.normal);
            }
            lxr_thread_exit(&heap_);
        }, i));
    }
    
    for (int i = 0; i < arraysize(workers); i++) {
        workers[i].join();
    }
}
