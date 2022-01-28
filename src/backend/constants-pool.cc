#include "backend/constants-pool.h"

namespace yalx {

namespace backend {

ConstantsPool::ConstantsPool(base::Arena *arena)
: arena_(arena)
, numbers_(arena)
, strings_(arena)
, string_pool_(arena) {
}

int ConstantsPool::FindOrInsertSlot(Kind kind, const void *data, size_t size) {
    assert(size <= 8);
    Slot key;
    key.kind = kind;
    ::memcpy(key.data, data, size);
    ::memset(&key.data[size], 0, 8 - size);
    if (auto iter = numbers_.find(key); iter != numbers_.end()) {
        return iter->second;
    }
    auto id = next_unique_id_++;
    numbers_[key] = id;
    return id;
}

int ConstantsPool::FindOrInsertString(const String *value) {
    if (auto iter = strings_.find(value->ToSlice()); iter != strings_.end()) {
        return iter->second;
    }
    auto id = static_cast<int>(string_pool_.size());
    strings_[value->ToSlice()] = id;
    string_pool_.push_back(value);
    return id;
}

} // namespace backend

} // namespace yalx
