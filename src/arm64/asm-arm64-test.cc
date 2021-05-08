#include "arm64/asm-arm64.h"
#include "base/base.h"
#include "gtest/gtest.h"

namespace yalx {

namespace arm64 {

TEST(Arm64Test, Register) {
    ASSERT_EQ(32, w1.size_in_bits());
    ASSERT_EQ(64, x1.size_in_bits());
}

TEST(Arm64Test, AliasRegisters) {
    ASSERT_TRUE(fp == x29);
    ASSERT_TRUE(lr == x30);
}

#if defined(YALX_ARCH_ARM64)

// TODO: Only for arm64

TEST(Arm64Test, RunSanity) {
    FAIL() << "fail";
}

#endif // defined(YALX_ARCH_ARM64)


} // namespace arm64

} // namespace yalx

