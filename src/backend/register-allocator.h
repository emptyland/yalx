#pragma once
#ifndef YALX_BACKEND_REGISTER_ALLOCATOR_H_
#define YALX_BACKEND_REGISTER_ALLOCATOR_H_

#include "base/checking.h"
#include "base/base.h"
#include <set>

namespace yalx {
namespace base {
class Arena;
} // namespace base
namespace ir {
class Function;
} // namespace ir
namespace backend {


class InstructionFunction;
class InstructionBlock;
class RegistersConfiguration;
class Frame;


struct LivePosition {
    int label = -1;
    int step  = -1;
    
    bool IsValid() const { return !IsInvliad(); }
    bool IsInvliad() const { return label == -1 && step == -1; }
};

struct TopTierLiveRange {
    LivePosition used_at_start;
    LivePosition used_at_end;
    
    void Update(int label, int step) {
        if (used_at_start.IsInvliad()) {
            used_at_start.label = label;
            used_at_start.step  = step;
        } else {
            used_at_end.label = label;
            used_at_end.step  = step;
        }
    }
};

class RegisterAllocator final {
public:
    RegisterAllocator(base::Arena *arena, const RegistersConfiguration *regconf, InstructionFunction *fun);
    ~RegisterAllocator();
    void Prepare();
    
    void ScanLiveRange();
private:
    void ScanBlockGraph(InstructionBlock *block);
    
    void Define(int virtual_register, int label, int step);
    void Use(int virtual_register, int label, int step);
    
    bool TryTrace(InstructionBlock *block) {
        if (tracing_.find(block) != tracing_.end()) {
            return true;
        }
        tracing_.insert(block);
        return false;
    }
    
    base::Arena *const arena_;
    const RegistersConfiguration *const regconf_;
    InstructionFunction *const fun_;
    TopTierLiveRange *live_ranges_ = nullptr;
    std::set<int> allocatable_gp_registers_;
    std::set<int> allocatable_fp_registers_;
    std::set<int> activity_gp_registers_;
    std::set<int> activity_fp_registers_;
    std::set<InstructionBlock *> tracing_;
}; // class RegisterAllocator

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_REGISTER_ALLOCATOR_H_
