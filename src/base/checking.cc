#include "base/checking.h"

namespace yalx {

namespace base {

#ifndef NDEBUG

DebugOutput::DebugOutput(const char *file, int line, const char *func) {
    auto len = ::strlen(file);
    if (len > 0) {
        auto s = file + len;
        for (auto i = 0; i < 2; i++) {
            do {
                s--;
            } while (*s != '/' && *s != '\\' && s > file);
            if (s == file) {
                break;
            }
        }
        if (s != file) {
            s++;
        }
        file = s;
    }
    
    printf("🧽[%s:%d (%s)] ", file, line, func);
}

#endif // NDEBUG

} // namespace base

} // namespace yalx
