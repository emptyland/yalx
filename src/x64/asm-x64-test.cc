#include "x64/asm-x64.h"
#include "base/env.h"
#include <gtest/gtest.h>

namespace yalx {

namespace x64 {

class X64AssemblerTest : public ::testing::Test {
public:
    
    base::OSPageMemory NewCodeBuffer() {
        return base::Env::OSPageAllocate(base::Env::kOSPageSize, base::Env::kMemoryWRX);
    }

    Assembler assembler_;
}; // class X64AssemblerTest


#if defined(YALX_ARCH_X64)

#define __ assembler_.

TEST_F(X64AssemblerTest, Sanity) {
    __ pushq(rbp);
    __ movq(rbp, rsp);
    __ xorq(rax, rax);
    __ popq(rbp);
    __ ret(0);
    
    auto memory = NewCodeBuffer();
    auto fun = memory.WriteTo<int64_t()>(assembler_.buf(), true/*code*/);
    ASSERT_EQ(0, fun());
}

TEST_F(X64AssemblerTest, Add) {
    __ pushq(rbp);
    __ movq(rbp, rsp);
    __ movq(rax, Argv_0);
    __ addq(rax, Argv_1);
    __ popq(rbp);
    __ ret(0);
    
    auto memory = NewCodeBuffer();
    auto fun = memory.WriteTo<int64_t(int64_t, int64_t)>(assembler_.buf(), true/*code*/);
    ASSERT_EQ(3, fun(1, 2));
}

#endif // defined(YALX_ARCH_X64)

} // namespace x64

} // namespace yalx
