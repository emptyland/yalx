#pragma once
#ifndef YALX_BASE_ENV_H_
#define YALX_BASE_ENV_H_

#include "base/status.h"
#include "base/base.h"

#if defined(YALX_OS_DARWIN)
#include <sys/mman.h>
#endif // defined(YALX_OS_DARWIN)

#include <memory>

namespace yalx {

namespace base  {

class SequentialFile;

class Env {
public:
    static constexpr int kMemoryReadable = 0x1;
    static constexpr int kMemoryWriteable = 0x2;
    static constexpr int kMemoryExecuteable = 0x4;
    
    // std::unique_ptr<SequentialFile> file;
    // auto rs = Env::NewSequentialFile("file", &file);
    // if (rs.fail()) {
    //     return rs;
    // }
    static Status NewSequentialFile(const std::string &name, std::unique_ptr<SequentialFile> *file);
    
    static inline Status OSPagedAllocate(size_t n, int access, void **result);
    
    static inline Status OSPagedFree(void *chunk, size_t n);
};


#if defined(YALX_OS_POSIX)

inline Status Env::OSPagedAllocate(size_t n, int access, void **result) {
    int flags = MAP_ANON|MAP_PRIVATE;
#if defined(YALX_OS_DARWIN)
    if (access & (kMemoryWriteable | kMemoryExecuteable)) {
        flags |= MAP_JIT;
    }
#endif // defined(YALX_OS_DARWIN)
    auto chunk = ::mmap(nullptr, n, access, MAP_ANON|MAP_PRIVATE, -1, 0);
    if (chunk == MAP_FAILED) {
        return ERR_CORRUPTION("mmap fail!");
    }
    
    return Status::OK();
}

inline Status Env::OSPagedFree(void *chunk, size_t n) {
    if (::munmap(chunk, n) < 0) {
        return ERR_CORRUPTION("munmap fail!");
    }
    return Status::OK();
}

#endif // defined(YALX_OS_POSIX)

} // namespace base

} // namespace yalx



#endif // YALX_BASE_ENV_H_
