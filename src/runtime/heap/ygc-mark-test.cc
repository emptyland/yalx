#include "runtime/heap/ygc-mark.h"
#include "runtime/heap/ygc.h"
#include "runtime/runtime.h"
#include "gtest/gtest.h"

class YGCMarkTest : public ::testing::Test {
public:
    void SetUp() override {
        ygc_mark_init(&mark_, nullptr);
    }

    void TearDown() override {
        ygc_mark_final(&mark_);
    }

    static void MarkingWorker1(yalx_worker *worker) {
        static constexpr size_t N = 10000;

        auto *const mark = static_cast<struct ygc_mark *>(worker->ctx);
        MarkingThreadScope scope(mark);
        for (int i = 0; i < N; i++) {
            auto ptr = ((worker->id * N) + i) << pointer_shift_in_bytes;
            auto obj = ygc_marked0(ptr);
            ygc_marking_mark_object(mark, obj);
        }
    }

    class MarkingThreadScope {
    public:
        explicit MarkingThreadScope(struct ygc_mark *mark)
                : mark_(mark) {
            auto *const thread = yalx_os_thread_self();
            auto tls = reinterpret_cast<ygc_tls_struct *>(&thread->gc_data[0]);
            for (int i = 0; i < YGC_MAX_MARKING_STRIPES; i++) {
                tls->stacks[i] = nullptr;
            }
        }
        ~MarkingThreadScope() {
            ygc_marking_tls_commit(mark_, yalx_os_thread_self());
        }
    private:
        struct ygc_mark *mark_;
    };

    struct ygc_mark mark_{};
};


TEST_F(YGCMarkTest, ThreadingSafe) {
    yalx_job job{};
    yalx_job_init(&job, "ygc-mark-test", ncpus);

    yalx_job_submit(&job, &MarkingWorker1, &mark_);

    yalx_job_final(&job);

    const ygc_marking_stripe *const stripe = &mark_.stripes[0];
    EXPECT_EQ(ncpus, stripe->n_stacks);

    for (auto x = stripe->committed_stacks; x != nullptr; x = x->next) {
        EXPECT_EQ(10000, x->top);
        EXPECT_EQ(16384, x->max);
    }
}
