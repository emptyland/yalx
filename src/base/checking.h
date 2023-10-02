#pragma once
#ifndef YALX_BASE_CHECKING_H_
#define YALX_BASE_CHECKING_H_

#include "runtime/macros.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

namespace yalx::base {

#if defined(YALX_OS_WINDOWS)

using OSChar = wchar_t;
#define OS_LITERAL(s) _CRT_WIDE(s)

#else

using OSChar = char;
#define OS_LITERAL(s) s

#endif

[[noreturn]] inline void Fatal(
        const OSChar *msg,
        const OSChar *file,
        int line) {
#if __DARWIN_UNIX03
    __assert_rtn(__func__, file, line, msg);
#elif YALX_OS_WINDOWS
    _wassert(msg, file, line);
#else
    __assert(msg, file, line);
#endif
}

#define UNREACHABLE() ::yalx::base::Fatal(OS_LITERAL("Noreachable"), OS_LITERAL(__FILE__), __LINE__)

#if defined(NDEBUG)

#define DCHECK_NOTNULL(p) (p)
#define printd(...)

#ifndef DCHECK
#define DCHECK(x)
#endif

#else

#define DCHECK_NOTNULL(p) (::yalx::base::CheckNotNull(p, OS_LITERAL(__FILE__), __LINE__))

#ifndef DCHECK
#define DCHECK(x) assert(x)
#endif

#define printd(...) ::yalx::base::DebugOutput(__FILE__, __LINE__, __func__).Print(__VA_ARGS__)

template<class T>
inline T *CheckNotNull(T *p, const OSChar *file, int line) {
    if (!p) {
        Fatal(OS_LITERAL("Pointer IS NULL"), file, line);
    }
    return p;
}

class DebugOutput {
public:
    DebugOutput(const char *file, int line, const char *func);
    
    static void Print(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        putc('\n', stdout);
    }
    
private:
}; // class DebugOutput

#endif

} // namespace yalx


#endif // YALX_BASE_CHECKING_H_
