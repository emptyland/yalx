#pragma once
#ifndef YALX_BACKEND_LINKAGE_SYMBOLS_H_
#define YALX_BACKEND_LINKAGE_SYMBOLS_H_

#include "base/arena-utils.h"

namespace yalx::backend {

using String = base::ArenaString;

class Linkage final {
public:
    enum Kind {
        kOriginal,
        kNativeHandle,
    };

    explicit Linkage(base::Arena *arena);
    
    const String *Mangle(const String *name)  { return Mangle(name->ToSlice()); }
    const String *Mangle(const char *z, size_t n) { return Mangle(std::string_view(z, n)); }
    const String *Mangle(std::string_view name);
    
    const String *MangleClassName(const String *name) {
        std::string buf;
        Build(&buf, name->ToString());
        buf.append("$class");
        return String::New(arena_, buf);
    }
    
    static void BuildNativeStub(std::string *buf, std::string_view name) {
        Build(buf, name);
        buf->append("_stub");
    }
    static void BuildNativeHandle(std::string *buf, std::string_view name) {
        Build(buf, name);
        buf->append("_had");
    }
    static void Build(std::string *buf, std::string_view name);
    
    int NextBlockLabel() { return next_block_label_++; }
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Linkage);
private:
    base::Arena *const arena_;
    base::ArenaUnorderedMap<std::string_view, const String *> symbols_;
    int next_block_label_ = 0;
}; // class LinkageSymbols



extern const String *const kLibc_memcpy;
extern const String *const kLibc_memset;

#define DECLARE_RUNTIME_FUNS(V) \
    V(builtin_classes) \
    V(pkg_init_once) \
    V(reserve_handle_returning_vals) \
    V(current_root) \
    V(yalx_exit_returning_scope) \
    V(associate_stub_returning_vals) \
    V(heap_alloc) \
    V(raise) \
    V(is_instance_of) \
    V(ref_asserted_to) \
    V(put_field) \
    V(put_field_chunk) \
    V(lazy_load_object) \
    V(array_alloc) \
    V(array_fill) \
    V(array_location_at) \
    V(array_location_at1) \
    V(array_location_at2) \
    V(array_location_at3) \
    V(array_set_ref) \
    V(array_set_ref1) \
    V(array_set_ref2) \
    V(array_set_ref3) \
    V(array_set_chunk) \
    V(array_set_chunk1) \
    V(array_set_chunk2) \
    V(array_set_chunk3) \
    V(closure) \
    V(concat)

#define DECL_RUNTIME_FUN_SYMBOL(name) extern const String *const kRt_##name;
    DECLARE_RUNTIME_FUNS(DECL_RUNTIME_FUN_SYMBOL)
#undef DECL_RUNTIME_FUN_SYMBOL

} // namespace yalx

#endif // YALX_BACKEND_LINKAGE_SYMBOLS_H_
