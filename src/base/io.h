#pragma once
#ifndef YALX_BASE_IO_H_
#define YALX_BASE_IO_H_

#include "base/status.h"
#include "base/base.h"
#include <stdarg.h>
#include <string_view>
#include <string>

namespace yalx {

namespace base {

class SequentialFile {
public:
    SequentialFile() = default;
    virtual ~SequentialFile() = default;
    virtual Status Read(size_t n, std::string_view *result, std::string *scratch) = 0;
    virtual Status Skip(size_t n) = 0;
    virtual Status Available(size_t *size) = 0;
}; // class SequentialFile


class WritableFile {
public:
    WritableFile() = default;
    virtual ~WritableFile() = default;
    
    virtual Status Append(std::string_view data) = 0;
    virtual Status PositionedAppend(std::string_view data, uint64_t offset) = 0;
    virtual Status Truncate(uint64_t size) = 0;
    virtual std::string ToString() const = 0;
}; // class WritableFile

class RandomAccessFile {
public:
    RandomAccessFile() = default;
    virtual ~RandomAccessFile() = default;
    
    virtual Status Read(uint64_t offset, size_t n, std::string_view *result, std::string *scratch) = 0;
    virtual Status GetFileSize(uint64_t *size) = 0;
}; // class RandomAccessFile

class PrintingWriter final {
public:
    static constexpr int kMaxInlineIndents = 6;
    
    PrintingWriter(WritableFile *file, bool ownership = false)
        : file_(file)
        , ownership_(ownership) {}
    ~PrintingWriter() {
        if (ownership_) { delete file_; }
    }
    
    PrintingWriter *Print(const char *fmt, ...);

    PrintingWriter *Println(const char *fmt, ...);
    
    PrintingWriter *Indent(int indent) {
        if (indent < kMaxInlineIndents) {
            return Write(kInlineIndents[indent]);
        }
        Write(kInlineIndents[kMaxInlineIndents - 1]);
        for (int i = 0; i < indent - (kMaxInlineIndents - 1); i++) {
            Write(kInlineIndents[1]);
        }
        return this;
    }
    
    PrintingWriter *Write(std::string_view str) {
        auto rs = file_->Append(str);
        assert(rs.ok());
        return this;
    }
    
    PrintingWriter *Writeln(std::string_view str) {
        Write(str);
        Write("\n");
        return this;
    }
private:
    static const char *kInlineIndents[kMaxInlineIndents];
    
    WritableFile *file_;
    bool ownership_;
};

SequentialFile *NewMemorySequentialFile(const std::string &buf);
SequentialFile *NewMemorySequentialFile(const char *z, size_t n);

WritableFile *NewWritableFile(FILE *fp);
WritableFile *NewMemoryWritableFile(std::string *buf);

} // namespace base

} // namespace yalx


#endif // YALX_BASE_IO_H_
