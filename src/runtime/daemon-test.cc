#include "runtime/daemon.h"
#include "gtest/gtest.h"
#include <thread>

class DaemonTest : public ::testing::Test {
public:
};

TEST_F(DaemonTest, Sanity) {
    task_queue queue{};
    task_queue_init(&queue);

    std::thread worker([&queue]{
        auto task = task_new(&queue, nullptr, nullptr, nullptr);
        task->param1 = 996;
        task_queue_post(&queue, task);
    });

    task_entry *task = nullptr;
    ASSERT_EQ(1, task_queue_take(&queue, &task));
    ASSERT_TRUE(task != nullptr);
    ASSERT_EQ(996, task->param1);
    task_final(&queue, task);

    worker.join();

    task_queue_final(&queue);
}

TEST_F(DaemonTest, MoreTasksPost) {
    static constexpr int N = 2000;

    task_queue queue{};
    task_queue_init(&queue);

    std::thread worker([&queue]{
        for (int i = 0; i < N; i++) {
            auto task = task_new(&queue, nullptr, nullptr, nullptr);
            task->param1 = 996;
            task->param2 = i;
            task_queue_post(&queue, task);
        }
    });

    task_entry *task = nullptr;
    for (int i = 0; i < N; i++) {
        ASSERT_EQ(1, task_queue_take(&queue, &task));
        ASSERT_TRUE(task != nullptr);
        ASSERT_EQ(996, task->param1);
        ASSERT_EQ(i, task->param2);
        task_final(&queue, task);
        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
    }

    worker.join();

    task_queue_final(&queue);
}