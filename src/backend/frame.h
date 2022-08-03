#pragma once
#ifndef YALX_BACKEND_FRAME_H_
#define YALX_BACKEND_FRAME_H_

#include "base/arena-utils.h"
#include "base/arena.h"
#include "base/base.h"

namespace yalx {
namespace ir {
class Value;
} // namespace ir
namespace backend {

class Frame final : public base::ArenaObject {
public:
    Frame(base::Arena *arena);

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
}; // class Frame

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_FRAME_H_
