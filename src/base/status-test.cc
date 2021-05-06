#include "base/status.h"
#include "gtest/gtest.h"

namespace yalx {

namespace base {

TEST(StatusTest, Sanity) {
    auto ok = Status::OK();
    ASSERT_TRUE(ok.ok());
}

} // namespace base

} // namespace yalx
