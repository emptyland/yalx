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
    auto block = new (arena()) InstructionBlock(arena(), nullptr, 0);
    auto instr = block->New(ArchNop);
    ASSERT_EQ(ArchNop, instr->op());
    instr = block->New(ArchUnreachable);
    ASSERT_EQ(ArchUnreachable, instr->op());
    ASSERT_EQ(2, block->instructions().size());
}

} // namespace backend

} // namespace yalx
