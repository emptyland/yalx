#include "backend/instruction-selector.h"
#include "backend/linkage-symbols.h"
#include "backend/frame.h"
#include "backend/arm64/instruction-codes-arm64.h"
#include "ir/operators-factory.h"
#include "ir/operator.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/type.h"
#include <gtest/gtest.h>

namespace yalx {

namespace backend {



class Arm64InstructionSelectorTest : public ::testing::Test {
public:
    static constexpr ir::SourcePosition kUnknown = ir::SourcePosition::Unknown();
    
    Arm64InstructionSelectorTest()
    : linkage_(&arena_)
    , ops_(&arena_) {
        auto name = String::New(arena(), "test");
        auto full_name = String::New(arena(), "testing.test");
        auto path = String::New(arena(), "testing");
        auto full_path = String::New(arena(), "testing:testing.test");
        module_ = new (arena()) ir::Module(arena(), name, full_name, path, full_path);
    }
    
    ir::Function *NewFun(const char *name,
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
    
protected:
    base::Arena *arena() { return &arena_; }
    
    base::Arena arena_;
    Linkage linkage_;
    ir::Module *module_;
    ir::OperatorsFactory ops_;
}; // class Arm64InstructionSelectorTest



TEST_F(Arm64InstructionSelectorTest, Sanity) {
    auto fun = NewFun("foo",
                      "fun (i32,i32)->(i32,i32)",
                      {ir::Types::Int32, ir::Types::Int32},
                      {ir::Types::Int32, ir::Types::Int32});
    auto blk = fun->entry();
    blk->NewNode(kUnknown, ir::Types::Void, ops_.Ret(2), fun->paramater(1), fun->paramater(1));
    
    auto instr_fun = Arm64SelectFunctionInstructions(arena(), &linkage_, fun);
    ASSERT_NE(nullptr, instr_fun);
    
    auto block = instr_fun->block(0);
    ASSERT_EQ(2, block->instructions().size());
    auto instr = block->instruction(0);
    ASSERT_EQ(ArchFrameEnter, instr->op());
    
    instr = block->instruction(1);
    ASSERT_EQ(ArchFrameExit, instr->op());
    ASSERT_TRUE(instr->InputAt(0)->IsUnallocated());
    if (auto opd = instr->InputAt(0)->AsUnallocated()) {
        ASSERT_EQ(UnallocatedOperand::kFixedSlot, opd->policy());
        ASSERT_EQ(24, opd->fixed_slot_offset());
    }
    
    ASSERT_TRUE(instr->InputAt(1)->IsUnallocated());
    if (auto opd = instr->InputAt(1)->AsUnallocated()) {
        ASSERT_EQ(UnallocatedOperand::kFixedSlot, opd->policy());
        ASSERT_EQ(28, opd->fixed_slot_offset());
    }
}

} // namespace backend

} // namespace yalx
