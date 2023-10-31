#include "runtime/heap/ygc.h"
#include "runtime/heap/ygc-forwarding.h"
#include "runtime/heap/heap.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/arrays.h"
#include "runtime/object/type.h"
#include "runtime/root-handles.h"
#include <gtest/gtest.h>

class YGCHeapTest : public ::testing::Test {
public:
    void SetUp() override {
        ASSERT_EQ(0, yalx_init_heap(GC_YGC, 512 * MB, &heap_));
        thread_enter_ = heap->thread_enter;
        thread_exit_ = heap->thread_exit;

        heap->thread_enter = ygc_thread_enter;
        heap->thread_exit = ygc_thread_exit;
        ygc_thread_enter(heap_, &thread_);
    }

    void TearDown() override {
        //yalx_os_thread_detach_self();
        ygc_thread_exit(heap_, &thread_);

        heap->thread_enter = thread_enter_;
        heap->thread_exit = thread_exit_;
        yalx_free_heap(heap_);
        heap_ = nullptr;

        yalx_free_root_handles();
    }

    yalx_value_array *NewDummyArray(const int n) const {
        std::unique_ptr<yalx_value_str *[]> elems(new yalx_value_str*[n]);
        char buf[16];
        for (int i = 0; i < n; i++) {
            snprintf(buf, arraysize(buf), "%d", i);
            elems[i] = yalx_new_string_direct(heap_, buf, strlen(buf));
        }
        auto arr = yalx_new_refs_array_with_data(heap_, string_class, 1, nullptr,
                                                 reinterpret_cast<yalx_ref_t *>(elems.get()), n);
        return reinterpret_cast<yalx_value_array *>(arr);
    }

    static void VisitObject1(yalx_heap_visitor *v, yalx_ref_t o) {
        auto obj_in_page = static_cast<std::vector<yalx_ref_t> *>(v->ctx);
        obj_in_page->push_back(o);
    }

    void (*thread_enter_)(struct heap *, struct yalx_os_thread *);
    void (*thread_exit_)(struct heap *, struct yalx_os_thread *);
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

    yalx_value_str *elems[3] = {
            yalx_new_string_direct(heap_, "1", 1),
            yalx_new_string_direct(heap_, "2", 1),
            yalx_new_string_direct(heap_, "3", 1),
    };
    ASSERT_FALSE(ygc_is_marked(elems[0]));
    ASSERT_FALSE(ygc_is_marked(elems[1]));
    ASSERT_FALSE(ygc_is_marked(elems[2]));
    auto arr = yalx_new_refs_array_with_data(heap_, string_class, 1, nullptr,
                                             reinterpret_cast<yalx_ref_t *>(elems), 3);
    yalx_add_root_handle(reinterpret_cast<yalx_ref_t>(arr));

    ygc_mark_start(heap_);
    ygc_marking_tls_commit(&ygc->mark, yalx_os_thread_self());
    ygc_mark(heap_, 0);

    auto hello2 = yalx_new_string(heap_, "hello", 5);
    ASSERT_EQ(ygc_offset(hello1), ygc_offset(hello2));

    auto vals = reinterpret_cast<yalx_value_str **>(reinterpret_cast<yalx_value_array *>(arr)->data);
    ASSERT_TRUE(ygc_is_marked(vals[0]));
    ASSERT_TRUE(ygc_is_marked(vals[1]));
    ASSERT_TRUE(ygc_is_marked(vals[2]));
}

TEST_F(YGCHeapTest, SelectRelactionSetDefault) {
    auto ygc = ygc_heap_of(heap_);
    auto arr = NewDummyArray(5);
    yalx_add_root_handle(reinterpret_cast<yalx_ref_t>(arr));

    ygc_mark_start(heap_);
    ygc_marking_tls_commit(&ygc->mark, yalx_os_thread_self());
    ygc_mark(heap_, 0);
    ygc_reset_relocation_set(heap_);
    ygc_select_relocation_set(ygc);

    EXPECT_EQ(0, ygc->relocation_set.size);
}

TEST_F(YGCHeapTest, SelectRelactionSetAll) {
    auto ygc = ygc_heap_of(heap_);
    ygc->fragmentation_limit = -1;
    auto arr = NewDummyArray(5);
    yalx_add_root_handle(reinterpret_cast<yalx_ref_t>(arr));

    ygc_mark_start(heap_);
    ygc_marking_tls_commit(&ygc->mark, yalx_os_thread_self());
    ygc_mark(heap_, 0);
    ygc_reset_relocation_set(heap_);
    ygc_select_relocation_set(ygc);

    EXPECT_EQ(1, ygc->relocation_set.size);

    auto fwd = ygc->relocation_set.forwards[0];
    ASSERT_TRUE(fwd != nullptr);

    EXPECT_EQ(1, fwd->refs);
    EXPECT_EQ(0, fwd->pinned);

    EXPECT_TRUE(forwarding_grab_page(fwd));
    EXPECT_FALSE(forwarding_drop_page(fwd, ygc));
}

TEST_F(YGCHeapTest, RelocateStartSanity) {
    auto ygc = ygc_heap_of(heap_);
    ygc->fragmentation_limit = -1;
    auto arr = NewDummyArray(5);
    yalx_add_root_handle(reinterpret_cast<yalx_ref_t>(arr));

    ygc_mark_start(heap_);
    ygc_marking_tls_commit(&ygc->mark, yalx_os_thread_self());
    ygc_mark(heap_, 0);
    ygc_reset_relocation_set(heap_);
    ygc_select_relocation_set(ygc);
    ygc_relocate_start(heap_);

    ASSERT_EQ(YGC_METADATA_REMAPPED, YGC_ADDRESS_GOOD_MASK);
    ASSERT_EQ(YGC_PHASE_RELOCATE, ygc_global_phase);

    size_t n = 0;
    auto handles = yalx_get_root_handles(&n);
    ASSERT_EQ(1, n);
    EXPECT_NE(ygc_offset(arr), ygc_offset(handles[0]));

    arr = reinterpret_cast<yalx_value_array *>(handles[0]);
    ASSERT_EQ(5, arr->len);
    auto elems = reinterpret_cast<yalx_value_str **>(arr->data);
    for (int i = 0; i < arr->len; i++) {
        char buf[16];
        snprintf(buf, arraysize(buf), "%d", i);
        EXPECT_STREQ(buf, elems[i]->bytes);
    }
}

TEST_F(YGCHeapTest, ConcurrentRelocateSanity) {
    auto ygc = ygc_heap_of(heap_);
    ygc->fragmentation_limit = -1;
    auto arr = NewDummyArray(5);
    yalx_add_root_handle(reinterpret_cast<yalx_ref_t>(arr));

    ygc_mark_start(heap_);
    ygc_marking_tls_commit(&ygc->mark, yalx_os_thread_self());
    ygc_mark(heap_, 0);
    ygc_reset_relocation_set(heap_);
    ygc_select_relocation_set(ygc);
    ygc_relocate_start(heap_);
    ygc_relocate(heap_);
}