#include "backend/register-allocator.h"
#include "backend/arm64/instruction-generating-arm64.h"
#include "ir/base-test.h"
#include <gtest/gtest.h>

namespace yalx {
namespace backend {

class RegisterAllocatorTest : public ir::BaseTest {
public:
    RegisterAllocatorTest(): registers_(Arm64RegisterConf(), arena()) {}

protected:
    RegisterAllocator registers_;
}; // class RegisterAllocatorTest


TEST_F(RegisterAllocatorTest, Sanity) {
    auto r1 = registers_.AllocateRegister(MachineRepresentation::kWord32);
    ASSERT_NE(nullptr, r1);
    ASSERT_EQ(MachineRepresentation::kWord32, r1->rep());
    ASSERT_EQ(0, r1->register_id());
    
    registers_.FreeRegister(r1);
    r1 = registers_.AllocateRegister(MachineRepresentation::kWord32);
    ASSERT_NE(nullptr, r1);
    ASSERT_EQ(MachineRepresentation::kWord32, r1->rep());
    ASSERT_EQ(0, r1->register_id());
}


} // namespace backend
} // namespace yalx
