#include "ir/node.h"
#include "ir/operators-factory.h"
#include <gtest/gtest.h>

namespace yalx {

namespace ir {

class NodeTest : public ::testing::Test {
public:
    NodeTest(): ops_(&arena_) {}
    
    void SetUp() override {
        auto name = String::New(&arena_, "main");
        auto path = String::New(&arena_, "main");
        auto full_path = String::New(&arena_, "project/src/main");
        auto module = new (&arena_) Module(&arena_, name, path, full_path);
        ASSERT_STREQ("main", module->name()->data());
        ASSERT_STREQ("main", module->path()->data());
        ASSERT_STREQ("project/src/main", module->full_path()->data());
        module_ = module;
    }
    
protected:
    base::Arena arena_;
    OperatorsFactory ops_;
    Module *module_ = nullptr;
};

TEST_F(NodeTest, Sanity) {
    auto k1 = Value::New0(&arena_, Types::Int32, ops_.I32Constant(1));
    auto k2 = Value::New0(&arena_, Types::Int32, ops_.I32Constant(2));
    auto ret = Value::New(&arena_, Types::Void, ops_.Ret(2), k1, k2);
    
    ASSERT_EQ(Operator::kRet, ret->op()->value());
    ASSERT_EQ(k1, ret->InputValue(0));
    ASSERT_EQ(k2, ret->InputValue(1));
}

TEST_F(NodeTest, Module) {
    auto fun = module_->NewFunction(module_->name());
    EXPECT_EQ(fun, module_->FindFunOrNull(module_->name()->ToSlice()));
    
    auto bb = fun->NewBlock(String::kEmpty);
    EXPECT_EQ(bb, fun->entry());

    fun->mutable_paramaters()->push_back(Value::New0(&arena_, Types::Int32, ops_.Argument(0)));
    fun->mutable_paramaters()->push_back(Value::New0(&arena_, Types::Int32, ops_.Argument(1)));
    
    auto val = bb->NewNode(Types::Void, ops_.Word32Add(), fun->paramater(0), fun->paramater(1));
    val = bb->NewNode(Types::Void, ops_.Ret(1), val);
    USE(val);
}

} // namespace ir

} // namespace yalx
