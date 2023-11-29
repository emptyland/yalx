#include "backend/linkage-symbols.h"
#include "runtime/macros.h"

namespace yalx::backend {

#ifdef YALX_OS_DARWIN
DECLARE_STATIC_STRING(kLibc_memcpy, "_memcpy");
DECLARE_STATIC_STRING(kLibc_memset, "_memset");
DECLARE_STATIC_STRING(kRt_YGC_ADDRESS_BAD_MASK, "_YGC_ADDRESS_BAD_MASK");

#define DEFINE_RUNTIME_FUN_SYMBOL(name) DECLARE_STATIC_STRING(kRt_##name, "_" #name);
    DECLARE_RUNTIME_FUNS(DEFINE_RUNTIME_FUN_SYMBOL)
#undef DEFINE_RUNTIME_FUN_SYMBOL
#endif // #ifdef YALX_OS_DARWIN

#ifdef YALX_OS_LINUX
DECLARE_STATIC_STRING(kLibc_memcpy, "memcpy");
DECLARE_STATIC_STRING(kLibc_memset, "memset");
DECLARE_STATIC_STRING(kRt_YGC_ADDRESS_BAD_MASK, "YGC_ADDRESS_BAD_MASK");

#define DEFINE_RUNTIME_FUN_SYMBOL(name) DECLARE_STATIC_STRING(kRt_##name, #name);
    DECLARE_RUNTIME_FUNS(DEFINE_RUNTIME_FUN_SYMBOL)
#undef DEFINE_RUNTIME_FUN_SYMBOL
#endif // #ifdef YALX_OS_LINUX


Linkage::Linkage(base::Arena *arena)
: arena_(arena)
, symbols_(arena) {
}

const String *Linkage::Mangle(std::string_view name) {
    if (auto iter = symbols_.find(name); iter != symbols_.end()) {
        return iter->second;
    }
    std::string buf;
    Build(&buf, name);
    auto symbol = String::New(arena_, buf.data(), buf.size());
    symbols_[name] = symbol;
    return symbol;
}

void Linkage::Build(std::string *buf, std::string_view name) {
#if defined(YALX_OS_DARWIN)
    buf->append("_");
#endif
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

} // namespace yalx
