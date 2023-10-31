#include "runtime/object/number.h"
#include "runtime/object/type.h"
#include "runtime/runtime.h"
#include "runtime/heap/heap.h"
#include <gtest/gtest.h>

TEST(NumberTest, BoolBoxing) {
    // heap.fast_boxing_numbers.bool_values[0]
    auto l = yalx_bool_value(true);
    ASSERT_TRUE(nullptr != l);
    ASSERT_TRUE(l->box.i32);
    
    l = yalx_bool_value(false);
    ASSERT_TRUE(nullptr != l);
    ASSERT_FALSE(l->box.i32);
    
    auto klass = CLASS(l);
    ASSERT_TRUE(nullptr != klass);
    ASSERT_STREQ("Bool", klass->name.z);
    
    ASSERT_EQ(yalx_bool_value(true), yalx_true_value());
    ASSERT_EQ(yalx_bool_value(false), yalx_false_value());
}

TEST(NumberTest, ByteBoxing) {
    for (int i = -128; i < 128; i++) {
        auto l = yalx_i8_value((i8_t)i);
        ASSERT_TRUE(nullptr != l);
        ASSERT_STREQ("I8", CLASS(l)->name.z);
        ASSERT_EQ(i, l->box.i8);
    }
    
    for (int i = 0; i < 256; i++) {
        auto l = yalx_u8_value((u8_t)i);
        ASSERT_TRUE(nullptr != l);
        ASSERT_STREQ("U8", CLASS(l)->name.z);
        ASSERT_EQ(i, l->box.u8);
    }
}


TEST(NumberTest, WordBoxing) {
    for (int i = -100; i < 100; i++) {
        auto l = yalx_new_i16(heap, (i16_t)i);
        ASSERT_TRUE(nullptr != l);
        ASSERT_STREQ("I16", CLASS(l)->name.z);
        ASSERT_EQ(i, l->box.i16);
    }
    
    for (int i = 0; i < 201; i++) {
        auto l = yalx_new_u16(heap, (u16_t)i);
        ASSERT_TRUE(nullptr != l);
        ASSERT_STREQ("U16", CLASS(l)->name.z);
        ASSERT_EQ(i, l->box.u16);
    }
    
    auto zero = yalx_new_i16(heap, 0);
    ASSERT_EQ(0, zero->box.i16);
    ASSERT_EQ(zero, yalx_new_i16(heap, 0));
    
    auto large = yalx_new_i16(heap, 10000);
    ASSERT_EQ(10000, large->box.i16);
}


TEST(NumberTest, FloatBoxing) {
    auto zero = yalx_new_f32(heap, 0);
    ASSERT_EQ(0, zero->box.f32);
    ASSERT_EQ(zero, yalx_new_f32(heap, 0));
    
    auto one = yalx_new_f32(heap, 1);
    ASSERT_EQ(1, one->box.f32);
    ASSERT_EQ(one, yalx_new_f32(heap, 1));
}
