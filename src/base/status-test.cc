#include "base/status.h"
#include "gtest/gtest.h"

namespace yalx {

namespace base {

TEST(StatusTest, Sanity) {
    auto ok = Status::OK();
    ASSERT_TRUE(ok.ok());
}

TEST(StatusTest, Errors) {
    auto s = ERR_CORRUPTION("fail");
    ASSERT_EQ("[base/status-test.cc:14] Corruption: fail", s.ToString());
}

} // namespace base

} // namespace yalx
