#pragma once
#ifndef YALX_BACKEND_LINKAGE_SYMBOLS_H_
#define YALX_BACKEND_LINKAGE_SYMBOLS_H_

#include "base/arena-utils.h"

namespace yalx {
namespace backend {

using String = base::ArenaString;

class LinkageSymbols final {
public:
    enum Kind {
        kOriginal,
        kNativeHandle,
    };

    LinkageSymbols(base::Arena *arena);
    
    const String *Mangle(const String *name)  { return Mangle(name->ToSlice()); }
    const String *Mangle(const char *z, size_t n) { return Mangle(std::string_view(z, n)); }
    const String *Mangle(std::string_view name);
    
    static void BuildNativeStub(std::string *buf, std::string_view name) {
        Build(buf, name);
        buf->append("_stub");
    }
    static void BuildNativeHandle(std::string *buf, std::string_view name) {
        Build(buf, name);
        buf->append("_had");
    }
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

extern const String *const kRt_pkg_init_once;
extern const String *const kRt_reserve_handle_returning_vals;
extern const String *const kRt_current_root;
extern const String *const kRt_yalx_exit_returning_scope;
extern const String *const kRt_associate_stub_returning_vals;

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_LINKAGE_SYMBOLS_H_
