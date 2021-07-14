#include "ir/node.h"
#include "ir/operators-factory.h"
#include <gtest/gtest.h>

namespace yalx {

namespace ir {

class NodeTest : public ::testing::Test {
public:
    NodeTest(): factory_(&arena_) {}
    
protected:
    base::Arena arena_;
    OperatorsFactory factory_;
};

TEST_F(NodeTest, Sanity) {
    auto k1 = Value::New0(&arena_, Types::Int32, factory_.I32Constant(1));
    auto k2 = Value::New0(&arena_, Types::Int32, factory_.I32Constant(2));
    auto ret = Value::New(&arena_, Types::Void, factory_.Ret(2), k1, k2);
    
    ASSERT_EQ(Operator::kRet, ret->op()->value());
    ASSERT_EQ(k1, ret->InputValue(0));
    ASSERT_EQ(k2, ret->InputValue(1));
}

} // namespace ir

} // namespace yalx
