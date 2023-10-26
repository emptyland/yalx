#include "runtime/heap/ygc.h"
#include "runtime/heap/heap.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/object/yalx-string.h"
#include <gtest/gtest.h>

class YGCHeapTest : public ::testing::Test {
public:
    void SetUp() override {
        ASSERT_EQ(0, yalx_init_heap(GC_YGC, 512 * MB, &heap_));

        //yalx_os_thread_attach_self(&thread_);
        ygc_thread_enter(heap_, &thread_);
    }

    void TearDown() override {
        //yalx_os_thread_detach_self();
        ygc_thread_exit(heap_, &thread_);

        yalx_free_heap(heap_);
        heap_ = nullptr;
    }

    static void VisitObject1(yalx_heap_visitor *v, yalx_ref_t o) {
        auto obj_in_page = static_cast<std::vector<yalx_ref_t> *>(v->ctx);
        obj_in_page->push_back(o);
    }

    struct heap *heap_ = nullptr;
    yalx_os_thread thread_{};
};

TEST_F(YGCHeapTest, Sanity) {
    auto str = yalx_new_string_direct(heap_, "hello", 5);
    ASSERT_STREQ("hello", str->bytes);

    ASSERT_TRUE(heap_->is_in(heap_, reinterpret_cast<uintptr_t>(str)));
    ASSERT_FALSE(heap_->is_in(heap_, 0));
}

TEST_F(YGCHeapTest, ObjectIteratingInPage) {
    auto hello = yalx_new_string_direct(heap_, "hello", 5);
    EXPECT_STREQ("hello", hello->bytes);
    auto world = yalx_new_string_direct(heap_, "world", 5);
    auto doom = yalx_new_string_direct(heap_, "doom", 4);
    auto ygc = ygc_heap_of(heap_);

    auto page = ygc_addr_in_page(ygc, reinterpret_cast<uintptr_t>(hello));
    ASSERT_EQ(page, ygc_addr_in_page(ygc, reinterpret_cast<uintptr_t>(world)));
    ASSERT_EQ(page, ygc_addr_in_page(ygc, reinterpret_cast<uintptr_t>(doom)));

    std::vector<yalx_value_str *> objs;
    yalx_heap_visitor closure {
            static_cast<void *>(&objs),
            0,
            0,
            &VisitObject1,
    };
    ygc_page_visit_objects(page, &closure);

    EXPECT_EQ(1729, objs.size());
    auto len = objs.size();

    EXPECT_EQ(hello, objs[len - 3]);
    EXPECT_EQ(world, objs[len - 2]);
    EXPECT_EQ(doom, objs[len - 1]);
}

TEST_F(YGCHeapTest, ObjectMarkingInPage) {
    auto hello = yalx_new_string_direct(heap_, "hello", 5);
    auto world = yalx_new_string_direct(heap_, "world", 5);
    auto doom = yalx_new_string_direct(heap_, "doom", 4);
    auto ygc = ygc_heap_of(heap_);

    auto page = ygc_addr_in_page(ygc, reinterpret_cast<uintptr_t>(hello));
    ASSERT_EQ(page, ygc_addr_in_page(ygc, reinterpret_cast<uintptr_t>(world)));
    ASSERT_EQ(page, ygc_addr_in_page(ygc, reinterpret_cast<uintptr_t>(doom)));

    ygc_page_mark_object(page, reinterpret_cast<yalx_value_any *>(world));

    EXPECT_EQ(1, page->live_map.live_objs);
    EXPECT_EQ(30, page->live_map.live_objs_in_bytes);

    std::vector<yalx_value_str *> objs;
    yalx_heap_visitor closure {
            static_cast<void *>(&objs),
            0,
            0,
            &VisitObject1,
    };
    live_map_visit_objects(&page->live_map, page, &closure);
    ASSERT_EQ(1, objs.size());
    EXPECT_EQ(world, objs[0]);
}

TEST_F(YGCHeapTest, MarkStartSanity) {
    ASSERT_EQ(YGC_METADATA_MARKED0, YGC_METADATA_MARKED);

    auto hello1 = yalx_new_string(heap_, "hello", 5);

    ygc_mark_start(heap_);
    ASSERT_EQ(2, ygc_global_tick);
    ASSERT_EQ(YGC_PHASE_MARK, ygc_global_phase);
    ASSERT_EQ(YGC_METADATA_MARKED1, YGC_METADATA_MARKED);

    auto hello2 = yalx_new_string(heap_, "hello", 5);
    ASSERT_EQ(ygc_offset(hello1), ygc_offset(hello2));
    ASSERT_TRUE(ygc_is_remapped(hello1));
    ASSERT_TRUE(ygc_is_marked(hello2));
    ASSERT_TRUE(ygc_is_bad(hello1));
    ASSERT_TRUE(ygc_is_good(hello2));
}

TEST_F(YGCHeapTest, ConcurrentMarkSanity) {
    auto ygc = ygc_heap_of(heap_);
    auto hello1 = yalx_new_string(heap_, "hello", 5);

    ygc_mark_start(heap_);
    ygc_marking_tls_commit(&ygc->mark, yalx_os_thread_self());

    ygc_mark(heap_, 0);

    auto hello2 = yalx_new_string(heap_, "hello", 5);
    ASSERT_EQ(ygc_offset(hello1), ygc_offset(hello2));
}
