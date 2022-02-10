#include "backend/linkage-symbols.h"


namespace yalx {
namespace backend {

#define DECLARE_STATIC_STRING(name, literal) \
static StaticString<sizeof(literal)> name##_stub { \
    0, \
    sizeof(literal), \
    literal, \
}; \
const String *const name = reinterpret_cast<const String *>(&name##_stub)

DECLARE_STATIC_STRING(kLibc_memcpy, "_memcpy");
DECLARE_STATIC_STRING(kLibc_memset, "_memset");

LinkageSymbols::LinkageSymbols(base::Arena *arena)
: arena_(arena)
, symbols_(arena) {
}

const String *LinkageSymbols::Symbolize(std::string_view name) {
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
