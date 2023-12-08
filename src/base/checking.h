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

#define UNREACHABLE() ::yalx::base::Fatal(OS_LITERAL("Unreachable"), OS_LITERAL(__FILE__), __LINE__)

#if defined(NDEBUG)

#define DCHECK_NOTNULL(p) (p)
#define printd(...)

#ifndef DCHECK
#define DCHECK(x) ::yalx::base::CheckOutput()
#endif

class CheckOutput {
public:
    inline CheckOutput() = default;
    inline CheckOutput &Hint(const char */*fmt*/, ...) { return *this; }
}; // class CheckOutput

#else

#define DCHECK_NOTNULL(p) (::yalx::base::CheckNotNull(p, OS_LITERAL(__FILE__), __LINE__))

#ifndef DCHECK
#define DCHECK(x) ::yalx::base::CheckOutput(__FILE__, __LINE__, __func__).Assert((x), #x)
#endif

#define printd(...) ::yalx::base::DebugOutput(__FILE__, __LINE__, __func__).Print(__VA_ARGS__)

template<class T>
inline T *CheckNotNull(T *p, const OSChar *file, int line) {
    if (!p) {
        Fatal(OS_LITERAL("Pointer IS NULL"), file, line);
    }
    return p;
}

class CheckOutput {
public:
    CheckOutput(const char *file, int line, const char *func);
    ~CheckOutput();

    CheckOutput &Assert(bool ok, const char *literal);

    CheckOutput &Hint(const char *fmt, ...);
private:
    const char *const file_;
    const char *const func_;
    char *message_ = nullptr;
    char *hint_ = nullptr;
    const int line_;
    bool ok_ = true;
}; // class CheckOutput

class DebugOutput {
public:
    DebugOutput(const char *file, int line, const char *func);
    
    static void Print(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        putc('\n', stdout);
        fflush(stdout);
    }
    
private:
}; // class DebugOutput

#endif

} // namespace yalx


#endif // YALX_BASE_CHECKING_H_
