#pragma once
#ifndef YALX_BASE_ENV_H_
#define YALX_BASE_ENV_H_

#include "base/status.h"
#include "base/checking.h"
#include "base/base.h"

#if defined(YALX_OS_DARWIN)
#include <sys/mman.h>
#endif // defined(YALX_OS_DARWIN)

#include <memory>

namespace yalx {

namespace base  {

class SequentialFile;
class Env;

class OSPageMemory {
public:
    OSPageMemory(OSPageMemory &&other)
        : OSPageMemory(other.chunk(), other.size()) {
        other.chunk_ = nullptr;
        other.size_ = 0;
    }
    inline ~OSPageMemory();
    
    DEF_VAL_GETTER(Address, chunk);
    DEF_VAL_GETTER(size_t, size);
    
    bool is_valid() const { return chunk_ != reinterpret_cast<Address>(-1) && size_ > 0; }
    
    template<class T> inline T *WriteTo(const std::string &buf) { return reinterpret_cast<T *>(Write(buf)); }
    
    Address Write(const std::string &buf) {
        assert(pos_ < size_);
        ::memcpy(chunk_ + pos_, &buf[0], buf.size());
        auto space = (chunk_ + pos_);
        pos_ += buf.size();
        return space;
    }
    
    friend class Env;
private:
    OSPageMemory(Address chunk, size_t size): chunk_(DCHECK_NOTNULL(chunk)), size_(size) {}

    Address chunk_;
    size_t size_;
    size_t pos_ = 0;
}; // class OSPageMemory


class Env {
public:
    static constexpr int kMemoryNone = 0;
    static constexpr int kMemoryReadable = 0x1;
    static constexpr int kMemoryWriteable = 0x2;
    static constexpr int kMemoryExecuteable = 0x4;
    
    static constexpr int kMemoryWR = kMemoryReadable | kMemoryWriteable;
    static constexpr int kMemoryWRX = kMemoryReadable | kMemoryWriteable | kMemoryExecuteable;
    
    static int kOSPageSize;
    
    // Global env initialize.
    static Status Init();
    
    // std::unique_ptr<SequentialFile> file;
    // auto rs = Env::NewSequentialFile("file", &file);
    // if (rs.fail()) {
    //     return rs;
    // }
    static Status NewSequentialFile(const std::string &name, std::unique_ptr<SequentialFile> *file);
    
    static OSPageMemory OSPageAllocate(size_t n, int access) {
        n = RoundUp(n, kOSPageSize);
        void *memory = nullptr;
        if (auto rs = OSPageAllocate(n, access, &memory); rs.fail()) {
            return OSPageMemory{reinterpret_cast<Address>(-1), 0};
        }
        return OSPageMemory{static_cast<Address>(memory), n};
    }
    
    // Allocate one or more pages from os.
    static inline Status OSPageAllocate(size_t n, int access, void **result);
    
    // Free one or moer pages to os.
    static inline Status OSPageFree(void *chunk, size_t n);
};


inline OSPageMemory::~OSPageMemory() {
    if (size_ > 0) {
        Env::OSPageFree(chunk_, size_);
    }
}


#if defined(YALX_OS_POSIX)

inline Status Env::OSPageAllocate(size_t n, int access, void **result) {
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
    *result = chunk;
    return Status::OK();
}

inline Status Env::OSPageFree(void *chunk, size_t n) {
    if (::munmap(chunk, n) < 0) {
        return ERR_CORRUPTION("munmap fail!");
    }
    return Status::OK();
}

#endif // defined(YALX_OS_POSIX)

} // namespace base

} // namespace yalx



#endif // YALX_BASE_ENV_H_
