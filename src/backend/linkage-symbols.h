#pragma once
#ifndef YALX_BACKEND_LINKAGE_SYMBOLS_H_
#define YALX_BACKEND_LINKAGE_SYMBOLS_H_

#include "base/arena-utils.h"

namespace yalx {
namespace backend {

using String = base::ArenaString;

class LinkageSymbols final {
public:
    LinkageSymbols(base::Arena *arena);
    
    const String *Symbolize(const String *name)  { return Symbolize(name->ToSlice()); }
    const String *Symbolize(const char *z, size_t n) { return Symbolize(std::string_view(z, n)); }
    const String *Symbolize(std::string_view name);
    
    static void Build(std::string *buf, std::string_view name);
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(LinkageSymbols);
private:
    base::Arena *const arena_;
    base::ArenaUnorderedMap<std::string_view, const String *> symbols_;
}; // class LinkageSymbols


template<int N>
struct StaticString {
    uint32_t hash_val;
    uint32_t len;
    char buf[N];
}; // struct StaticString

extern const String *const kLibc_memcpy;
extern const String *const kLibc_memset;

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_LINKAGE_SYMBOLS_H_
