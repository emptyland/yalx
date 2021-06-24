#include "base/format.h"
#include "base/base.h"

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

/*static*/ int ParseI64(const char *s, size_t n, int64_t *val) {
    int sign = s[0] == '-' ? -1 : 1;
    if (s[0] == '-' || s[0] == '+') {
        s++;
        n--;
    }
    if (n == 0) {
        return -1;
    }
    if (n > 19) {
        return 1;
    }
    
    uint64_t l = 0;
    for (size_t i = 0; i < n; ++i) {
        if (*s < '0' || *s > '9') {
            return -1;
        }
        uint64_t d = 0x7fffffffffffffffULL - l * 10;
        uint64_t e = (*s++ - '0');
        d += (sign < 0) ? 1 : 0;
        if (e > d) {
            return 1;
        }
        l = l * 10 + e;
    }

    *val = l * sign;
    return 0;
}
    
/*static*/ int ParseU64(const char *s, size_t n, uint64_t *val) {
    if (n == 0) {
        return -1;
    }
    if (n > 20) { /*MAX: 18446744073709551615*/
        return 1;
    }
    uint64_t l = 0;
    for (size_t i = 0; i < n; ++i) {
        if (*s < '0' || *s > '9') {
            return -1;
        }
        uint64_t d = 0xffffffffffffffffULL - l * 10;
        uint64_t e = (*s++ - '0');
        if (e > d) {
            return 1;
        }
        l = l * 10 + e;
    }
    *val = l;
    return 0;
}
    
/*static*/ int ParseH64(const char *s, size_t n, uint64_t *val) {
    if (n == 0) {
        return -1;
    }
    if (n > 16) { /*MAX: ffffffffffffffff*/
        return 1;
    }
    uint64_t l = 0;
    for (size_t i = 0; i < n; ++i) {
        uint64_t e = 0;
        if (*s >= '0' && *s <= '9') {
            e = *s - '0';
        } else if (*s >= 'a' && *s <= 'f') {
            e = *s - 'a' + 10;
        } else if (*s >= 'A' && *s <= 'F') {
            e = *s - 'A' + 10;
        } else {
            return -1;
        }
        l = (l << 4) | e;
        ++s;
    }
    *val = l;
    return 0;
}

/*static*/ int ParseO64(const char *s, size_t n, uint64_t *val) {
    if (n == 0) {
        return -1;
    }
    if (n > 22) { /*MAX: 1777777777777777777777*/
        return 1;
    }
    
    uint64_t l = 0;
    for (size_t i = 0; i < n; ++i) {
        if (*s < '0' || *s > '7') {
            return -1;
        }
        uint64_t d = 0xffffffffffffffffULL - l * 8;
        uint64_t e = (*s++ - '0');
        if (e > d) {
            return 1;
        }
        l = l * 8 + e;
    }
    *val = l;
    return 0;
}
    
/*static*/ int ParseI32(const char *s, size_t n, int32_t *val) {
    int sign = s[0] == '-' ? -1 : 1;
    if (s[0] == '-' || s[0] == '+') {
        s++;
        n--;
    }
    if (n == 0) {
        return -1;
    }
    if (n > 10) {
        return 1;
    }
    
    uint32_t l = 0;
    for (size_t i = 0; i < n; ++i) {
        if (*s < '0' || *s > '9') {
            return -1;
        }
        uint32_t d = 0x7fffffffU - l * 10;
        uint32_t e = (*s++ - '0');
        d += (sign < 0) ? 1 : 0;
        if (e > d) {
            return 1;
        }
        l = l * 10 + e;
    }
    
    *val = l * sign;
    return 0;
}

} // namespace base

} // namespace yalx
