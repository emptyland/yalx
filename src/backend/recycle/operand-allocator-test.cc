#include "backend/operand-allocator.h"
#include "backend/arm64/instruction-generating-arm64.h"
#include "ir/base-test.h"
#include <gtest/gtest.h>

namespace yalx {
namespace backend {

class OperandAllocatorTest : public ir::BaseTest {
public:
    OperandAllocatorTest()
    : operands_(Arm64StackConf(), Arm64RegisterConf(), OperandAllocator::kRegisterFirst, arena()) {}
    
protected:
    OperandAllocator operands_;
}; // class StackSlotAllocatorTest

TEST_F(OperandAllocatorTest, Sanity) {
    auto opd = operands_.Allocate(OperandAllocator::kVal, 4);
    ASSERT_TRUE(nullptr != opd);
    ASSERT_TRUE(opd->IsRegister());
    ASSERT_EQ(MachineRepresentation::kWord32, opd->AsRegister()->rep());
    ASSERT_EQ(0, opd->AsRegister()->register_id());
    
    opd = operands_.Allocate(OperandAllocator::kRef, kPointerSize);
    ASSERT_TRUE(opd->IsRegister());
    ASSERT_EQ(MachineRepresentation::kWord64, opd->AsRegister()->rep());
    ASSERT_EQ(1, opd->AsRegister()->register_id());
}

TEST_F(OperandAllocatorTest, Allocation) {
    auto opd = operands_.AllocateStackSlot(OperandAllocator::kVal, 4, 0, StackSlotAllocator::kFit);
    ASSERT_TRUE(opd->IsLocation());
    ASSERT_EQ(Arm64Mode_MRI, opd->AsLocation()->mode());
    ASSERT_EQ(29, opd->AsLocation()->register0_id());
    ASSERT_EQ(-4, opd->AsLocation()->k());
    
    auto r0 = operands_.AllocateReigster(OperandAllocator::kRef, kPointerSize);
    ASSERT_TRUE(r0->IsRegister());
    ASSERT_EQ(MachineRepresentation::kWord64, r0->AsRegister()->rep());
    ASSERT_EQ(0, r0->AsRegister()->register_id());
}

} // namespace backend
} // namespace yalx
