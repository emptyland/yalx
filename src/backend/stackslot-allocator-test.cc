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
    auto slot1 = slots_.AllocateValSlot(1, nullptr);
    ASSERT_EQ(Arm64Mode_MRI, slot1->mode());
    ASSERT_EQ(29, slot1->register0_id());
    ASSERT_EQ(-1, slot1->register1_id());
    ASSERT_EQ(-36, slot1->k());
}

} // namespace backend
} // namespace yalx
