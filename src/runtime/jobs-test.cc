#include "runtime/jobs.h"
#include "runtime/runtime.h"
#include "gtest/gtest.h"

class JobsTest : public ::testing::Test {
public:
    void SetUp() override {
        yalx_job_init(&job_, "test", ncpus);
    }

    void TearDown() override {
        yalx_job_final(&job_);
    }

    static void ConcurrentAdd(yalx_worker *worker) {
        auto arr = static_cast<int *>(worker->ctx);
        for (int i = 0; i < 100000; i++) {
            arr[worker->id]++;
        }
        //printf("done: %d\n", worker->id);
    }

    yalx_job job_{};
};

TEST_F(JobsTest, Sanity) {
    int *arr = new int[ncpus];
    memset(arr, 0, sizeof(int) * ncpus);
    yalx_job_submit(&job_, ConcurrentAdd, static_cast<void *>(arr));
    for (int i = 0; i < ncpus; i++) {
        ASSERT_EQ(100000, arr[i]);
    }
}