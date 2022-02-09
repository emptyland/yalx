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
        kF32,
        kF64,
        kPtr,
        kRef,
    };
    
    constexpr static const int kAnyRegister = RegisterAllocator::kAny;
    

    OperandAllocator(const StackConfiguration *sconf,
                     const RegisterConfiguration *rconf,
                     Policy policy,
                     base::Arena *arena);
    
    StackSlotAllocator *slots() { return &slots_; }
    RegisterAllocator *registers() { return &registers_; }
    
    void Prepare(ir::Function *fun);
    
    InstructionOperand *Allocated(ir::Value *value) {
        if (auto iter = allocated_.find(value); iter != allocated_.end()) {
            return iter->second;
        } else {
            return nullptr;
        }
    }
    
    InstructionOperand *Allocate(ir::Value *value);
    InstructionOperand *Allocate(ir::Type ty);
    InstructionOperand *Allocate(OperandMark mark, size_t size, ir::Model *model = nullptr);
    
    LocationOperand *AllocateStackSlot(ir::Value *value, StackSlotAllocator::Policy policy);
    LocationOperand *AllocateStackSlot(OperandMark mark, size_t size,
                                       StackSlotAllocator::Policy policy,
                                       ir::Model *model = nullptr);
    
    RegisterOperand *AllocateReigster(ir::Value *value, int designate = kAnyRegister);
    RegisterOperand *AllocateReigster(OperandMark mark, size_t size, int designate = kAnyRegister);

    void Free(ir::Value *value) {
        if (auto iter = allocated_.find(value); iter != allocated_.end()) {
            Free(iter->second);
            allocated_.erase(iter);
        }
    }
    void Free(InstructionOperand *operand);
    
    void ReleaseDeads(int position);
    
    void Move(ir::Value *value, InstructionOperand *operand) {
        if (auto iter = allocated_.find(value); iter != allocated_.end()) {
            Free(iter->second);
        }
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
    InstructionOperand *TryAllocateRegisterFirst(ir::Value *value);
    
    void Alive(ir::Value *value, int ir_position) {
        if (auto iter = live_ranges_.find(value); iter == live_ranges_.end()) {
            live_ranges_[value] = LiveRange{ir_position, -1};
        } else {
            live_ranges_[value].start_position = std::min(ir_position, live_ranges_[value].start_position);
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
