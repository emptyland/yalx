#include "backend/linkage-symbols.h"


namespace yalx {
namespace backend {


DECLARE_STATIC_STRING(kLibc_memcpy, "_memcpy");
DECLARE_STATIC_STRING(kLibc_memset, "_memset");

DECLARE_STATIC_STRING(kRt_pkg_init_once, "_pkg_init_once");
DECLARE_STATIC_STRING(kRt_reserve_handle_returning_vals, "_reserve_handle_returning_vals");
DECLARE_STATIC_STRING(kRt_current_root, "_current_root");
DECLARE_STATIC_STRING(kRt_yalx_exit_returning_scope, "_yalx_exit_returning_scope");
DECLARE_STATIC_STRING(kRt_associate_stub_returning_vals, "_associate_stub_returning_vals");
DECLARE_STATIC_STRING(kRt_heap_alloc, "_heap_alloc");
DECLARE_STATIC_STRING(kRt_raise, "_throw_it");
DECLARE_STATIC_STRING(kRt_is_instance_of, "_is_instance_of");
DECLARE_STATIC_STRING(kRt_ref_asserted_to, "_ref_asserted_to");
DECLARE_STATIC_STRING(kRt_put_field, "_put_field");
DECLARE_STATIC_STRING(kRt_lazy_load_object, "_lazy_load_object");

LinkageSymbols::LinkageSymbols(base::Arena *arena)
: arena_(arena)
, symbols_(arena) {
}

const String *LinkageSymbols::Mangle(std::string_view name) {
    if (auto iter = symbols_.find(name); iter != symbols_.end()) {
        return iter->second;
    }
    std::string buf;
    Build(&buf, name);
    auto symbol = String::New(arena_, buf.data(), buf.size());
    symbols_[name] = symbol;
    return symbol;
}

void LinkageSymbols::Build(std::string *buf, std::string_view name) {
    buf->append("_");
    for (size_t i = 0; i < name.size(); i++) {
        auto ch = name[i];
        switch (ch) {
            case '.':
                buf->append("_Zd");
                break;
            case '$':
                buf->append("_Z4");
                break;
            case ':':
                buf->append("_Zo");
                break;
            case '<':
                buf->append("_Dk");
                break;
            case '>':
                buf->append("_Dl");
                break;
            case '/':
            case '\\':
                buf->append("_Zp");
                break;
            default:
                buf->append(1, ch);
                break;
        }
    }
}

} // namespace backend
} // namespace yalx
