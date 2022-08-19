#include "backend/instruction-selector-test.h"
#include "backend/instruction-selector.h"
#include "backend/registers-configuration.h"
#include "backend/register-allocator.h"
#include "backend/arm64/instruction-codes-arm64.h"
#include "base/io.h"

namespace yalx {

namespace backend {


class Arm64InstructionSelectorTest : public InstructionSelectorTest {
public:
    
    const RegistersConfiguration *regconf_ = RegistersConfiguration::of_arm64();
}; // class Arm64InstructionSelectorTest

TEST_F(Arm64InstructionSelectorTest, Sanity) {
    auto fun = NewFun("foo",
                      "fun (i32,i32)->(i32,i32)",
                      {ir::Types::Int32, ir::Types::Int32},
                      {ir::Types::Int32, ir::Types::Int32});
    auto blk = fun->entry();
    blk->NewNode(kUnknown, ir::Types::Void, ops()->Ret(2), fun->paramater(1), fun->paramater(1));
    
    auto instr_fun = Arm64SelectFunctionInstructions(arena(), linkage(), fun);
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

TEST_F(Arm64InstructionSelectorTest, HeapAllocSelecting) {
    auto foo_class = NewFooClass();
    auto foo_ty = ir::Type::Ref(foo_class);
    auto fun = NewFun("foo", "fun ()->(testing:testing.test.Foo)", {}, {foo_ty});
    auto blk = fun->entry();
    auto rs = blk->NewNode(kUnknown, foo_ty, ops()->HeapAlloc(foo_class));
    blk->NewNode(kUnknown, ir::Types::Void, ops()->Ret(1), rs);
    
    auto instr_fun = Arm64SelectFunctionInstructions(arena(), linkage(), fun);
    ASSERT_NE(nullptr, instr_fun);
    
    auto block = instr_fun->block(0);
    ASSERT_EQ(6, block->instructions().size());
    
    auto instr = block->instruction(0);
    ASSERT_EQ(ArchFrameEnter, instr->op());
    
    instr = block->instruction(1);
    ASSERT_EQ(ArchSaveCallerRegisters, instr->op());
    
    instr = block->instruction(2);
    ASSERT_EQ(ArchLoadEffectAddress, instr->op());
    
    instr = block->instruction(3);
    ASSERT_EQ(ArchCallNative, instr->op());
}

TEST_F(Arm64InstructionSelectorTest, ScanVirtualRegisters) {
    auto foo_class = NewFooClass();
    auto foo_ty = ir::Type::Ref(foo_class);
    auto fun = NewFun("foo", "fun ()->(testing:testing.test.Foo)", {}, {foo_ty});
    auto blk = fun->entry();
    auto rs = blk->NewNode(kUnknown, foo_ty, ops()->HeapAlloc(foo_class));
    blk->NewNode(kUnknown, ir::Types::Void, ops()->Ret(1), rs);
    
    auto instr_fun = Arm64SelectFunctionInstructions(arena(), linkage(), fun);
    ASSERT_NE(nullptr, instr_fun);
    
    RegisterAllocator allocator(arena(), regconf_, instr_fun);
    allocator.Prepare();
    
    ASSERT_EQ(2, instr_fun->frame()->virtual_registers_size());
}

//fun issue1(%n: i32): i16 {
//entry:
//    Br void out [L1:]
//L1:
//    %0 = Phi i32 i32 0, i32 %1 in [entry:, L2:]
//    %2 = Phi i16 i16 1, i16 %3 in [entry:, L2:]
//    %4 = ICmp u8 i32 %0, i32 %n <sle>
//    Br void u8 %4 out [L2:, L3:]
//L2:
//    %3 = Add i16 i16 %2, i16 3
//    %1 = Add i32 i32 %0, i32 1
//    Br void out [L1:]
//L3:
//    Ret void i16 %2
//} // issue09:issue09.issue1
TEST_F(Arm64InstructionSelectorTest, PhiNodesAndLoop) {
    auto fun = NewFun("foo", "fun (i32)->(i32)", {ir::Types::Int32}, {ir::Types::Int32});
    auto entry = fun->entry();
    auto param0 = fun->paramater(0);
    
    auto l1 = fun->NewBlock(nullptr);
    auto l2 = fun->NewBlock(nullptr);
    auto l3 = fun->NewBlock(nullptr);
    entry->NewNode(kUnknown, ir::Types::Void, ops()->Br(0, 1), l1);
    entry->LinkTo(l1);
    
    auto zero = ir::Value::New(arena(), kUnknown, ir::Types::Int32, ops()->I32Constant(0));
    auto one = ir::Value::New(arena(), kUnknown, ir::Types::Int32, ops()->I32Constant(1));
    
    auto phi0 = l1->NewNode(kUnknown, ir::Types::Int32, ops()->Phi(2, 2), zero, zero, entry, l2);
    auto phi1 = l1->NewNode(kUnknown, ir::Types::Int32, ops()->Phi(2, 2), one, one, entry, l2);
    auto cond = l1->NewNode(kUnknown, ir::Types::UInt8, ops()->ICmp(ir::ICondition::sle), phi0, param0);
    l1->NewNode(kUnknown, ir::Types::Void, ops()->Br(1, 2), cond, l2, l3);
    l1->LinkTo(l2);
    l1->LinkTo(l3);
    
    auto three = ir::Value::New(arena(), kUnknown, ir::Types::Int32, ops()->I32Constant(3));
    auto ret1 = l2->NewNode(kUnknown, ir::Types::Int32, ops()->Add(), phi1, three);
    phi1->Replace(arena(), 1, one, ret1);
    auto ret0 = l2->NewNode(kUnknown, ir::Types::Int32, ops()->Add(), phi0, one);
    phi0->Replace(arena(), 1, zero, ret0);
    l2->NewNode(kUnknown, ir::Types::Void, ops()->Br(0, 1), l1);
    l2->LinkTo(l1);
    
    l3->NewNode(kUnknown, ir::Types::Void, ops()->Ret(1), phi1);
    
    auto instr_fun = Arm64SelectFunctionInstructions(arena(), linkage(), fun);
    ASSERT_NE(nullptr, instr_fun);

    RegisterAllocator allocator(arena(), regconf_, instr_fun);
    allocator.Prepare();
    allocator.ComputeBlocksOrder();
    ASSERT_EQ(4, allocator.ordered_blocks_size());
    for (int i = 0; i < allocator.ordered_blocks_size(); i++) {
        EXPECT_EQ(i, allocator.OrderedBlockAt(i)->label());
    }
    
    allocator.NumberizeAllInstructions();
    {
        std::string buf;
        base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
        instr_fun->PrintTo(&printer);

        printf("%s", buf.c_str());
    }
    allocator.ComputeLocalLiveSets();
    allocator.ComputeGlobalLiveSets();
    
    auto state = allocator.BlockLivenssStateOf(allocator.OrderedBlockAt(0));
    EXPECT_FALSE(state->DoesInLiveIn(instr_fun->frame()->GetVirtualRegister(param0)));
    EXPECT_TRUE(state->DoesInLiveOut(instr_fun->frame()->GetVirtualRegister(param0)));

    EXPECT_TRUE(state->DoesInLiveIn(instr_fun->frame()->GetVirtualRegister(ret0)));
    EXPECT_TRUE(state->DoesInLiveIn(instr_fun->frame()->GetVirtualRegister(ret1)));
    EXPECT_TRUE(state->DoesInLiveIn(instr_fun->frame()->GetVirtualRegister(zero)));
    EXPECT_TRUE(state->DoesInLiveIn(instr_fun->frame()->GetVirtualRegister(one)));
    
    state = allocator.BlockLivenssStateOf(allocator.OrderedBlockAt(3));
    EXPECT_FALSE(state->DoesInLiveIn(instr_fun->frame()->GetVirtualRegister(param0)));
    EXPECT_FALSE(state->DoesInLiveIn(instr_fun->frame()->GetVirtualRegister(ret0)));
    EXPECT_FALSE(state->DoesInLiveIn(instr_fun->frame()->GetVirtualRegister(ret1)));
    EXPECT_FALSE(state->DoesInLiveIn(instr_fun->frame()->GetVirtualRegister(phi0)));
    EXPECT_TRUE(state->DoesInLiveIn(instr_fun->frame()->GetVirtualRegister(phi1)));

    //==================================================================================================================
    // Build Interval
    allocator.BuildIntervals();
    
    auto interval = allocator.IntervalOf(param0);
    EXPECT_EQ(10, interval->range(0).from);
    EXPECT_EQ(10, interval->range(0).to);
    
    EXPECT_EQ(0, interval->range(1).from);
    EXPECT_EQ(6, interval->range(1).to);
    
    interval = allocator.IntervalOf(phi0);
    EXPECT_EQ(14, interval->range(0).from);
    EXPECT_EQ(16, interval->range(0).to);
    
    EXPECT_EQ(10, interval->range(1).from);
    EXPECT_EQ(14, interval->range(1).to);
    
    EXPECT_EQ(6, interval->range(2).from);
    EXPECT_EQ(10, interval->range(2).to);
    
    
    //==================================================================================================================
    // Walk Intervals
    allocator.WalkIntervals();
    
    interval = allocator.IntervalOf(param0);
    ASSERT_NE(nullptr, interval);
    ASSERT_TRUE(interval->has_assigned_gp_register());
    EXPECT_EQ(0, interval->assigned_operand());
    
    interval = allocator.IntervalOf(phi0);
    ASSERT_TRUE(interval->has_assigned_gp_register());
    EXPECT_EQ(0, interval->assigned_operand());
    
    interval = allocator.IntervalOf(phi1);
    ASSERT_TRUE(interval->has_assigned_gp_register());
    EXPECT_EQ(1, interval->assigned_operand());
    
    interval = allocator.IntervalOf(ret0);
    ASSERT_TRUE(interval->has_assigned_gp_register());
    EXPECT_EQ(5, interval->assigned_operand());
    
    //==================================================================================================================
    // Finalize
    allocator.AssignRegisters();
    ASSERT_TRUE(true);
    
    {
        std::string buf;
        base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
        instr_fun->PrintTo(&printer);
        
        printf("%s", buf.c_str());
    }
}

} // namespace backend

} // namespace yalx
