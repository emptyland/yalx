#include "base/checking.h"
#include "runtime/macros.h"
#include <stdlib.h>
#if defined(YALX_OS_POSIX)
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <cxxabi.h>
#endif // defined(YALX_OS_POSIX)

namespace yalx::base {

#ifndef NDEBUG

#if defined(YALX_OS_POSIX)

static void PrintBacktracePosix(FILE *fp) {
    //struct back heap_alloc(backtrace_frame_class);
    unw_cursor_t cursor;
    unw_context_t context;

    // Initialize cursor to current frame for local unwinding.
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    while (unw_step(&cursor) > 0) {
        unw_word_t offset = 0, pc = 0;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);


        char demangle[256];
        if (unw_get_proc_name(&cursor, demangle, 256, &offset) == 0) {
            char name[512];
            size_t len = sizeof(name);
            int status;
            abi::__cxa_demangle(demangle, name, &len, &status);
            ::fprintf(fp, "    0x%lx %s+%lu\n", pc, name, offset);
        } else {
            ::fprintf(fp, "    0x%lx Unknown\n", pc);
        }
    }
}


#endif // defined(YALX_OS_POSIX)

static const char *GetShortFilePath(const char *file) {
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
    return file;
}

DebugOutput::DebugOutput(const char *file, int line, const char *func) {
    file = GetShortFilePath(file);
    printf("🧽[%s:%d (%s)] ", file, line, func);
}

CheckOutput::CheckOutput(const char *file, int line, const char *func)
: file_(file)
, line_(line)
, func_(func) {

}

CheckOutput::~CheckOutput() {
    if (!ok_) {
        fprintf(stderr, "[%s:%d (%s)] Assertion `%s` fail. %s\n",
                file_, line_, func_, literal_, !hint_ ? "" : hint_);
    #if defined(YALX_OS_POSIX)
        PrintBacktracePosix(stderr);
    #endif // defined(YALX_OS_POSIX)
    }
    ::free(const_cast<char *>(hint_));
    ::free(const_cast<char *>(literal_));
    if (!ok_) {
        abort();
    }
}

CheckOutput &CheckOutput::Assert(bool ok, const char *literal) {
    ok_ = ok;
    if (!ok_) {
        literal_ = ::strdup(literal);
    }
    return *this;
}

char *VsprintfDup(const char *fmt, va_list ap, char *buf) {
    va_list copied;
    int len = 128, rv = len;
    do {
        len = rv + 128;
        buf = static_cast<char *>(::realloc(buf, len));
        va_copy(copied, ap);
        rv = ::vsnprintf(buf, len, fmt, ap);
        va_copy(ap, copied);
    } while (rv > len);

    return buf;
}

CheckOutput &CheckOutput::Hint(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    hint_ = VsprintfDup(fmt, ap, hint_);
    va_end(ap);
    return *this;
}

#endif // NDEBUG

} // namespace yalx::base
