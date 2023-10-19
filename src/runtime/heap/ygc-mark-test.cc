#include "runtime/heap/ygc-mark.h"
#include "runtime/heap/ygc.h"
#include "runtime/runtime.h"
#include "gtest/gtest.h"

class YGCMarkTest : public ::testing::Test {
public:
    void SetUp() override {
        ygc_mark_init(&mark_);
    }

    void TearDown() override {
        ygc_mark_final(&mark_);
    }

    static void MarkingWorker1(yalx_worker *worker) {
        static constexpr size_t N = 10000;

        auto *const mark = static_cast<struct ygc_mark *>(worker->ctx);
        for (int i = 0; i < N; i++) {
            auto ptr = ((worker->id * N) + i) << pointer_shift_in_bytes;
            auto obj = ygc_marked0(ptr);
            ygc_marking_mark_object(mark, obj);
        }
    }

    struct ygc_mark mark_{};
};

TEST_F(YGCMarkTest, Sanity) {
    static constexpr size_t N = 10000;
    for (int i = 0; i < N; i++) {
        ygc_marking_mark_object(&mark_, ygc_marked0(i << pointer_shift_in_bytes));
    }
    const ygc_marking_stripe *const stripe = &mark_.stripes[0];
    EXPECT_EQ(N, stripe->top);
    EXPECT_EQ(16384, stripe->max);

    for (int i = 0; i < N; i++) {
        auto ptr = ygc_marked0(i << pointer_shift_in_bytes);
        EXPECT_EQ(ptr, stripe->stack[i]);
    }
}

TEST_F(YGCMarkTest, ThreadingSafe) {
    yalx_job job{};
    yalx_job_init(&job, "ygc-mark-test", ncpus);

    yalx_job_submit(&job, &MarkingWorker1, &mark_);

    yalx_job_final(&job);

    const ygc_marking_stripe *const stripe = &mark_.stripes[0];
    EXPECT_EQ(ncpus * 10000, stripe->top);
}

//template <class T>
//class CC
//{
//};
//
//template <int d, class N>
//class CB
//{
//public:
//    typedef CC<CB<d, N>> *pointer_type;
//};
//
//template <class X>
//class CA
//{
//public:
//    template <int d>
//    static CA FromCB(const typename CB<d, X>::pointer_type x) {
//        printf("dim: %d, value: %p\n", d, x);
//        return CA{};
//    }
//};
//
//
//TEST_F(YGCMarkTest, CtorDemo) {
//    CB<2, double>::pointer_type v = nullptr;
//    CA<double> t = CA<double>::FromCB<2>(v);
//}