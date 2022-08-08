#pragma once
#ifndef YALX_BACKEND_FRAME_H_
#define YALX_BACKEND_FRAME_H_

#include "base/arena-utils.h"
#include "base/arena.h"
#include "base/base.h"

namespace yalx {
namespace ir {
class Value;
class Function;
} // namespace ir
namespace backend {

class Frame final : public base::ArenaObject {
public:
    static constexpr size_t kSlotAlignmentSize = 4;
    static constexpr size_t kStackAlignmentSize = 16;
    static constexpr size_t kCalleeReservedSize = 16;
    
    Frame(base::Arena *arena, ir::Function *fun);
    
    DEF_PTR_GETTER(ir::Function, fun);
    DEF_VAL_GETTER(int, stack_size);
    DEF_VAL_PROP_RW(int, returning_val_size);

    int GetVirtualRegister(ir::Value *value) {
        if (auto iter = std::find(virtual_registers_.begin(), virtual_registers_.end(), value);
            iter != virtual_registers_.end()) {
            return static_cast<int>(iter - virtual_registers_.begin());
        } else {
            virtual_registers_.push_back(value);
            return static_cast<int>(virtual_registers_.size()) - 1;
        }
    }
    
    ir::Value *GetValue(int vid) const;
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Frame);
private:
    base::ArenaVector<ir::Value *> virtual_registers_;
    ir::Function *fun_;
    int returning_val_size_ = 0;
    int stack_size_ = 0;
}; // class Frame

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_FRAME_H_
