#include "base/checking.h"
#include <stdlib.h>

namespace yalx::base {

#ifndef NDEBUG

const char *GetShortFilePath(const char *file) {
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
: file_(GetShortFilePath(file))
, line_(line)
, func_(func) {

}

CheckOutput::~CheckOutput() {
    if (!ok_) {
        fprintf(stderr, "[%s:%d (%s)] Assertion `%s` fail. %s\n",
                file_, line_, func_, literal_, !hint_ ? "" : hint_);
    }
    ::free(const_cast<char *>(hint_));
    ::free(const_cast<char *>(literal_));
}

CheckOutput &CheckOutput::Assert(bool ok, const char *literal) {
    ok_ = ok;
    if (!ok_) {
        literal_ = ::strdup(literal);
    }
    return *this;
}

CheckOutput &CheckOutput::Hint(const char *fmt, ...) {
    // TODO:
    return *this;
}

#endif // NDEBUG

} // namespace yalx
