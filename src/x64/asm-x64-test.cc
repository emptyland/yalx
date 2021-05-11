#include "x64/asm-x64.h"
#include "base/env.h"
#include <gtest/gtest.h>

namespace yalx {

namespace x64 {

class X64AssemblerTest : public ::testing::Test {
public:
    
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
    
    auto memory = base::Env::OSPageAllocate(base::Env::kOSPageSize,
                                            base::Env::kMemoryExecuteable|
                                            base::Env::kMemoryReadable|
                                            base::Env::kMemoryWriteable);
    auto fun = memory.WriteTo<int64_t()>(assembler_.buf());
    ASSERT_EQ(0, fun());
}

#endif // defined(YALX_ARCH_X64)

} // namespace x64

} // namespace yalx
