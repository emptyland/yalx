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
        auto module = new (&arena_) Module(&arena_, name, name, path, full_path);
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
    auto clazz = module_->NewClassModel(name, name, nullptr);
    EXPECT_EQ(StructureModel::kClass, clazz->declaration());
    
    clazz->InsertField({String::New(&arena_, "a"), kPublic, 0, Types::Int32, false});
    clazz->InsertField({String::New(&arena_, "b"), kPublic, 0, Types::Int32, false});
    
    ASSERT_EQ(2, clazz->fields_size());
    if (auto field = clazz->FindField("a")) {
        EXPECT_STREQ("a", field->name->data());
        EXPECT_EQ(Type::kInt32, field->type.kind());
        EXPECT_EQ(kPublic, field->access);
        //EXPECT_FALSE(field->is_volatile);
    }
}

TEST_F(MetadataTest, Interface) {
    auto name = String::New(&arena_, "Foo");
    auto clazz = module_->NewInterfaceModel(name, name);
    
    name = String::New(&arena_, "foo");
    auto prototype = new (&arena_) PrototypeModel(&arena_, String::kEmpty, false/*vargs*/);
    prototype->mutable_params()->push_back(Types::Int32);
    prototype->mutable_params()->push_back(Types::Int32);
    prototype->mutable_return_types()->push_back(Types::Int32);
    auto fun = module_->NewStandaloneFunction(Function::kAbstract, name, name, prototype);
    
    fun->mutable_paramaters()->push_back(Value::New(&arena_, SourcePosition::Unknown(), Types::Int32, ops_.Argument(0)));
    fun->mutable_paramaters()->push_back(Value::New(&arena_, SourcePosition::Unknown(), Types::Int32, ops_.Argument(1)));
    clazz->InsertMethod(fun);
    
    if (auto method = clazz->FindMethod("foo")) {
        EXPECT_EQ(0, method->in_itab);
        EXPECT_EQ(0, method->in_vtab);
        EXPECT_EQ(kPublic, method->access);
        EXPECT_STREQ("foo", method->fun->name()->data());
    }
}


} // namespace ir

} // namespace yalx
