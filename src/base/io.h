#pragma once
#ifndef YALX_BASE_IO_H_
#define YALX_BASE_IO_H_

#include "base/status.h"
#include "base/base.h"
#include <string_view>
#include <string>

namespace yalx {

namespace base {

class SequentialFile {
public:
    SequentialFile() = default;
    virtual ~SequentialFile() {}
    virtual Status Read(size_t n, std::string_view *result, std::string *scratch) = 0;
    virtual Status Skip(size_t n) = 0;
    virtual Status GetFileSize(size_t *size) = 0;
}; // class SequentialFile


} // namespace base

    
} // namespace yalx


#endif // YALX_BASE_IO_H_