#include "runtime/checking.h"
#include "runtime/macros.h"
#if defined(YALX_OS_POSIX)
#include <errno.h>
#include <string.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

//LOGGING_INFO,
//LOGGING_WARN,
//LOGGING_ERROR,
//LOGGING_FATAL,
static const char *levels_text[] = {
        "INFO",
        "WARN",
        "ERROR",
        "FATAL",
};

void yalx_logging_printf(enum logging_level level, const char *file, int line, const char *fmt, ...) {
    FILE *fp = level >= LOGGING_WARN ? stderr : stdout;
    va_list ap;
    va_start(ap, fmt);
    fprintf(fp, "[%s:%d] %s ", file, line, levels_text[level]);
    vfprintf(fp, fmt, ap);
    fputc('\n', fp);
    fflush(fp);
    va_end(ap);

    if (level == LOGGING_FATAL) {
        abort();
    }
}

#if defined(YALX_OS_POSIX)
void yalx_logging_sys_error(const char *file, int line, const char *fmt, ...) {
    char fixed_buf[512];
    char *buf = &fixed_buf[0];
    size_t len = sizeof(fixed_buf);
    const int e = errno;

    while (strerror_r(e, buf, len) < 0) {
        len <<= 1;
        if (buf == &fixed_buf[0]) {
            buf = malloc(len);
        } else {
            buf = realloc(buf, len);
        }
        DCHECK(len <= 4096);
    }

    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "[%s:%d] <OS> ", file, line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, ": %s\n", buf);
    fflush(stderr);
    va_end(ap);

    if (buf != &fixed_buf[0]) {
        free(buf);
    }
}
#endif
