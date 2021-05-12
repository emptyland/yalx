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


class WritableFile {
public:
    WritableFile() {}
    virtual ~WritableFile();
    
    virtual Status Append(std::string_view data) = 0;
    virtual Status PositionedAppend(std::string_view data, uint64_t offset) = 0;
    virtual Status Truncate(uint64_t size) = 0;
    
}; // class WritableFile

class RandomAccessFile {
public:
    RandomAccessFile() {}
    virtual ~RandomAccessFile();
    
    virtual Status Read(uint64_t offset, size_t n, std::string_view *result, std::string *scratch) = 0;
    virtual Status GetFileSize(uint64_t *size) = 0;
}; // class RandomAccessFile

} // namespace base

    
} // namespace yalx


#endif // YALX_BASE_IO_H_
