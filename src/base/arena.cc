#include "base/arena.h"

namespace yalx {

namespace base {

Arena::Arena(): lazy_objects_(this) {}

Arena::~Arena() { Purge(); }

void Arena::Purge() {
    lazy_objects_.clear();
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

void *Arena::Allocate(size_t n) {
    n = RoundUp(n, 4);
    if (ShouldUseLargeBlock(n)) {
        return NewLargeBlock(n);
    }
    auto block = block_;
    if (!block_ || block_->free_size() < n) {
        block = NewBlock();
    }
    
    void *chunk = block->free_address();
    block->size += n;
    return DbgInitZag(chunk, n);
}

} // namespace base

} // namespace yalx
