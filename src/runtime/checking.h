#pragma once
#ifndef YALX_RUNTIME_CHECKING_H_
#define YALX_RUNTIME_CHECKING_H_

#include <assert.h>

enum logging_level {
    LOGGING_INFO,
    LOGGING_WARN,
    LOGGING_ERROR,
    LOGGING_FATAL,
};

#if defined(NDEBUG)

#define DCHECK(e)
#define DLOG(level, ...)

#else // defined(NDEBUG)

#define DCHECK(e) assert(e)
#define DLOG(level, ...) yalx_logging_printf((LOGGING_##level), __FILE__, __LINE__, __VA_ARGS__)

#endif // defined(NDEBUG)

#ifndef LOG
#define LOG(level, ...) yalx_logging_printf((LOGGING_##level), __FILE__, __LINE__, __VA_ARGS__)
#endif

#ifndef PLOG
#define PLOG(...) yalx_logging_sys_error(__FILE__, __LINE__, __VA_ARGS__)
#endif

#ifndef UNREACHABLE
#define UNREACHABLE() LOG(FATAL, "No Reachable")
#endif


void yalx_logging_printf(enum logging_level level, const char *file, int line, const char *fmt, ...);
void yalx_logging_sys_error(const char *file, int line, const char *fmt, ...);

#endif // YALX_RUNTIME_CHECKING_H_


