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



extern const String *const kLibc_memcpy;
extern const String *const kLibc_memset;

extern const String *const kRt_builtin_classes;

extern const String *const kRt_pkg_init_once;
extern const String *const kRt_reserve_handle_returning_vals;
extern const String *const kRt_current_root;
extern const String *const kRt_yalx_exit_returning_scope;
extern const String *const kRt_associate_stub_returning_vals;
extern const String *const kRt_heap_alloc;
extern const String *const kRt_raise;
extern const String *const kRt_is_instance_of;
extern const String *const kRt_ref_asserted_to;
extern const String *const kRt_put_field;
extern const String *const kRt_put_field_chunk;
extern const String *const kRt_lazy_load_object;
extern const String *const kRt_array_alloc;
extern const String *const kRt_array_fill;
extern const String *const kRt_array_location_at;
extern const String *const kRt_array_location_at1;
extern const String *const kRt_array_location_at2;
extern const String *const kRt_array_location_at3;
extern const String *const kRt_array_set_ref;
extern const String *const kRt_array_set_ref1;
extern const String *const kRt_array_set_ref2;
extern const String *const kRt_array_set_ref3;
extern const String *const kRt_array_set_chunk;
extern const String *const kRt_array_set_chunk1;
extern const String *const kRt_array_set_chunk2;
extern const String *const kRt_array_set_chunk3;

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_LINKAGE_SYMBOLS_H_
