#include "base/io.h"
#include "base/checking.h"
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


SequentialFile *NewMemorySequentialFile(const std::string &buf) {
    return new MemorySequentialFile(buf);
}

SequentialFile *NewMemorySequentialFile(const char *z, size_t n) {
    return new MemorySequentialFile(z, n);
}

} // namespace base

} // namespace yalx
