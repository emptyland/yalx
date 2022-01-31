#include "backend/stackslot-allocator.h"

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
, max_stack_size_(conf->saved_size()) {
    
}

LocationOperand *StackSlotAllocator::AllocateSlot(size_t size) {
    StackSlot slot;
    slot.size    = RoundUp(size, conf_->slot_alignment_size());
    slot.operand = new (arena_) LocationOperand(conf_->addressing_mode(),
                                                conf_->id_of_fp(), -1,
                                                LatestOffset() - slot.size);
    Push(slot);
    return slot.operand;
}

void StackSlotAllocator::FreeSlot(LocationOperand *operand) {
    assert(!slots_.empty());
    assert(slots_.back().operand == operand);
    slots_.resize(slots_.size() - 1);
}

int StackSlotAllocator::LatestOffset() const {
    return slots_.empty() ? -conf_->saved_size() : (slots_.back().operand->k() - slots_.back().size);
}

} // namespace backend
} // namespace yalx
