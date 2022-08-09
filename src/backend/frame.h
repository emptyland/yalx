#pragma once
#ifndef YALX_BACKEND_FRAME_H_
#define YALX_BACKEND_FRAME_H_

#include "base/arena-utils.h"
#include "base/checking.h"
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
    
    size_t virtual_registers_size() const { return virtual_registers_.size(); }
    
    int AllocateSlot(size_t size_in_bytes, size_t padding_size) {
        //int origin = stack_size();
        stack_size_ += (padding_size + RoundUp(size_in_bytes, kSlotAlignmentSize));
        DCHECK(stack_size() % kSlotAlignmentSize == 0);
        return -stack_size();
    }

    int GetVirtualRegister(ir::Value *value);
    
    ir::Value *GetValue(int vid) const;
    
    void SetRename(ir::Value *value, ir::Value *rename) {
        auto vid = GetVirtualRegister(value);
        if (vid >= virtual_register_rename_.size()) {
            virtual_register_rename_.resize(vid + 1, -1);
        }
        virtual_register_rename_[vid] = GetVirtualRegister(rename);
    }
    
    int GetRename(int vid) {
        int rename = vid;
        while (true) {
            if (static_cast<size_t>(rename) >= virtual_register_rename_.size()) {
                break;
            }
            int next = virtual_register_rename_[rename];
            if (next == -1) {
                break;
            }
            rename = next;
        }
        return rename;
    }
    
    int NextVirtualRegister() { return next_virtual_register_++; }
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Frame);
private:
    base::ArenaVector<ir::Value *> virtual_registers_;
    base::ArenaVector<int> virtual_register_rename_;
    ir::Function *fun_;
    int next_virtual_register_ = 0;
    int returning_val_size_ = 0;
    int stack_size_ = 0;
}; // class Frame

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_FRAME_H_
