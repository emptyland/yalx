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

TEST_F(InstructionTest, BinOpTest) {
    InstructionOperand inputs[2];
    inputs[0] = UnallocatedOperand{UnallocatedOperand::kMustHaveRegister, 1};
    inputs[1] = UnallocatedOperand{UnallocatedOperand::kRegisterOrSlotOrConstant, 2};
    
    UnallocatedOperand output{UnallocatedOperand::kMustHaveRegister, 3};
    
    auto instr = Instruction::New(arena(), Arm64Add, arraysize(inputs), inputs, 1, &output, 0, nullptr);
    ASSERT_EQ(2, instr->inputs_count());
    ASSERT_EQ(1, instr->outputs_count());
    ASSERT_EQ(0, instr->temps_count());
    
    ASSERT_TRUE(instr->InputAt(0)->IsUnallocated());
    ASSERT_TRUE(instr->InputAt(1)->IsUnallocated());
    ASSERT_TRUE(instr->OutputAt(0)->IsUnallocated());
}

} // namespace backend

} // namespace yalx
