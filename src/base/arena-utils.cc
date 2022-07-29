#include "base/arena-utils.h"

namespace yalx {
    
namespace base {

class AreanStringPool {
public:
    AreanStringPool(Arena *arena): pool_(arena) {}
    
    static uintptr_t Uniquely() {
        static int dummy = 0;
        return reinterpret_cast<uintptr_t>(&dummy);
    }
    
    inline void Init() {}
    
    ArenaString *FindOrNull(std::string_view key) {
        if (auto iter = pool_.find(key); iter != pool_.end()) {
            return iter->second;
        } else {
            return nullptr;
        }
    }
    
    void Insert(ArenaString *s) { pool_[s->ToSlice()] = s; }
    
private:
    ArenaUnorderedMap<std::string_view, ArenaString *> pool_;
}; // class AreanStringPool
    
static const char empty_arean_string_stub[sizeof(ArenaString)] = {0};
    
/*static*/ const ArenaString *const ArenaString::kEmpty =
    reinterpret_cast<const ArenaString *>(empty_arean_string_stub);
    
/*static*/ ArenaString *ArenaString::New(base::Arena *arena, const char *s,
                                         size_t n) {
    auto pool = arena->Industry<AreanStringPool>();
    if (n < kMaxPoolStringLen) {
        if (auto in_pool = pool->FindOrNull(std::string_view(s, n))) {
            return in_pool;
        }
    }
    
    void *chunk = arena->Allocate(n + 1 + sizeof(ArenaString));
    if (!chunk) {
        return nullptr;
    }
    auto obj = new (chunk) ArenaString(s, n);
    if (n < kMaxPoolStringLen) {
        pool->Insert(obj);
    }
    return obj;
}
    
ArenaString::ArenaString(const char *s, size_t n)
    : hash_val_(Hash::Js(s, n))
    , len_(static_cast<uint32_t>(n)) {
    ::memcpy(buf_, s, n);
    buf_[n] = '\0';
}
    
ArenaString::ArenaString(size_t n)
    : hash_val_(0)
    , len_(static_cast<uint32_t>(n)) {
    buf_[n] = '\0';
}
    
} // namespace base
    
} // namespace yalx
