#include "base/env.h"
#include "base/at-exit.h"
#include "base/base.h"
#include "runtime/runtime.h"
#include "gtest/gtest.h"

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    yalx::base::Env::Init();
    yalx::base::AtExit at_exit(yalx::base::AtExit::INITIALIZER);
    ::yalx_runtime_init();
    return RUN_ALL_TESTS();
}
