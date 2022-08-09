#include "backend/frame.h"


namespace yalx {

namespace backend {

Frame::Frame(base::Arena *arena, ir::Function *fun)
: virtual_registers_(arena)
, virtual_register_rename_(arena)
, fun_(fun) {
    
}

int Frame::GetVirtualRegister(ir::Value *value) {
    if (auto iter = std::find(virtual_registers_.begin(), virtual_registers_.end(), value);
        iter != virtual_registers_.end()) {
        return static_cast<int>(iter - virtual_registers_.begin());
    }
    
    int vid = NextVirtualRegister();
    if (vid >= virtual_registers_.size()) {
        virtual_registers_.resize(vid + 1, nullptr);
    }
    virtual_registers_[vid] = value;
    return vid;
}


} // namespace backend

} // namespace yalx
