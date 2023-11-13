#include "runtime/mm-thread.h"
#include "runtime/process.h"
#include "runtime/runtime.h"
#include "gtest/gtest.h"
#include <thread>

extern "C" void fast_poll_page();

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

    static void TestRun2(void *ctx) {
        auto *const mm = static_cast<yalx_mm_thread *>(ctx);
        ASSERT_GE(mm_synchronize_poll(mm), 11);

        for (int i = 0; i < 1000; i++) {
            ASSERT_EQ(0, mm_synchronize_poll(mm));
        }
        printf("[Polling] processor end: %d\n", thread_local_mach->owns->id.value);
    }

    static void TestRun3(void */*ctx*/) {
        fast_poll_page();
        printf("[Polling]\n");
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

TEST_F(MMThreadTest, SafePointSynchronizePolling) {
    ASSERT_EQ(0, mm_synchronize_poll(&thread_));
}

TEST_F(MMThreadTest, SafePointSynchronizeSanity) {
    thread_local_mach->state = MACH_IDLE;

    mm_synchronize_begin(&thread_);

    ASSERT_EQ(SYNCHRONIZED, thread_.state);
    ASSERT_EQ(1, thread_.safepoint_counter);

    ASSERT_EQ(thread_.safepoint_counter, thread_.wait_barrier.barrier_tag);
    ASSERT_EQ(0, thread_.wait_barrier.waiters);

    mm_synchronize_end(&thread_);

    thread_local_mach->state = MACH_RUNNING;
}

TEST_F(MMThreadTest, SafePointSynchronizeWaiting) {
    thread_local_mach->state = MACH_IDLE;
    mm_synchronize_begin(&thread_);

    std::thread workers[4];
    for (size_t i = 0; i < 4; i++) {
        workers[i] = std::thread([this](){
            auto mills = mm_synchronize_poll(&thread_);
            ASSERT_GT(mills, 11);
            //printf("polling time: %f ms\n", mills);
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(11));

    mm_synchronize_end(&thread_);
    thread_local_mach->state = MACH_RUNNING;

    for (size_t i = 0; i < 4; i++) {
        workers[i].join();
    }
}

TEST_F(MMThreadTest, SafePointSynchronizeWaiting2) {
    thread_local_mach->state = MACH_IDLE;
    mm_synchronize_begin(&thread_);

    std::unique_ptr<machine[]> workers(new machine[nprocs]);
    for (int i = 0; i < nprocs; i++) {
        yalx_init_machine(&workers[i]);
        yalx_add_machine_to_processor(&procs[i], &workers[i]);
        yalx_mach_run_dummy(&workers[i], TestRun2, &thread_);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(11));

    mm_synchronize_end(&thread_);
    thread_local_mach->state = MACH_RUNNING;

    for (size_t i = 0; i < nprocs; i++) {
        // yalx_os_thread_join(&workers[i].thread, 0);
        yalx_mach_join(&workers[i]);
    }
}

TEST_F(MMThreadTest, SafePointSynchronizeRaisePollingPageException) {
    printf("offset of machine::saved_exception_pc=%zd", offsetof(machine, saved_exception_pc));

    thread_local_mach->state = MACH_IDLE;
    mm_synchronize_begin(&mm_thread);

    std::unique_ptr<machine[]> workers(new machine[nprocs]);
    for (int i = 0; i < 1; i++) {
        yalx_init_machine(&workers[i]);
        yalx_add_machine_to_processor(&procs[i], &workers[i]);
        yalx_mach_run_dummy(&workers[i], TestRun3, &mm_thread);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(11));

    puts("mm_synchronize_end()...after");
    mm_synchronize_end(&mm_thread);
    thread_local_mach->state = MACH_RUNNING;
    puts("mm_synchronize_end()...before");

    for (size_t i = 0; i < 1; i++) {
        yalx_mach_join(&workers[i]);
    }
}