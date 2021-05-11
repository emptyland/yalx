#include "base/env.h"
#include <gtest/gtest.h>

namespace yalx {

namespace base {

TEST(EnvTest, OSPageMemory) {
    auto memory = Env::OSPageAllocate(Env::kOSPageSize, Env::kMemoryWriteable|Env::kMemoryExecuteable);
    ASSERT_TRUE(memory.is_valid());
    ASSERT_EQ(Env::kOSPageSize, memory.size());
}


} // namespace base

} // namespace yalx
