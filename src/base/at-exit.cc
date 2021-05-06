#include "base/at-exit.h"
#include "base/base.h"

namespace yalx {

namespace {

base::AtExit *top = nullptr;

} // namespace

namespace base {

struct AtExit::Hook {
    Hook *next;
    Callback callback;
    void *params;
};

AtExit::AtExit(Linker)
    : prev_(top) {
    //DCHECK_NE(top, this);
    top = this;
}

AtExit::~AtExit() {
    //DCHECK_EQ(top, this);
    Hook *p = nullptr;
    while (hook_) {
        hook_->callback(hook_->params);
        p = hook_;
        hook_ = hook_->next;
        delete p;
    }
    top = prev_;
}

/*static*/ AtExit *AtExit::This() { return top; }

void AtExit::Register(Callback callback, void *params) {
    std::lock_guard<std::mutex> lock(mutex_);
    hook_ = new Hook{hook_, callback, params};
}

} // namespace base

} // namespace yalx
