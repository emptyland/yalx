#pragma once
#ifndef YALX_BASE_PRINTD_H_
#define YALX_BASE_PRINTD_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

namespace yalx {

#ifdef NDEBUG
#define printd(...)
#else
#define printd(...) ::yalx::DebugOutput(__FILE__, __LINE__, __func__).Print(__VA_ARGS__)
#endif

class DebugOutput {
public:
    DebugOutput(const char *file, int line, const char *func) {
        auto len = ::strlen(file);
        if (len > 0) {
            auto s = file + len;
            for (auto i = 0; i < 2; i++) {
                while (*s != '/' && *s != '\\' && s > file) {
                    s--;
                }
                if (s == file) {
                    break;
                } else {
                    s++;
                }
            }
            file = s;
        }
        
        printf("[%s:%d (%s)] ", file, line, func);
    }
    
    void Print(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        putc('\n', stdout);
    }
    
private:
}; // class DebugOutput

} // namespace yalx


#endif // YALX_BASE_PRINTD_H_
