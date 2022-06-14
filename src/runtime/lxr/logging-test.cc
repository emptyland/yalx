#include "runtime/lxr/logging.h"
#include "runtime/lxr/block.h"
#include "base/base.h"
#include <gtest/gtest.h>
#include <thread>

class LxrLoggingTest : public ::testing::Test {
public:
    static constexpr size_t kDefaultAligment = 8;
    
    void SetUp() override {
        block_ = lxr_new_normal_block(NULL);
        ASSERT_NE(nullptr, block_);
        lxr_init_fields_logger(&logger_);
    }
    
    void TearDown() override {
        lxr_free_fields_logger(&logger_);
        ASSERT_NE(nullptr, block_);
        lxr_delete_block(block_);
    }
    
protected:
    lxr_block_header *block_;
    struct lxr_fields_logger logger_;
};

struct Dummy {
    void *a;
    void *b;
    int c;
};

TEST_F(LxrLoggingTest, Sanity) {
    ASSERT_FALSE(lxr_has_logged(&logger_, NULL));
    
    auto *ob = static_cast<Dummy *>(lxr_block_allocate(block_, sizeof(Dummy), kDefaultAligment));
    auto ok = lxr_attempt_to_log(&logger_, static_cast<void *>(&ob->a));
    ASSERT_TRUE(ok);
    ASSERT_TRUE(lxr_has_logged(&logger_, static_cast<void *>(&ob->a)));
    ASSERT_FALSE(lxr_attempt_to_log(&logger_, static_cast<void *>(&ob->a)));
    
    //printf("%zd\n", logger_.used_memory_in_bytes);
}

TEST_F(LxrLoggingTest, FuzzToLog) {
    uintptr_t base = reinterpret_cast<uintptr_t>(block_);
    for (int i = 0; i < 100000; i++) {
        auto mb = rand() % 1000;
        auto offset = yalx::RoundUp(rand() % 1048576, 4);
        auto addr = base + mb * 1048576 + offset;
        lxr_attempt_to_log(&logger_, reinterpret_cast<void *>(addr));
        ASSERT_TRUE(lxr_has_logged(&logger_, reinterpret_cast<void *>(addr)));
    }
    //printf("%zd\n", logger_.used_memory_in_bytes);
}

TEST_F(LxrLoggingTest, ThreadSafeSanity) {
    const uintptr_t base = reinterpret_cast<uintptr_t>(block_);
    std::thread workers[5];
    std::atomic<int> succ(0);
    for (int i = 0; i < arraysize(workers); i++) {
        workers[i] = std::move(std::thread([&succ, base, this](){
            for (int i = 0; i < 4000000; i+=4) {
                auto ok = lxr_attempt_to_log(&logger_, reinterpret_cast<void *>(base + i));
                if (ok) {
                    succ.fetch_add(1);
                }
            }
        }));
    }

    for (int i = 0; i < arraysize(workers); i++) {
        workers[i].join();
    }
    ASSERT_EQ(1000000, succ.load(std::__1::memory_order_relaxed));
}
