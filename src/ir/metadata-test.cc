#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/operators-factory.h"
#include <gtest/gtest.h>

namespace yalx {

namespace ir {

class MetadataTest : public ::testing::Test {
public:
    MetadataTest(): ops_(&arena_) {}
    
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

TEST_F(MetadataTest, Sanity) {
    // StructureModel(base::Arena *arena, const String *name, Constraint constraint, Module *owns, StructureModel *base_of);
    auto name = String::New(&arena_, "Foo");
    auto clazz = module_->NewClassModel(name, nullptr);
    EXPECT_EQ(StructureModel::kClass, clazz->declaration());
    
    clazz->InsertField({String::New(&arena_, "a"), Model::kVal, kPublic, 0, Types::Int32, false});
    clazz->InsertField({String::New(&arena_, "b"), Model::kVal, kPublic, 0, Types::Int32, false});
    
    ASSERT_EQ(2, clazz->fields_size());
    if (auto [field, ok] = clazz->FindField("a"); true) {
        ASSERT_TRUE(ok);
        EXPECT_STREQ("a", field.name->data());
        EXPECT_EQ(Type::kInt32, field.type.kind());
        EXPECT_EQ(Model::kVal, field.constraint);
        EXPECT_EQ(kPublic, field.access);
        EXPECT_FALSE(field.is_volatile);
    }
}

TEST_F(MetadataTest, Interface) {
    auto name = String::New(&arena_, "Foo");
    auto clazz = module_->NewInterfaceModel(name);
    
    name = String::New(&arena_, "foo");
    auto prototype = new (&arena_) PrototypeModel(&arena_, false/*vargs*/);
    prototype->mutable_params()->push_back(Types::Int32);
    prototype->mutable_params()->push_back(Types::Int32);
    prototype->mutable_return_types()->push_back(Types::Int32);
    auto fun = module_->NewStandaloneFunction(name, prototype);
    
    fun->mutable_paramaters()->push_back(Value::New0(&arena_, SourcePosition::Unknown(), Types::Int32, ops_.Argument(0)));
    fun->mutable_paramaters()->push_back(Value::New0(&arena_, SourcePosition::Unknown(), Types::Int32, ops_.Argument(1)));
    clazz->InsertMethod(fun);
    
    if (auto [method, ok] = clazz->FindMethod("foo"); true) {
        ASSERT_TRUE(ok);
        EXPECT_EQ(0, method.offset);
        EXPECT_EQ(kPublic, method.access);
        EXPECT_STREQ("foo", method.fun->name()->data());
        EXPECT_FALSE(method.is_native);
        EXPECT_FALSE(method.is_override);
    }
}


} // namespace ir

} // namespace yalx
