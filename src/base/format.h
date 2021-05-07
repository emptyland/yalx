#ifndef YALX_BASE_FORMAT_H_
#define YALX_BASE_FORMAT_H_

#include <stdarg.h>
#include <string>

namespace yalx {

namespace base {

__attribute__ (( __format__ (__printf__, 1, 2)))
std::string Sprintf(const char *fmt, ...);

std::string Vsprintf(const char *fmt, va_list ap);

} // namespace base

} // namespace yalx


#endif // YALX_BASE_FORMAT_H_
