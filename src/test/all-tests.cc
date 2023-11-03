#include "runtime/heap/heap.h"
#include "runtime/runtime.h"
#include "base/env.h"
#include "base/at-exit.h"
#include "base/base.h"
#include "gtest/gtest.h"

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    yalx::base::Env::Init();
    yalx::base::AtExit at_exit(yalx::base::AtExit::INITIALIZER);
    
    yalx_runtime_options options {
        512 * MB,
        GC_NONE,
    };
    ::yalx_runtime_init(&options);
    int rs = RUN_ALL_TESTS();
    ::yalx_runtime_eixt();
    return rs;
}
