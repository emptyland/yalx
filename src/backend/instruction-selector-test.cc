#include "instruction-selector-test.h"

namespace yalx {

namespace backend {

InstructionSelectorTest::InstructionSelectorTest()
: linkage_(&arena_)
, const_pool_(&arena_)
, ops_(&arena_) {
    auto name = String::New(arena(), "test");
    auto full_name = String::New(arena(), "testing.test");
    auto path = String::New(arena(), "testing");
    auto full_path = String::New(arena(), "testing:testing.test");
    module_ = new (arena()) ir::Module(arena(), name, full_name, path, full_path);
}

ir::Function *InstructionSelectorTest::NewFun(const char *name,
                                              const char *sign,
                                              std::initializer_list<ir::Type> args,
                                              std::initializer_list<ir::Type> rets) {
    auto proto = new (arena()) ir::PrototypeModel(arena(), String::New(arena(), sign), false);
    for (auto ty : args) {
        proto->mutable_params()->push_back(ty);
    }
    for (auto ty: rets) {
        proto->mutable_return_types()->push_back(ty);
    }

    auto fun_name = String::New(arena(), name);
    auto fun = module_->NewStandaloneFunction(ir::Function::kDefault, fun_name, fun_name, proto);
    
    int hint = 0;
    for (auto ty : args) {
        fun->mutable_paramaters()->push_back(ir::Value::New(arena(), kUnknown, ty, ops_.Argument(hint++)));
    }
    fun->NewBlock(String::New(arena(), "entry"));
    return fun;
}

//    const String *name;
//    Access access;
//    ptrdiff_t offset;
//    Type type;
//    int16_t enum_value;
ir::StructureModel *InstructionSelectorTest::NewFooClass() {
    auto name = String::New(arena(), "Foo");
    auto full_name = String::New(arena(), "testing:testing.test.Foo");
    auto klass = module_->NewClassModel(name, full_name, nullptr);
    klass->InsertField({
        String::New(arena(), "a"),
        ir::kPublic,
        0,
        ir::Types::Int32,
        0
    });
    klass->InsertField({
        String::New(arena(), "b"),
        ir::kPublic,
        0,
        ir::Types::Int32,
        0
    });
    klass->UpdatePlacementSizeInBytes();
    return klass;
}

} // namespace backend

} // namespace yalx
