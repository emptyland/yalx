#include "backend/stackslot-allocator.h"
#include "base/bit-ops.h"

namespace yalx {
namespace backend {

StackConfiguration::StackConfiguration(AddressingMode addressing_mode,
                                       uint32_t saved_size,
                                       uint32_t slot_alignment_size,
                                       uint32_t stack_alignment_size,
                                       int id_of_fp,
                                       int id_of_sp)
: addressing_mode_(addressing_mode)
, saved_size_(saved_size)
, slot_alignment_size_(slot_alignment_size)
, stack_alignment_size_(stack_alignment_size)
, id_of_fp_(id_of_fp)
, id_of_sp_(id_of_sp) {
}

StackSlotAllocator::StackSlotAllocator(const StackConfiguration *conf, base::Arena *arena)
: conf_(conf)
, arena_(arena)
, max_stack_size_(conf->saved_size())
, stack_size_(conf->saved_size()) {

}

LocationOperand *StackSlotAllocator::AllocateValSlot(size_t size, ir::Model *model) {
    return Allocate(size, false/*is_ref*/, model);
}

LocationOperand *StackSlotAllocator::AllocateRefSlot() {
    return Allocate(kPointerSize, true/*is_ref*/, nullptr/*model*/);
}

void StackSlotAllocator::FreeSlot(LocationOperand *operand) {
    assert(!slots_.empty());
    auto iter = std::find_if(slots_.begin(), slots_.end(),
                             [operand](const auto &slot) {return slot.operand == operand;} );
    assert(iter != slots_.end());
    MarkUnused(-operand->k() - iter->size, iter->size);
    if (operand->k() == stack_size_) { // The top one
        stack_size_ -= iter->size;
    }
    slots_.erase(iter);
}

LocationOperand *StackSlotAllocator::Allocate(size_t size, bool is_ref, ir::Model *model) {
    StackSlot slot;
    slot.size    = RoundUp(size, conf_->slot_alignment_size());
    slot.is_ref  = is_ref;
    slot.model   = model;
    
    int offset = 0;
    auto fit = FindFirstFitSpace(&offset, slot.size);
    if (!fit) {
        offset = stack_size_;
    }
    slot.operand = new (arena_) LocationOperand(conf_->addressing_mode(), conf_->id_of_fp(), -1, -offset - slot.size);
    if (fit) {
        for (auto iter = slots_.begin(); iter != slots_.end(); iter++) {
            if (slot.operand->k() < iter->operand->k()) {
                slots_.insert(iter + 1, slot);
            }
        }
    } else {
        InsertSlot(slot);
    }
    return slot.operand;
}

bool StackSlotAllocator::FindFirstFitSpace(int *offset, int size) const {
    for (int i = 0; i < bitmap_.size(); i++) {
        auto fz = base::Bits::FindFirstZero32(bitmap_[i]);
        if (fz >= 0 && fz < 32) {
            auto required_size = size;
            auto maybe_fit = i * 32 + fz;
            for (auto j = maybe_fit; j < bitmap_.size() * 32; j++) {
                if (TestBit(j)) {
                    required_size -= conf_->slot_alignment_size();
                }
                if (required_size <= 0) {
                    *offset = maybe_fit * conf_->slot_alignment_size();
                    return *offset + size <= stack_size_;
                }
            }
        }
    }
    return false;
}

} // namespace backend
} // namespace yalx
