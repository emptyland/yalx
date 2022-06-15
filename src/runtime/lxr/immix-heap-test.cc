#include "runtime/lxr/immix-heap.h"
#include <base/base.h>
#include <gtest/gtest.h>
#include <thread>

class LxrImmixHeapTest : public ::testing::Test {
public:

    void SetUp() override {
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

TEST_F(LxrImmixHeapTest, AllocatingThreadSafe) {
    static const int k = 1000000;
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
