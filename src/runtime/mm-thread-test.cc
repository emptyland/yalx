#include "runtime/mm-thread.h"
#include <gtest/gtest.h>

class MMThreadTest : public ::testing::Test {
public:
    void SetUp() override {
        ASSERT_EQ(0, yalx_mm_thread_start(&thread_));
    }

    void TearDown() override {
        if (!mm_thread_is_shutting_down(&thread_)) {
            yalx_mm_thread_shutdown(&thread_);
        }
    }

    static void TestRun1(task_entry *task) {
        auto ptr = static_cast<char **>(task->ctx);
        *ptr = strdup("Hello");

        auto thread = mm_thread_of_task(task);
        //ASSERT_TRUE(!mm_thread_is_shutting_down(thread));
        ASSERT_EQ(yalx_os_thread_self(), &thread->thread);
    }

    yalx_mm_thread thread_{};
};

TEST_F(MMThreadTest, Sanity) {
    static char *value = nullptr;
    mm_thread_post_routine_to(&thread_, &TestRun1, static_cast<void *>(&value));
    yalx_mm_thread_shutdown(&thread_);
    ASSERT_TRUE(value != nullptr);
    ASSERT_STREQ("Hello", value);
    free(value);
}