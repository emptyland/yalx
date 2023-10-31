#pragma once
#ifndef YALX_VM_ISOLATE_H_
#define YALX_VM_ISOLATE_H_

#include "base/checking.h"
#include "base/status.h"
#include "base/base.h"

namespace yalx {

namespace vm {

class Isolate final {
public:
    Isolate();
    ~Isolate();
    
    base::Status Initialize();
    
    static Isolate *Get() { return DCHECK_NOTNULL(vm_); }
    
private:
    void Enter();
    void Exit();
    
    static Isolate *vm_;
}; // class Isolate



} // namespace vm

} // namespace yalx

#endif // YALX_VM_ISOLATE_H_
