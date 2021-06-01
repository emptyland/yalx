#include "runtime/stack.h"
#include <gtest/gtest.h>

TEST(StackTest, Sanity) {
    struct stack stack;
    ASSERT_EQ(0, yalx_init_stack(16 * KB, &stack));
    ASSERT_EQ(16 * KB, stack.size);
    ASSERT_EQ(16 * KB, stack.top - stack.bottom);
    
    yalx_free_stack(&stack);
}

TEST(StackTest, NewStackFromPool) {
    struct stack *stack = yalx_new_stack_from_pool(&stack_pool, 16 * KB);
    ASSERT_NE(nullptr, stack);
    ASSERT_EQ(16 * KB, stack->size);
    ASSERT_EQ(16 * KB, stack->top - stack->bottom);
    yalx_delete_stack_to_pool(&stack_pool, stack);
    
    auto other = yalx_new_stack_from_pool(&stack_pool, 16 * KB);
    ASSERT_NE(nullptr, other);
    ASSERT_EQ(other, stack);
}
