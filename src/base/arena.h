#pragma once
#ifndef YALX_BASE_ARENA_H_
#define YALX_BASE_ARENA_H_

#include <map>
#include "base/base.h"

namespace yalx {
    
namespace base {

class Arena;

template <class T>
class ArenaAllocator {
public:
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    template <class O>
    struct rebind {
        typedef ArenaAllocator<O> other;
    };
    
    //#ifdef V8_CC_MSVC
    //    // MSVS unfortunately requires the default constructor to be defined.
    //    ZoneAllocator() : ZoneAllocator(nullptr) { UNREACHABLE(); }
    //#endif
    explicit ArenaAllocator(Arena* arena) throw() : arena_(arena) {}
    explicit ArenaAllocator(const ArenaAllocator& other) throw()
        : ArenaAllocator<T>(other.arena_) {}
    template <typename U>
    ArenaAllocator(const ArenaAllocator<U>& other) throw()
        : ArenaAllocator<T>(other.arena()) {}
    
    template <typename U>
    friend class ZoneAllocator;
    
    T* address(T& x) const { return &x; }
    const T* address(const T& x) const { return &x; }
    
    inline T* allocate(size_t n, const void* hint = 0);
    
    void deallocate(T* p, size_t) { /* noop for Zones */
    }
    
    size_t max_size() const throw() {
        return std::numeric_limits<int>::max() / sizeof(T);
    }
    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        void* v_p = const_cast<void*>(static_cast<const void*>(p));
        new (v_p) U(std::forward<Args>(args)...);
    }
    template <typename U>
    void destroy(U* p) {
        p->~U();
    }
    
    bool operator == (ArenaAllocator const& other) const {
        return arena_ == other.arena_;
    }
    bool operator != (ArenaAllocator const& other) const {
        return arena_ != other.arena_;
    }
    
    Arena* arena() const { return arena_; }
    
private:
    Arena* arena_;
}; // template <class T> class ArenaAllocator
    
class Arena {
public:
    static constexpr size_t kBlockSize = base::kMB;
    
    Arena();
    ~Arena();
    
    void Purge();
    
    template<class T, class... Args>
    T *New(Args&&... args) { return new (Allocate(sizeof(T))) T(std::forward<Args>(args)...); }
    
    template<class T> T *NewArray(size_t n) {
        return static_cast<T *>(Allocate(sizeof(T) * n));
    }
    
    void *Allocate(size_t n);
    
    int CountBlocks() const {
        int n = 0;
        for (auto i = block_; i != nullptr; i = i->next) {
            n++;
        }
        return n;
    }
    
    int CountLargeBlocks() const {
        int n = 0;
        for (auto i = large_blocks_; i != nullptr; i = i->next) {
            n++;
        }
        return n;
    }
    
    template <class T>
    inline T *Lazy() {
        if (auto iter = lazy_objects_.find(T::Uniquely()); iter != lazy_objects_.end()) {
            return static_cast<T *>(iter->second);
        }
        if (auto inst = New<T>(this)) {
            inst->Init();
            lazy_objects_[T::Uniquely()] = inst;
            return inst;
        }
        return nullptr;
    }
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Arena);
private:
    template<class T> struct Less : public std::binary_function<T, T, bool> {
        bool operator ()(const T &lhs, const T &rhs) const { return lhs < rhs; }
    }; // struct ArenaLess
    
    class LazyObjectMap : public std::map<uintptr_t,
                                          void *,
                                          Less<uintptr_t>,
                                          ArenaAllocator<std::pair<const uintptr_t, void *>>> {
    public:
        // Constructs an empty map.
        explicit LazyObjectMap(Arena* zone)
            : std::map<uintptr_t,
                       void *,
                       Less<uintptr_t>,
                       ArenaAllocator<std::pair<const uintptr_t, void *>>>(
              Less<uintptr_t>(), ArenaAllocator<std::pair<const uintptr_t, void *>>(zone)) {}
    }; // class LazyObjectMap
    
    struct BlockHeader {
        BlockHeader *next;
        int32_t      size;
        
        size_t free_size() const { return (kBlockSize - sizeof(BlockHeader)) - size; }

        Address free_address() { return reinterpret_cast<Address>(this + 1) + size; }
    };
    
    static constexpr size_t kUsefulSize = kBlockSize - sizeof(BlockHeader);
    
    bool ShouldUseLargeBlock(size_t n) { return n >= kUsefulSize / 2; }
    
    BlockHeader *NewLargeBlock(size_t n) {
        size_t block_size = n + sizeof(BlockHeader);
        void *memory = ::malloc(block_size);
        DbgInitZag(memory, block_size);
        BlockHeader *block = static_cast<BlockHeader *>(memory);
        block->next = large_blocks_;
        block->size = static_cast<uint32_t>(n);
        large_blocks_ = block;
        return block;
    }
    
    BlockHeader *NewBlock() {
        BlockHeader *block = static_cast<BlockHeader *>(::malloc(kBlockSize));
        block->next = block_;
        block->size = 0;
        block_ = block;
        return block;
    }
    
    BlockHeader *block_ = nullptr;
    BlockHeader *large_blocks_ = nullptr;
    LazyObjectMap lazy_objects_;
}; // class Arena

template<class T>
inline T* ArenaAllocator<T>::allocate(size_t n, const void* hint) {
    return arena_->NewArray<T>(n);
}

class ArenaObject {
public:
    void *operator new (size_t n, Arena *arena) { return arena->Allocate(n); }
}; // class ArenaObject
    
} // namespace base
    
} // namespace yalx

#endif // YALX_BASE_ARENA_H_
