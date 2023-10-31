#include "runtime/heap/heap.h"
#include "runtime/heap/object-visitor.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/type.h"
#include <gtest/gtest.h>

class HeapTest : public ::testing::Test {
public:
    void SetUp() override {
        yalx_init_heap(GC_NONE, 512 * MB, &heap_);
        string_pool_init(&kpool_, 4);
    }
    
    void TearDown() override {
        string_pool_free(&kpool_);
        yalx_free_heap(heap_);
    }

    struct heap *heap_ = nullptr;
    struct string_pool kpool_;
};

TEST_F(HeapTest, Sanity) {
    auto space = string_pool_ensure_space(&kpool_, "a", 1);
    ASSERT_EQ(nullptr, space->value);
    
    auto str = yalx_new_string_direct(heap_, "a", 1);
    ASSERT_TRUE(nullptr != str);
    ASSERT_STREQ("a", str->bytes);
    
    space->value = str;
    space = string_pool_ensure_space(&kpool_, "a", 1);
    ASSERT_EQ(space->value, str);
}

TEST_F(HeapTest, StringPoolRehash) {
    ASSERT_EQ(4, kpool_.slots_shift);
    
    char buf[16];
    for (int i = 0; i < 32; i++) {
        //sprintf(buf, "%04x", i);
        snprintf(buf, arraysize(buf), "%04x", i);
        auto str = yalx_new_string_direct(heap_, buf, strlen(buf));
        auto space = string_pool_ensure_space(&kpool_, str->bytes, str->len);
        space->value = str;
    }
    
    ASSERT_EQ(6, kpool_.slots_shift);
}

namespace {

struct ObjectsCounter {
    yalx_root_visitor visitor;
    int n_bool = 0;
    int n_i8 = 0;
    int n_u8 = 0;
    int n_i16 = 0;
    int n_u16 = 0;
    int n_i32 = 0;
    int n_u32 = 0;
    int n_i64 = 0;
    int n_u64 = 0;
    int n_f32 = 0;
    int n_f64 = 0;
    int n_str = 0;
};

void VisitPointer(yalx_root_visitor *v, yalx_ref_t *p) {
    if (!*p) {
        return;
    }
    auto counter = reinterpret_cast<ObjectsCounter *>(v);
    switch (yalx_builtin_type(CLASS(*p))) {
        case Type_Bool:
            counter->n_bool++;
            break;
        case Type_I8:
            counter->n_i8++;
            break;
        case Type_U8:
            counter->n_u8++;
            break;
        case Type_I32:
            counter->n_i32++;
            break;
        case Type_F32:
            counter->n_f32++;
            break;
        case Type_F64:
            counter->n_f64++;
            break;
        case Type_string:
            counter->n_str++;
            break;
        // TODO:
        default:
            break;
    }
}

void VisitPointers(yalx_root_visitor *v, yalx_ref_t *begin, yalx_ref_t *end) {
    for (yalx_ref_t *i = begin; i < end; i++) {
        VisitPointer(v, i);
    }
}

} // namespace

TEST_F(HeapTest, RootVisitor) {
    ObjectsCounter counter;
    counter.visitor.visit_pointer = VisitPointer;
    counter.visitor.visit_pointers = VisitPointers;
    
    yalx_heap_visit_root(heap_, &counter.visitor);
    
    ASSERT_EQ(2, counter.n_bool);
    ASSERT_EQ(3, counter.n_f32);
    ASSERT_EQ(0, counter.n_str);
}
