#pragma once
#ifndef YALX_BASE_CHECKING_H_
#define YALX_BASE_CHECKING_H_

#include <assert.h>

namespace yalx {

namespace base {

#if defined(NDEBUG)

#ifdef assert
    #undef  assert
    #define assert(x)
#endif

#define UNREACHED()
#define DCHECK_NOTNULL(p) (p)

#else

#define UNREACHABLE() assert(0 && "unreached")
#define DCHECK_NOTNULL(p) (::yalx::base::CheckNotNull(p, __FILE__, __LINE__))

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

#endif

} // namespace base

} // namespace yalx


#endif // YALX_BASE_CHECKING_H_
