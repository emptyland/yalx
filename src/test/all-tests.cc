#include "base/at-exit.h"
#include "base/base.h"
#include "gtest/gtest.h"

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    yalx::base::AtExit at_exit(yalx::base::AtExit::INITIALIZER);
    return RUN_ALL_TESTS();
}
