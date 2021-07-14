#include "ir/operators-factory.h"
#include "ir/operator.h"
#include <gtest/gtest.h>

namespace yalx {

namespace ir {

class OperatorTest : public ::testing::Test {
public:
    OperatorTest(): factory_(&arena_) {}
    
protected:
    base::Arena arena_;
    OperatorsFactory factory_;
};

TEST_F(OperatorTest, Sanity) {
    auto phi = factory_.Phi(2, 2);
    ASSERT_EQ(Operator::kPhi, phi->value());
    ASSERT_EQ(2, phi->value_in());
    ASSERT_EQ(1, phi->value_out());
    ASSERT_EQ(2, phi->control_in());
    ASSERT_EQ(0, phi->control_out());
    
    auto k = factory_.I32Constant(100);
    EXPECT_EQ(Operator::kI32Constant, k->value());
    EXPECT_EQ(0, k->value_in());
    EXPECT_EQ(0, k->value_out());
    EXPECT_EQ(0, k->control_in());
    EXPECT_EQ(0, k->control_out());
}

TEST_F(OperatorTest, Constants) {
    auto k = factory_.I32Constant(-1);
    EXPECT_EQ(Operator::kI32Constant, k->value());
    EXPECT_EQ(-1, OperatorWith<int32_t>::Data(k));
}

} // namespace ir

} // namespace yalx
