#include "runtime/stack.h"
#include <gtest/gtest.h>

TEST(StackTest, Sanity) {
    struct stack stack;
    ASSERT_EQ(0, yalx_init_stack(16 * KB, &stack));
    ASSERT_EQ(16 * KB, stack.size);
    ASSERT_EQ(16 * KB, stack.top - stack.bottom);
    
    yalx_free_stack(&stack);
}
