#include "vm/bytecode/bytecode-array-builder.h"
#include <gtest/gtest.h>

namespace yalx {

namespace vm {

class BytecodeArrayBuilderTest : public ::testing::Test {
public:
    BytecodeArrayBuilderTest(): builder_(&arena_) {}
    
    base::Arena arena_;
    BytecodeArrayBuilder builder_;
};

TEST_F(BytecodeArrayBuilderTest, Sanity) {
    auto bc = builder_.EmitNoPrefix<Bitwise::kLong>(Bytecode::kLda, 0);
    ASSERT_NE(nullptr, bc);
    
    ASSERT_EQ(Bitwise::kLong, bc->bitwise());
    ASSERT_EQ(Bytecode::kLda, bc->prefix());
    ASSERT_EQ(bc->prefix(), bc->opcode());
    ASSERT_EQ(1, bc->OperandsCount());
    ASSERT_EQ(0, Operands<int8_t>::At(bc, 0));
}

TEST_F(BytecodeArrayBuilderTest, Opcode_Move) {
    auto bc = builder_.EmitNoPrefix<Bitwise::kQuad>(Bytecode::kMove, 32, 64);
    
    ASSERT_EQ(Bitwise::kQuad, bc->bitwise());
    ASSERT_EQ(Bytecode::kMove, bc->prefix());
    ASSERT_EQ(bc->prefix(), bc->opcode());
    ASSERT_EQ(2, bc->OperandsCount());
    ASSERT_EQ(32, Operands<int8_t>::At(bc, 0));
    ASSERT_EQ(64, Operands<int8_t>::At(bc, 1));
}

TEST_F(BytecodeArrayBuilderTest, FloatingOperands) {
    auto bc = builder_.Emit<float, Bitwise::kFloat>(Bytecode::kLoadImm, 3.14f);
    
    ASSERT_EQ(Bitwise::kFloat, bc->bitwise());
    ASSERT_EQ(Bytecode::kWide32, bc->prefix());
    ASSERT_EQ(Bytecode::kLoadImm, bc->opcode());
    ASSERT_EQ(1, bc->OperandsCount());
    ASSERT_NEAR(3.14f, Operands<float>::At(bc, 0), 0.001f);
}

} // namespace vm

} // namespace yalx
