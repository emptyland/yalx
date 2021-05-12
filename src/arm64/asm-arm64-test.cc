#include "arm64/asm-arm64.h"
#include "base/env.h"
#include "base/base.h"
#include <gtest/gtest.h>

namespace yalx {

namespace arm64 {

class Arm64AssemblerTest : public ::testing::Test {
public:
    
    base::OSPageMemory NewCodeBuffer() {
        return base::Env::OSPageAllocate(base::Env::kOSPageSize, base::Env::kMemoryWRX);
    }

    Assembler assembler_;
}; // class X64AssemblerTest

#define __ assembler_.

TEST_F(Arm64AssemblerTest, Register) {
    ASSERT_EQ(32, w1.size_in_bits());
    ASSERT_EQ(64, x1.size_in_bits());
}

TEST_F(Arm64AssemblerTest, AliasRegisters) {
    ASSERT_TRUE(fp == x29);
    ASSERT_TRUE(lr == x30);
}

TEST_F(Arm64AssemblerTest, BindBeforeJump) {
    Label label;
    __ nop();
    __ bind(&label);
    __ add(x0, x1, Operand{x2});
    __ b(&label);
    __ ret();
}

TEST_F(Arm64AssemblerTest, BindAfterJump) {
    Label label;
    __ nop();
    __ b(&label);
    __ add(x0, x1, Operand{x2});
    __ b(&label);
    __ nop();
    __ bind(&label);
    __ ret();
    
    auto instr = assembler_[1];
    ASSERT_TRUE(instr->IsImmBranch());
    ASSERT_EQ(4, instr->ImmBranch());
    
    instr = assembler_[3];
    ASSERT_TRUE(instr->IsImmBranch());
    ASSERT_EQ(2, instr->ImmBranch());
}

#if defined(YALX_ARCH_ARM64)

// TODO: Only for arm64

TEST_F(Arm64AssemblerTest, RunSanity) {
    FAIL() << "fail";
}

#endif // defined(YALX_ARCH_ARM64)


} // namespace arm64

} // namespace yalx

