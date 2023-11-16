#include "backend/x64/lower-posix-x64.h"
#include "compiler/compiler-suit-test.h"

namespace yalx::backend {

class X64PosixLowerTest : public cpl::CompilerTestSuit {
public:
    void SetUp() override {
        auto rs = ParseAll("tests/32-code-lower");
        ASSERT_TRUE(rs.ok()) << rs.ToString();
    }

};

TEST_F(X64PosixLowerTest, Sanity) {
    PrintAllPackages();
}

} // namespace yalx::backend