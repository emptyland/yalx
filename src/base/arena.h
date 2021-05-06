#pragma once
#ifndef YALX_BASE_ARENA_H_
#define YALX_BASE_ARENA_H_

#include "base/base.h"

namespace yalx {
    
namespace base {
    
class Arena {
public:
    static constexpr size_t kBlockSize = base::kMB;
    
    Arena() = default;
    
    ~Arena() { Purge(); }
    
    void Purge() {
        while (block_) {
            auto block = block_;
            block_ = block_->next;
            ::free(block);
        }
        while (large_blocks_) {
            auto block = large_blocks_;
            large_blocks_ = large_blocks_->next;
            ::free(block);
        }
    }
    
    template<class T> T *New() { return new (Allocate(sizeof(T))) T(); }
    
    template<class T> T *NewArray(size_t n) {
        return static_cast<T *>(Allocate(sizeof(T) * n));
    }
    
    void *Allocate(size_t n) {
        n = RoundUp(n, 4);
        if (ShouldUseLargeBlock(n)) {
            return NewLargeBlock(n);
        }
        auto block = block_;
        if (!block_ || block_->free_size() < n) {
            block = NewBlock();
        }
        
        void *chunk = block->free_address();
        // TODO:
        block->size += n;
        return chunk;
    }
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Arena);
private:
    struct BlockHeader {
        BlockHeader *next;
        int32_t      size;
        
        size_t free_size() const { return (kBlockSize - sizeof(BlockHeader)) - size; }

        Address free_address() { return reinterpret_cast<Address>(this + 1 + size); }
    };
    
    static constexpr size_t kUsefulSize = kBlockSize - sizeof(BlockHeader);
    
    bool ShouldUseLargeBlock(size_t n) { return n >= kUsefulSize / 2; }
    
    BlockHeader *NewLargeBlock(size_t n) {
        size_t block_size = n + sizeof(BlockHeader);
        void *memory = ::malloc(block_size);
        // TODO:
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
}; // class Arena

class ArenaObject {
public:
    void *operator new (size_t n, Arena *arena) { return arena->Allocate(n); }
}; // class ArenaObject
    
} // namespace base
    
} // namespace yalx

#endif // YALX_BASE_ARENA_H_
