#pragma once
#ifndef YALX_BASE_CHECKING_H_
#define YALX_BASE_CHECKING_H_

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

namespace yalx {

namespace base {

#if defined(NDEBUG)

#define UNREACHABLE()
#define DCHECK_NOTNULL(p) (p)
#define printd(...)

#ifndef DCHECK
#define DCHECK(x)
#endif

#else

#define UNREACHABLE() assert(0 && "unreached")
#define DCHECK_NOTNULL(p) (::yalx::base::CheckNotNull(p, __FILE__, __LINE__))

#ifndef DCHECK
#define DCHECK(x) assert(x)
#endif

#define printd(...) ::yalx::base::DebugOutput(__FILE__, __LINE__, __func__).Print(__VA_ARGS__)

template<class T>
inline T *CheckNotNull(T *p, const char *file, int line) {
    if (!p) {
    #if __DARWIN_UNIX03
        __assert_rtn(__func__, file, line, "Pointer IS NULL");
    #else
        __assert("Pointer IS NULL", file, line);
    #endif
    }
    return p;
}

class DebugOutput {
public:
    DebugOutput(const char *file, int line, const char *func);
    
    void Print(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        putc('\n', stdout);
    }
    
private:
}; // class DebugOutput

#endif

} // namespace base

} // namespace yalx


#endif // YALX_BASE_CHECKING_H_
