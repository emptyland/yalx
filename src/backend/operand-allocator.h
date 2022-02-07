#pragma once
#ifndef YALX_BACKEND_OPERAND_ALLOCATOR_H_
#define YALX_BACKEND_OPERAND_ALLOCATOR_H_

#include "backend/stackslot-allocator.h"
#include "backend/register-allocator.h"

namespace yalx {
namespace backend {

class OperandAllocator final {
public:
    enum Policy {
        kStackOnly,
        kRegisterFirst,
    };
    
    enum OperandMark {
        kVal,
        kPtr,
        kRef,
    };
    

    OperandAllocator(const StackConfiguration *sconf,
                     const RegisterConfiguration *rconf,
                     Policy policy,
                     base::Arena *arena);
    
    void Prepare(ir::Function *fun);
    
    InstructionOperand *Allocate(ir::Value *value);
    InstructionOperand *Allocate(OperandMark mark, size_t size, ir::Model *model = nullptr);
    
    LocationOperand *AllocateStackSlot(ir::Value *value, StackSlotAllocator::Policy policy);
    LocationOperand *AllocateStackSlot(OperandMark mark, size_t size,
                                       StackSlotAllocator::Policy policy,
                                       ir::Model *model = nullptr);

    void Free(ir::Value *value) {
        if (auto iter = allocated_.find(value); iter != allocated_.end()) {
            Free(iter->second);
            allocated_.erase(iter);
        }
    }
    void Free(InstructionOperand *operand);
    
    void ReleaseDeads(int position);
    
    void Move(ir::Value *value, InstructionOperand *operand) {
        auto iter = allocated_.find(value);
        assert(iter != allocated_.end());
        Free(iter->second);
        allocated_[value] = operand;
    }
    
    struct LiveRange {
        int start_position = -1;
        int stop_position  = -1;
    };
    
    struct LiveRecord {
        size_t index;
        size_t size;
    };
private:
    void Alive(ir::Value *value, int ir_position) {
        if (auto iter = live_ranges_.find(value); iter == live_ranges_.end()) {
            live_ranges_[value] = LiveRange{ir_position, -1};
        }
    }
    
    void Dead(ir::Value *value, int ir_position) {
        if (auto iter = live_ranges_.find(value); iter != live_ranges_.end()) {
            if (iter->second.stop_position == -1) {
                iter->second.stop_position = ir_position;
            }
        }
    }

    const Policy policy_;
    base::Arena *const arena_;
    StackSlotAllocator slots_;
    RegisterAllocator registers_;
    std::map<ir::Value *, Instruction::Operand *> allocated_;
    std::map<ir::Value *, LiveRange> live_ranges_;
    std::vector<LiveRecord> dead_records_;
    std::vector<ir::Value *> deads_;
}; // class OperandAllocator

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_OPERAND_ALLOCATOR_H_
