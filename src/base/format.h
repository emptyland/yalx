#ifndef YALX_BASE_FORMAT_H_
#define YALX_BASE_FORMAT_H_

#include "runtime/macros.h"
#include <stdarg.h>
#include <string>
#include <string.h>

namespace yalx::base {

#if defined(YALX_USE_CLANG)
__attribute__ (( __format__ (__printf__, 1, 2)))
#endif
std::string Sprintf(const char *fmt, ...);

std::string Vsprintf(const char *fmt, va_list ap);

// return:
//  0 = parse ok
// > 0 = overflow
// < 0 = bad char
int ParseI64(const char *s, size_t n, int64_t *val);

inline int ParseI64(const char *s, int64_t *val) {
    return ParseI64(s, !s ? 0 : strlen(s), val);
}



// return:
//  0 = parse ok
// > 0 = overflow
// < 0 = bad char
int ParseU64(const char *s, size_t n, uint64_t *val);

inline int ParseU64(const char *s, uint64_t *val) {
    return ParseU64(s, !s ? 0 : strlen(s), val);
}



// return:
//  0 = parse ok
// > 0 = overflow
// < 0 = bad char
int ParseH64(const char *s, size_t n, uint64_t *val);

inline int ParseH64(const char *s, uint64_t *val) {
    return ParseH64(s, !s ? 0 : strlen(s), val);
}

// return:
//  0 = parse ok
// > 0 = overflow
// < 0 = bad char
int ParseO64(const char *s, size_t n, uint64_t *val);

inline int ParseO64(const char *s, uint64_t *val) {
    return ParseO64(s, !s ? 0 : strlen(s), val);
}

// return:
//  0 = parse ok
// > 0 = overflow
// < 0 = bad char
int ParseI32(const char *s, size_t n, int32_t *val);

inline int ParseI32(const char *s, int32_t *val) {
    return ParseI32(s, !s ? 0 : strlen(s), val);
}


} // namespace yalx


#endif // YALX_BASE_FORMAT_H_
