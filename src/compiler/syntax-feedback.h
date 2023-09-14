#pragma once
#ifndef YALX_COMPILER_SYNTAX_FEEDBACK_H_
#define YALX_COMPILER_SYNTAX_FEEDBACK_H_

#include "base/format.h"
#include "base/base.h"
#include <stdarg.h>

namespace yalx {
    
namespace cpl {

class SourcePosition;

// Feed back for error or warning
class SyntaxFeedback {
public:
    SyntaxFeedback() = default;
    virtual ~SyntaxFeedback() = default;
    
    void Feedback(const SourcePosition &location, const char *message) {
        DidFeedback(location, message, ::strlen(message));
    }
#if defined(YALX_USE_CLANG)
    __attribute__ (( __format__ (__printf__, 3, 4)))
#endif
    void Printf(const SourcePosition &location, const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        std::string message(base::Vsprintf(fmt, ap));
        va_end(ap);
        DidFeedback(location, message.data(), message.size());
    }
    
    virtual void DidFeedback(const SourcePosition &location, const char *z, size_t n) = 0;
    
    virtual void DidFeedback(const char *z) = 0;

    DEF_VAL_PROP_RW(std::string, package_name);
    DEF_VAL_PROP_RW(std::string, file_name);
protected:
    std::string package_name_;
    std::string file_name_;
}; // class SyntaxFeedback


} // namespace cpl

} // namespace yalx


#endif // YALX_COMPILER_SYNTAX_FEEDBACK_H_
