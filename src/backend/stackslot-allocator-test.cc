#include "backend/stackslot-allocator.h"
#include "backend/arm64/instruction-generating-arm64.h"
#include "ir/base-test.h"
#include <gtest/gtest.h>

namespace yalx {
namespace backend {

class StackSlotAllocatorTest : public ir::BaseTest {
public:
    StackSlotAllocatorTest()
    : slots_(Arm64StackConf(), arena()) {}

protected:
    StackSlotAllocator slots_;
}; // class StackSlotAllocatorTest

TEST_F(StackSlotAllocatorTest, Sanity) {
    auto slot1 = slots_.AllocateValSlot(1, 0);
    ASSERT_EQ(Arm64Mode_MRI, slot1->mode());
    ASSERT_EQ(29, slot1->register0_id());
    ASSERT_EQ(-1, slot1->register1_id());
    ASSERT_EQ(-4, slot1->k());
}

TEST_F(StackSlotAllocatorTest, Allocate) {
    auto slot1 = slots_.AllocateRefSlot(0);
    ASSERT_EQ(Arm64Mode_MRI, slot1->mode());
    ASSERT_EQ(29, slot1->register0_id());
    ASSERT_EQ(-8, slot1->k());
    
    auto slot2 = slots_.AllocateRefSlot(0);
    ASSERT_EQ(-16, slot2->k());
    
    slots_.FreeSlot(slot1);
    auto slot3 = slots_.AllocateRefSlot(0);
    ASSERT_EQ(-8, slot3->k());
}

TEST_F(StackSlotAllocatorTest, Free) {
    LocationOperand *slots[3];
    slots[0] = slots_.AllocateRefSlot(0);
    slots[1] = slots_.AllocateValSlot(8, 0);
    slots[2] = slots_.AllocateRefSlot(0);
    
    for (size_t i = 0; i < arraysize(slots); i++) {
        slots_.FreeSlot(slots[i]);
    }

    auto slot0 = slots_.AllocateRefSlot(0);
    ASSERT_EQ(slots[0]->k(), slot0->k());
}

} // namespace backend
} // namespace yalx
