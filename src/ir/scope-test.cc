#include "ir/scope.h"
#include "ir/node.h"
#include "ir/metadata.h"
#include "ir/operators-factory.h"
#include "ir/operator.h"
#include <gtest/gtest.h>

namespace yalx {

namespace ir {

class ScopeTest : public ::testing::Test {
public:
    ScopeTest(): ops_(&arena_) {}
    
    void SetUp() override {
        auto name = String::New(&arena_, "main");
        auto path = String::New(&arena_, "main");
        auto full_path = String::New(&arena_, "project/src/main");
        auto module = new (&arena_) Module(&arena_, name, path, full_path);
        ASSERT_STREQ("main", module->name()->data());
        ASSERT_STREQ("main", module->path()->data());
        ASSERT_STREQ("project/src/main", module->full_path()->data());
        module_ = module;
        
        PrototypeModel *prototype = new (&arena_) PrototypeModel(&arena_, false);
        prototype->mutable_params()->push_back(Types::Void);
        prototype->mutable_return_types()->push_back(Types::Void);
        fun_ = module_->NewFunction(prototype);
    }
    
protected:
    base::Arena arena_;
    OperatorsFactory ops_;
    Module *module_ = nullptr;
    Function *fun_ = nullptr;
    IRCodeEnvScope *scope_ = nullptr;
};

TEST_F(ScopeTest, Sanity) {
    auto block = fun_->NewBlock(String::New(&arena_, "l1"));
    
    IRCodeBranchScope lv1(&scope_, nullptr, block);
    ASSERT_EQ(scope_, &lv1);
    ASSERT_TRUE(lv1.IsTrunk());
    ASSERT_FALSE(lv1.IsBranch());
    ASSERT_EQ(1, lv1.level());
    
    block = fun_->NewBlock(String::New(&arena_, "l2"));
    IRCodeBranchScope lv2(&scope_, nullptr, block);
    ASSERT_EQ(scope_, &lv2);
    EXPECT_EQ(&lv1, scope_->prev());
    ASSERT_TRUE(lv2.IsTrunk());
    ASSERT_FALSE(lv2.IsBranch());
    ASSERT_EQ(2, lv2.level());
}

TEST_F(ScopeTest, Branchs) {
    auto block = fun_->NewBlock(String::New(&arena_, "l1"));
    
    IRCodeBranchScope trunk(&scope_, nullptr, block);
    ASSERT_TRUE(trunk.IsTrunk());
    ASSERT_FALSE(trunk.IsBranch());
    ASSERT_EQ(1, trunk.level());
    
    block = fun_->NewBlock(String::New(&arena_, "br1"));
    auto br1 = trunk.Branch(nullptr, block);
    ASSERT_TRUE(br1->IsBranch());
    
    block = fun_->NewBlock(String::New(&arena_, "br2"));
    auto br2 = trunk.Branch(nullptr, block);
    ASSERT_TRUE(br2->IsBranch());
    
    ASSERT_TRUE(trunk.InBranchs(br1));
    ASSERT_TRUE(trunk.InBranchs(br2));
    
    br1->Enter();
    ASSERT_EQ(scope_, br1);
    br1->Exit();
    
    br2->Enter();
    ASSERT_EQ(scope_, br2);
    br2->Exit();
}

TEST_F(ScopeTest, BranchsValueVersions) {
    auto block = fun_->NewBlock(String::New(&arena_, "l1"));
    
    auto k100 = Value::New0(&arena_, SourcePosition::Unknown(), Types::Int32, ops_.I32Constant(100));
    auto k200 = Value::New0(&arena_, SourcePosition::Unknown(), Types::Int32, ops_.I32Constant(200));
    auto k1 = Value::New0(&arena_, SourcePosition::Unknown(), Types::Int32, ops_.I32Constant(1));
    auto k2 = Value::New0(&arena_, SourcePosition::Unknown(), Types::Int32, ops_.I32Constant(2));
    
    IRCodeBranchScope trunk(&scope_, nullptr, block);
    trunk.Register("a", k100, Model::kVar);
    trunk.Register("b", k200, Model::kVar);
    
    block = fun_->NewBlock(String::New(&arena_, "br1"));
    auto br1 = trunk.Branch(nullptr, block);
    br1->Enter();
    br1->Update("a", &trunk, k1);
    ASSERT_EQ(k1, br1->FindValueOrNull("a"));
    ASSERT_EQ(k100, trunk.FindValueOrNull("a"));
    br1->Exit();
    
    block = fun_->NewBlock(String::New(&arena_, "br2"));
    auto br2 = trunk.Branch(nullptr, block);
    br2->Enter();
    br2->Update("b", &trunk, k2);
    ASSERT_EQ(k2, br2->FindValueOrNull("b"));
    br2->Exit();
}

TEST_F(ScopeTest, NestedBranchs) {
    auto block = fun_->NewBlock(String::New(&arena_, "l1"));
    
    auto k100 = Value::New0(&arena_, SourcePosition::Unknown(), Types::Int32, ops_.I32Constant(100));
    auto k200 = Value::New0(&arena_, SourcePosition::Unknown(), Types::Int32, ops_.I32Constant(200));
    auto k1 = Value::New0(&arena_, SourcePosition::Unknown(), Types::Int32, ops_.I32Constant(1));
    auto k2 = Value::New0(&arena_, SourcePosition::Unknown(), Types::Int32, ops_.I32Constant(2));
    
    IRCodeBranchScope trunk(&scope_, nullptr, block);
    trunk.Register("a", k100, Model::kVar);
    trunk.Register("b", k200, Model::kVar);
    
    block = fun_->NewBlock(String::New(&arena_, "br1"));
    auto br1 = trunk.Branch(nullptr, block);
    br1->Enter();
    {
        block = fun_->NewBlock(String::New(&arena_, "b1"));
        IRCodeBranchScope b1(&scope_, nullptr, block);
        block = fun_->NewBlock(String::New(&arena_, "b2"));
        IRCodeBranchScope b2(&scope_, nullptr, block);
        b2.Update("a", &trunk, k1);
        auto sym = b2.FindSymbol("a");
        ASSERT_EQ(Symbol::kVal, sym.kind);
        ASSERT_EQ(k1, sym.core.value);
    }
    ASSERT_EQ(k1, br1->FindValueOrNull("a"));
    br1->Exit();
}

} // namespace ir

} // namespace yalx
