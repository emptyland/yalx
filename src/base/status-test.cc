#include "base/status.h"
#include "gtest/gtest.h"

namespace yalx::base {

TEST(StatusTest, Sanity) {
    auto ok = Status::OK();
    ASSERT_TRUE(ok.ok());
}

TEST(StatusTest, Errors) {
    auto s = ERR_CORRUPTION("fail");
    ASSERT_EQ("[base/status-test.cc:12] Corruption: fail", s.ToString());
}

} // namespace yalx
