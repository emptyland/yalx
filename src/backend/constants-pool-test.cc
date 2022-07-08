#include "backend/constants-pool.h"
#include <gtest/gtest.h>

namespace yalx {

namespace backend {

class ConstantsPoolTest : public ::testing::Test {
public:
    ConstantsPoolTest(): pool_(&arena_) {}
    
protected:
    base::Arena arena_;
    ConstantsPool pool_;
}; // class ConstantsPoolTest

TEST_F(ConstantsPoolTest, Sanity) {
    auto id = pool_.FindOrInsertWord8(0);
    EXPECT_EQ(id, pool_.FindOrInsertWord8(0));
    
    id = pool_.FindOrInsertFloat32(1.1);
    EXPECT_EQ(id, pool_.FindOrInsertFloat32(1.1));
    EXPECT_NE(id, pool_.FindOrInsertFloat64(1.1));
    
    ASSERT_EQ(3, pool_.numbers().size());
}

TEST_F(ConstantsPoolTest, Kind) {
    for (int i = 0; i < 1000; i++) {
        auto id = pool_.FindOrInsertWord32(i);
        EXPECT_EQ(id, pool_.FindOrInsertWord32(i));
        EXPECT_EQ(i, id);
    }
    
    for (auto pair : pool_.numbers()) {
        auto slot = std::get<0>(pair);
        EXPECT_EQ(MachineRepresentation::kWord32, slot.kind);
        EXPECT_LT(slot.value<uint32_t>(), 1000);
    }
}

TEST_F(ConstantsPoolTest, StringPool) {
    auto s1 = String::New(&arena_, "ok");
    auto id = pool_.FindOrInsertString(s1);
    EXPECT_EQ(id, pool_.FindOrInsertString(s1));
    EXPECT_EQ(s1, pool_.string_pool()[0]);
    
    auto s2 = String::New(&arena_, "no");
    EXPECT_NE(id, pool_.FindOrInsertString(s2));
    id = pool_.FindOrInsertString(s2);
    EXPECT_EQ(id, pool_.FindOrInsertString(s2));
    EXPECT_EQ(s2, pool_.string_pool()[1]);
}

} // namespace backend

} // namespace yalx
