#pragma once
#ifndef YALX_BACKEND_STACKSLOT_ALLOCATOR_H_
#define YALX_BACKEND_STACKSLOT_ALLOCATOR_H_

#include "backend/instruction.h"
#include "base/arena-utils.h"
#include "base/base.h"
#include <vector>

namespace yalx {
namespace ir {
class Function;
class Value;
} // namespace ir
namespace backend {


class StackConfiguration final {
public:
    StackConfiguration(AddressingMode addressing_mode,
                       uint32_t saved_size,
                       uint32_t slot_alignment_size,
                       uint32_t stack_alignment_size,
                       int id_of_fp,
                       int id_of_sp);
    
    DEF_VAL_GETTER(AddressingMode, addressing_mode);
    DEF_VAL_GETTER(uint32_t, saved_size);
    DEF_VAL_GETTER(uint32_t, slot_alignment_size);
    DEF_VAL_GETTER(uint32_t, stack_alignment_size);
    DEF_VAL_GETTER(int, id_of_fp);
    DEF_VAL_GETTER(int, id_of_sp);
    
private:
    const AddressingMode addressing_mode_;
    const uint32_t saved_size_;
    const uint32_t slot_alignment_size_;
    const uint32_t stack_alignment_size_;
    const int id_of_fp_;
    const int id_of_sp_;
}; // class StackConfiguration

struct StackSlot {
    LocationOperand *operand;
    uint32_t bitmap[4];
    int      size;
};

class StackSlotAllocator final {
public:
    StackSlotAllocator(const StackConfiguration *conf, base::Arena *arena);
    
    LocationOperand *AllocateSlot(size_t size);
    void FreeSlot(LocationOperand *operand);
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(StackSlotAllocator);
private:
    int LatestOffset() const;
    
    void Push(const StackSlot &slot) {
        slots_.push_back(slot);
        UpdateMaxStackSize(slot.operand, slot.size);
    }

    void UpdateMaxStackSize(const LocationOperand *operand, int size) {
        assert(operand->k() < 0);
        auto new_top = RoundUp(-operand->k() + size, conf_->stack_alignment_size());
        if (new_top > max_stack_size_) {
            max_stack_size_ = new_top;
        }
    }
    
    const StackConfiguration *const conf_;
    base::Arena *const arena_;
    uint32_t max_stack_size_; // In bytes
    std::vector<StackSlot> slots_;
}; // class StackSlotAllocator

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_STACKSLOT_ALLOCATOR_H_
