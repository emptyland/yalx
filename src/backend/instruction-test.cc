#include "backend/instruction.h"
#include <gtest/gtest.h>

namespace yalx {

namespace backend {

class InstructionTest : public ::testing::Test {
public:
    
protected:
    base::Arena *arena() { return &arena_; }
    
    base::Arena arena_;
}; // class InstructionTest


TEST_F(InstructionTest, Sanity) {
    auto block = new (arena()) InstructionBlock(arena(), nullptr, 0);
    ASSERT_TRUE(block->successors().empty());
    ASSERT_TRUE(block->predecessors().empty());
}

TEST_F(InstructionTest, ArchNop) {

}

} // namespace backend

} // namespace yalx
