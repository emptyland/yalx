#include "base/arena.h"
#include <string.h>

namespace yalx::base {

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

char *Arena::Duplicate(const char *z, size_t n) {
    auto dest = static_cast<char *>(Allocate(n + 1));
    ::memcpy(dest, z, n);
    dest[n] = '\0';
    return dest;
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

} // namespace yalx
