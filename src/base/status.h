#pragma once
#ifndef YALX_BASE_STATUS_H_
#define YALX_BASE_STATUS_H_

#include "base/base.h"
#include <string>
#include <string_view>

namespace yalx {

namespace base {

class Status final {
public:
    enum Code : int {
        kOk,
        kNotFound,
        kCorruption,
        kNotSupported,
        kInvalidArgument,
        kEOF
    };

    Status() : Status(nullptr, 0, kOk, "") {}

    Status(const Status &other): Status(other.file_name_, other.line_, other.code(), other.message()) {}

    Status(Status &&other)
        : file_name_(other.file_name_)
        , line_(other.line_)
        , state_(other.state_) {
        other.file_name_ = nullptr;
        other.line_ = 0;
        other.state_ = nullptr;
    }

    ~Status() {
        delete [] state_;
        state_ = nullptr;
    }
    
    static Status OK() { return Status(); }
    
    static Status Eof() { return Status(nullptr, 0, kEOF, ""); }
    
    static Status NotFound(const char *file_name, int line, std::string_view message = "") {
        return Status(file_name, line, kNotFound, message);
    }
    
    static Status Corruption(const char *file_name, int line, std::string_view message = "") {
        return Status(file_name, line, kCorruption, message);
    }

    static Status PError(const char *file_name, int line, std::string_view message = "");

    static Status NotSupported(const char *file_name, int line, std::string_view message = "") {
        return Status(file_name, line, kNotSupported, message);
    }

    static Status InvalidArgument(const char *file_name, int line, std::string_view message = "") {
        return Status(file_name, line, kInvalidArgument, message);
    }

    std::string_view message() const {
        return ok() ? "" : std::string_view(state_ + 8, *reinterpret_cast<const int *>(state_));
    }

    bool operator !() const { return fail(); }
    
    bool ok() const { return code() == kOk; }
    
    bool fail() const { return !ok(); }
    
    bool IsNotFound() const { return code() == kNotFound; }
    
    bool IsCorruption() const { return code() == kCorruption; }
    
    bool IsNotSupported() const { return code() == kNotSupported; }
    
    bool IsInvalidArgument() const { return code() == kInvalidArgument; }
    
    bool IsEof() const { return code() == kEOF; }
    
    std::string ToString() const;
    
    void operator = (const Status &other) {
        file_name_ = other.file_name_;
        line_ = other.line_;
        delete [] state_;
        if (state_) {
            state_ = MakeState(code(), std::string_view(state_ + 8, *reinterpret_cast<const int *>(state_)));
        } else {
            state_ = nullptr;
        }
    }
    
    void operator = (Status &&other) {
        file_name_ = other.file_name_;
        line_ = other.line_;
        state_ = other.state_;
        
        other.file_name_ = nullptr;
        other.line_ = 0;
        other.state_ = nullptr;
    }
private:
    Status(const char *file_name, int line, Code code, std::string_view message)
        : file_name_(file_name)
        , line_(line)
        , state_(MakeState(code, message)) {}
    
    Code code() const {
        return state_ == nullptr ? kOk : *reinterpret_cast<const Code *>(state_ + 4);
    }
    
    static const char *MakeState(Code code, std::string_view message) {
        if (code == kOk) {
            return nullptr;
        }
        size_t len = message.length() + sizeof(code) + sizeof(int);
        char *state = new char[len];
        *reinterpret_cast<int *>(state) = static_cast<int>(message.length());
        *reinterpret_cast<Code *>(state + 4) = code;
        memcpy(state + 8, message.data(), message.length());
        return state;
    }

    const char *file_name_;
    int line_;
    const char *state_;
}; // class Status

#define ERR_NOT_FOUND()           ::yalx::base::Status::NotFound(__FILE__, __LINE__)
#define ERR_CORRUPTION(msg)       ::yalx::base::Status::Corruption(__FILE__, __LINE__, msg)
#define ERR_PERROR(msg)           ::yalx::base::Status::PError(__FILE__, __LINE__, msg)
#define ERR_NOT_SUPPORTED()       ::yalx::base::Status::NotSupported(__FILE__, __LINE__)
#define ERR_INVALID_ARGUMENT(msg) ::yalx::base::Status::InvalidArgument(__FILE__, __LINE__, msg)

} // namespace base

} // namespace yalx

#endif // YALX_BASE_STATUS_H_
