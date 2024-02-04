#include "base/status.h"
#include "base/format.h"
#include <string.h>

namespace yalx::base {

std::string Status::ToString() const {
    std::string s;
    if (file_name_) {
        int n = 0;
        const char *x = nullptr;
        size_t len = ::strlen(file_name_);
        for (x = file_name_ + len; x > file_name_; --x) {
            if (*x == '/' || *x == '\\') {
                ++n;
            }
            if (n == 2) {
                x++;
                break;
            }
        }
        s = base::Sprintf("[%s:%d] ", x, line_);
    }
    
    switch (code()) {
        case kOk:
            return "Ok";
        case kNotFound:
            s.append("Not Found: ");
            break;
        case kCorruption:
            s.append("Corruption: ");
            break;
        case kNotSupported:
            s.append("Not Supported: ");
            break;
        case kInvalidArgument:
            s.append("Invalid Argument: ");
            break;
        default:
            break;
    }
    s.append(message());
    return s;
}

Status &Status::operator = (const Status &other) {
    if (&other == this) {
        return *this;
    }
    file_name_ = other.file_name_;
    line_ = other.line_;
    delete [] state_;
    if (state_) {
        state_ = MakeState(code(), std::string_view(state_ + 8, *reinterpret_cast<const int *>(state_)));
    } else {
        state_ = nullptr;
    }
    return *this;
}

Status &Status::operator = (Status &&other) noexcept {
    file_name_ = other.file_name_;
    line_ = other.line_;
    state_ = other.state_;

    other.file_name_ = nullptr;
    other.line_ = 0;
    other.state_ = nullptr;
}

#if defined(YALX_OS_POSIX)

Status Status::PError(const char *file_name, int line, std::string_view message) {
    char buf[256];
    ::strerror_r(errno, buf, arraysize(buf));
    return Corruption(file_name, line, Sprintf("%s, \"%s\".", message.data(), buf));
}

#endif // defined(YALX_OS_POSIX)

} // namespace yalx
