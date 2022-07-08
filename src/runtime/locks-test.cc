#include "runtime/locks.h"
#include <gtest/gtest.h>
#include <thread>


TEST(LocksTest, Sanity) {
    struct yalx_spin_lock lock{0};
    yalx_spin_lock(&lock);
    
    yalx_spin_unlock(&lock);
}

TEST(LocksTest, ThreadSafe) {
    static const int k = 100000;
    int c = 0;
    struct yalx_spin_lock lock{0};
    
    std::thread workers[5];
    for (int i = 0; i < 5; i++) {
        workers[i] = std::thread([&c, this](struct yalx_spin_lock *lock){
            for (int i = 0; i < k; i++) {
                yalx_spin_lock(lock);
                c++;
                yalx_spin_unlock(lock);
            }
        }, &lock);
    }
    for (int i = 0; i < 5; i++) {
        workers[i].join();
    }
    ASSERT_EQ(5 * k, c);
}
