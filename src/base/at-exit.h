#pragma once
#ifndef YALX_BASE_AT_EXIT_H_
#define YALX_BASE_AT_EXIT_H_

#include <mutex>

namespace yalx {

namespace base {

class AtExit {
public:
    enum Linker {
        INITIALIZER
    };
    using Callback = void (*)(void *);
    
    explicit AtExit(Linker);
    ~AtExit();

    static AtExit *This();
    
    void Register(Callback callback, void *params);
private:
    struct Hook;
    
    AtExit *prev_ = nullptr;
    Hook *hook_ = nullptr;
    std::mutex mutex_;
}; // class AtExit

} // namespace base

} // namespace yalx

#endif // YALX_BASE_AT_EXIT_H_
