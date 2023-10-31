#include "vm/isolate.h"
#include <mutex>

namespace yalx {

namespace vm {

Isolate *Isolate::vm_;
static std::mutex vm_scope_mutex;

Isolate::Isolate() {
    Enter();
}

Isolate::~Isolate() {
    Exit();
}

base::Status Isolate::Initialize() {
    // TODO:
    return base::Status::OK();
}

void Isolate::Enter() {
    std::unique_lock<std::mutex> lock(vm_scope_mutex);
    DCHECK(vm_ == nullptr);
    DCHECK(vm_ != this);
    vm_ = this;
}

void Isolate::Exit() {
    std::unique_lock<std::mutex> lock(vm_scope_mutex);
    DCHECK(vm_ == this);
    vm_ = nullptr;
}

} // namespace vm

} // namespace yalx
