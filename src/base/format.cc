#include "base/format.h"


namespace yalx {

namespace base {

/*static*/ std::string Sprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    std::string str(Vsprintf(fmt, ap));
    va_end(ap);
    return str;
}

/*static*/ std::string Vsprintf(const char *fmt, va_list ap) {
    va_list copied;
    int len = 128, rv = len;
    std::unique_ptr<char[]> buf;
    do {
        len = rv + 128;
        buf.reset(new char[len]);
        va_copy(copied, ap);
        rv = ::vsnprintf(buf.get(), len, fmt, ap);
        va_copy(ap, copied);
    } while (rv > len);
    //buf[rv] = 0;
    return std::string(buf.get());
}

} // namespace base

} // namespace yalx
