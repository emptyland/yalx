#include "backend/operand-allocator.h"
#include "ir/metadata.h"
#include "ir/operator.h"
#include "ir/type.h"
#include "ir/node.h"

namespace yalx {
namespace backend {


OperandAllocator::OperandAllocator(const StackConfiguration *sconf,
                                   const RegisterConfiguration *rconf,
                                   Policy policy,
                                   base::Arena *arena)
: policy_(policy)
, arena_(arena)
, slots_(sconf, arena)
, registers_(rconf, arena) {
    
}

void OperandAllocator::Prepare(ir::Function *fun) {
    int position = 0;
    for (auto blk : fun->blocks()) {
        for (auto instr : blk->instructions()) {
            if (instr->type().kind() != ir::Type::kVoid) {
                Alive(instr, position);
            }
            for (int i = 0; i < instr->op()->value_in(); i++) {
                Alive(instr->InputValue(i), position);
            }
            position++;
        }
    }
    const auto total = position;
    for (int64_t i = fun->blocks_size() - 1; i >= 0; i--) {
        auto blk = fun->block(i);
        for (int64_t j = blk->instructions_size() - 1; j >= 0; j--) {
            auto instr = blk->instruction(j);
            if (instr->type().kind() != ir::Type::kVoid) {
                Dead(instr, position);
            }
            for (int k = 0; k < instr->op()->value_in(); k++) {
                Dead(instr->InputValue(k), position);
            }
            position--;
        }
    }
    assert(position == 0);
    std::unique_ptr<std::vector<ir::Value *>[]> deads(new std::vector<ir::Value *>[total + 1]);
    for (auto [val, range] : live_ranges_) {
        assert(range.stop_position >= 0);
        assert(range.stop_position < total + 1);
        
        deads[range.stop_position].push_back(val);
    }
    
    LiveRecord rd;
    rd.index = 0;
    rd.size  = 0;
    for (int i = 0; i < total + 1; i++) {
        rd.size = deads[i].size();
        dead_records_.push_back(rd);
        for (auto val : deads[i]) { deads_.push_back(val); }
        rd.index += rd.size;
    }
}

LocationOperand *OperandAllocator::AllocateStackSlot(ir::Value *value, StackSlotAllocator::Policy policy) {
    
}

LocationOperand *OperandAllocator::AllocateStackSlot(OperandMark mark, size_t size, StackSlotAllocator::Policy policy,
                                                     ir::Model *model) {
    switch (mark) {
        case kPtr:
            return slots_.AllocateValSlot(kPointerSize, policy, model);
        case kVal:
            return slots_.AllocateValSlot(size, policy, model);
        case kRef:
            return slots_.AllocateRefSlot(policy);
        default:
            UNREACHABLE();
            break;
    }
}


void OperandAllocator::ReleaseDeads(int position) {
    assert(position >= 0);
    assert(position < dead_records_.size());
    
    auto rd = dead_records_[position];
    for (size_t i = rd.index; i < rd.index + rd.size; i++) {
        auto key = deads_[i];
        if (auto iter = allocated_.find(key); iter != allocated_.end()) {
            auto operand = iter->second;
            if (operand->IsLocation()) {
                slots_.FreeSlot(static_cast<LocationOperand *>(operand));
            } else if (operand->IsRegister()) {
                // TODO:
                UNREACHABLE();
            } else {
                UNREACHABLE();
            }
        }
    }
}

} // namespace backend
} // namespace yalx
