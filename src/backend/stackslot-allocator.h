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
class Model;
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
    int size: 30;
    int is_ref: 2;
    ir::Model *model;
};

class StackSlotAllocator final {
public:
    StackSlotAllocator(const StackConfiguration *conf, base::Arena *arena);
    
    LocationOperand *AllocateValSlot(size_t size, ir::Model *model = nullptr);
    LocationOperand *AllocateRefSlot();

    void FreeSlot(LocationOperand *operand);
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(StackSlotAllocator);
private:
    LocationOperand *Allocate(size_t size, bool is_ref, ir::Model *model);
    // int LatestOffset() const;
    
    void InsertSlot(const StackSlot &slot) {
        stack_size_ += slot.size;
        for (auto iter = slots_.begin(); iter != slots_.end(); iter++) {
            if (slot.operand->k() < iter->operand->k()) {
                slots_.insert(iter + 1, slot);
            }
        }
        UpdateMaxStackSize(slot.operand, slot.size);
    }

    void UpdateMaxStackSize(const LocationOperand *operand, int size) {
        assert(operand->k() < 0);
        auto new_top = RoundUp(-operand->k() + size, conf_->stack_alignment_size());
        if (new_top > max_stack_size_) {
            max_stack_size_ = new_top;
        }
    }
    
    bool FindFirstFitSpace(int *offset, int size) const;
    
    void MarkUnused(int offset, int size) {
        assert(offset >= 0);
        assert(size > 0);
        auto begin = offset / conf_->slot_alignment_size();
        auto end   = begin + size / conf_->slot_alignment_size();
        for (int i = begin; i < end; i++) {
            ClearBit(i);
        }
    }

    void ClearBit(int index) {
        assert(index >= 0);
        assert(index < bitmap_.size() * 32);
        bitmap_[index / 32] &= ~(1u >> (index % 32));
    }
    
    bool TestBit(int index) const {
        assert(index >= 0);
        assert(index < bitmap_.size() * 32);
        return bitmap_[index / 32] & (1u >> (index % 32));
    }
    
    const StackConfiguration *const conf_;
    base::Arena *const arena_;
    uint32_t max_stack_size_; // In bytes
    uint32_t stack_size_; // Current stack size in bytes
    std::vector<StackSlot> slots_;
    /** Bitmap of used memory graph */
    std::vector<uint32_t>  bitmap_; // 1 bit = 4 bytes
}; // class StackSlotAllocator

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_STACKSLOT_ALLOCATOR_H_
