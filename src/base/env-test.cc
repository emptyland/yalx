#include "base/env.h"
#include "base/io.h"
#include <gtest/gtest.h>

namespace yalx {

namespace base {

TEST(EnvTest, OSPageMemory) {
    auto memory = Env::OSPageAllocate(Env::kOSPageSize, Env::kMemoryWriteable|Env::kMemoryExecuteable);
    ASSERT_TRUE(memory.is_valid());
    ASSERT_EQ(Env::kOSPageSize, memory.size());
}

TEST(EnvTest, OpenNotExistFile) {
    std::unique_ptr<SequentialFile> file;
    auto rs = Env::NewSequentialFile(".not_exists_file", &file);
    ASSERT_TRUE(rs.IsCorruption());
    //FAIL() << rs.ToString();
}


} // namespace base

} // namespace yalx
