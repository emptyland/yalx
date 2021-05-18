#include "arm64/asm-arm64.h"
#include "base/env.h"
#include "base/base.h"
#include <gtest/gtest.h>

namespace yalx {

namespace arm64 {

#define __ assembler_.

class Arm64AssemblerTest : public ::testing::Test {
public:
    
    base::OSPageMemory NewCodeBuffer() {
        return base::Env::OSPageAllocate(base::Env::kOSPageSize, base::Env::kMemoryWRX);
    }
    
    void FrameEnter(int frame_size = 0x10) {
        __ sub(sp, sp, frame_size);
        __ stp(fp, lr, MemOperand{sp, frame_size - 0x10});
    }
    
    void FrameExit(int frame_size = 0x10) {
        __ ldp(fp, lr, MemOperand{sp, frame_size - 0x10});
        __ add(sp, sp, frame_size);
        __ ret();
    }

    Assembler assembler_;
}; // class X64AssemblerTest

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
    
    __ ldr(x0, MemOperand(x1, Operand{0}));
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

TEST_F(Arm64AssemblerTest, PosixABICalling) {
    __ sub(sp, sp, Operand{0x20});
    __ stp(fp, lr, MemOperand(sp, Operand{0x10}));
    __ add(x0, x0, x1);
    __ ldp(fp, lr, MemOperand(sp, Operand{0x10}));
    __ add(sp, sp, Operand{0x20});
    __ ret();
    
    auto memory = NewCodeBuffer();
    
    auto fun = memory.WriteTo<int64_t(int64_t, int64_t)>(assembler_.buf(), true/*code*/);
    ASSERT_EQ(1, fun(1, 0));
    ASSERT_EQ(3, fun(1, 2));
}

TEST_F(Arm64AssemblerTest, SimpleAdd) {
    __ sub(sp, sp, 0x10);
    __ stp(fp, lr, MemOperand{sp, 0x10});
    __ add(w0, w0, w1);
    __ ldp(fp, lr, MemOperand{sp, 0x10});
    __ add(sp, sp, 0x10);
    __ ret();

    auto memory = NewCodeBuffer();
    auto fun = memory.WriteTo<int(int, int)>(assembler_.buf(), true/*code*/);
    ASSERT_EQ(1, fun(1, 0));
    ASSERT_EQ(3, fun(1, 2));
}

TEST_F(Arm64AssemblerTest, SimpleMul) {
    __ sub(sp, sp, 0x10);
    __ stp(fp, lr, MemOperand{sp, 0x10});
    __ mul(w0, w0, w1);
    __ ldp(fp, lr, MemOperand{sp, 0x10});
    __ add(sp, sp, 0x10);
    __ ret();

    auto memory = NewCodeBuffer();
    auto fun = memory.WriteTo<int(int, int)>(assembler_.buf(), true/*code*/);
    ASSERT_EQ(0, fun(1, 0));
    ASSERT_EQ(2, fun(1, 2));
}

struct Arm64TestStub {
    int i1;
    int i2;
    int i3;
    int i4;
};

TEST_F(Arm64AssemblerTest, MemoryOffset) {
    FrameEnter();
    //__ brk(0);
    __ movz(w1, 0);
    __ str(w1, MemOperand{x0, 0x0});
    __ movz(w1, 1);
    __ str(w1, MemOperand{x0, 0x4});
    __ movz(w1, 2);
    __ str(w1, MemOperand{x0, 0x8});
    __ movz(w1, 3);
    __ str(w1, MemOperand{x0, 0xc});
    FrameExit();
    
    auto memory = NewCodeBuffer();
    auto fun = memory.WriteTo<void(Arm64TestStub *)>(assembler_.buf(), true/*code*/);
    
    Arm64TestStub stub;
    ::memset(&stub, 0, sizeof(stub));
    fun(&stub);
    ASSERT_EQ(0, stub.i1);
    ASSERT_EQ(1, stub.i2);
    ASSERT_EQ(2, stub.i3);
    ASSERT_EQ(3, stub.i4);
}

TEST_F(Arm64AssemblerTest, FloatingOps) {
    FrameEnter();
    __ fadd(s0, s0, s1);
    FrameExit();
    
    auto memory = NewCodeBuffer();
    auto fun = memory.WriteTo<float(float, float)>(assembler_.buf(), true/*code*/);
    
    ASSERT_EQ(0, fun(0, 0));
    ASSERT_EQ(2, fun(1, 1));
}

int Arm64TestStubFunAdd(int a, int b) {
    return a + b;
}

int Arm64TestStubFunSub(int a, int b) {
    return a - b;
}

TEST_F(Arm64AssemblerTest, CallExtenalFunction) {
    FrameEnter(0x20);
    __ str(x0, MemOperand{fp, -0x10});
    __ movz(w0, 1);
    __ movz(w1, 2);
    __ ldr(x9, MemOperand{fp, -0x10});
    __ blr(x9);
    FrameExit(0x20);
    
    auto memory = NewCodeBuffer();
    auto fun = memory.WriteTo<int(Address)>(assembler_.buf(), true/*code*/);
    
    ASSERT_EQ(3, fun(reinterpret_cast<Address>(Arm64TestStubFunAdd)));
    ASSERT_EQ(-1, fun(reinterpret_cast<Address>(Arm64TestStubFunSub)));
}

#endif // defined(YALX_ARCH_ARM64)


} // namespace arm64

} // namespace yalx

