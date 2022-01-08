#include "base/io.h"
#include "base/checking.h"
#include "base/format.h"
#include <string.h>
#include <string>

namespace yalx {

namespace base {

class MemorySequentialFile : public SequentialFile {
public:
    MemorySequentialFile(const std::string buf): MemorySequentialFile(buf.data(), buf.size()) {}
    MemorySequentialFile(const char *buf, size_t n)
        : buf_(new char[n])
        , end_(buf_ + n)
        , pos_(buf_) {
        ::memcpy(buf_, buf, n);
    }
    ~MemorySequentialFile() override { delete[] buf_; };
    Status Read(size_t n, std::string_view *result, std::string */*scratch*/) override {
        if (pos_ >= end_) {
            return Status::Eof();
        }
        assert(pos_ < end_);
        auto available = static_cast<size_t>(end_ - pos_);
        n = std::min(available, n);
        *result = std::string_view(pos_, n);
        pos_ += n;
        return Status::OK();
    }
    Status Skip(size_t n) override {
        pos_ = std::min(pos_ + n, end_);
        return Status::OK();
    }
    Status Available(size_t *size) override {
        assert(pos_ <= end_);
        *size = end_ - pos_;
        return Status::OK();
    }
private:
    char *const buf_;
    char *end_;
    char *pos_;
}; // class SequentialFile

class LibCWritableFile final : public WritableFile {
public:
    LibCWritableFile(FILE *fp) : file_(fp) {}
    ~LibCWritableFile() override { if (file_) { ::fclose(file_); } }
    
    Status Append(std::string_view data) override {
        if (::fwrite(data.data(), 1, data.size(), file_) < data.size()) {
            return ERR_PERROR("Write file fail");
        }
        return Status::OK();
    }
    
    Status PositionedAppend(std::string_view data, uint64_t offset) override {
        if (::fseek(file_, offset, SEEK_SET) < 0) {
            return ERR_PERROR("Seek file position fail!");
        }
        return Append(data);
    }

    Status Truncate(uint64_t size) override {
        // TODO:
        UNREACHABLE();
        return ERR_NOT_SUPPORTED();
    }
    
    std::string ToString() const override { return "libc-file"; }
private:
    FILE *file_;
}; // class LibCWritableFile

class MemoryWritableFile final : public WritableFile {
public:
    MemoryWritableFile(std::string *buf): buf_(buf) {}
    ~MemoryWritableFile() override {}
    
    Status Append(std::string_view data) override {
        buf_->append(data.data(), data.size());
        return Status::OK();
    }
    
    Status PositionedAppend(std::string_view data, uint64_t offset) override {
        if (offset + buf_->size() >= buf_->size()) {
            Truncate(offset + buf_->size());
        }
        ::memcpy(&(*buf_)[0] + offset, data.data(), data.size());
        return Status::OK();
    }
    
    Status Truncate(uint64_t size) override {
        buf_->resize(size, 0);
        return Status::OK();
    }
    
    std::string ToString() const override { return *buf_; }
    
private:
    std::string *const buf_;
}; // class MemoryWritableFile


const char *PrintingWriter::kInlineIndents[PrintingWriter::kMaxInlineIndents] = {
    "",
    "    ",
    "        ",
    "            ",
    "                ",
    "                    ",
};

PrintingWriter *PrintingWriter::Print(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    std::string str(Vsprintf(fmt, ap));
    va_end(ap);
    return Write(str);
}

PrintingWriter *PrintingWriter::Println(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    std::string str(Vsprintf(fmt, ap));
    va_end(ap);
    return Writeln(str);
}


SequentialFile *NewMemorySequentialFile(const std::string &buf) {
    return new MemorySequentialFile(buf);
}

SequentialFile *NewMemorySequentialFile(const char *z, size_t n) {
    return new MemorySequentialFile(z, n);
}

WritableFile *NewWritableFile(FILE *fp) { return new LibCWritableFile(fp); }

WritableFile *NewMemoryWritableFile(std::string *buf) { return new MemoryWritableFile(buf); }

} // namespace base

} // namespace yalx
