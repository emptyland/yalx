#include "base/env.h"
#include "base/io.h"
#include <unistd.h>
#include <stdio.h>

namespace yalx {

namespace base {

int Env::kOSPageSize = 0;

Status Env::Init() {
    kOSPageSize = ::getpagesize();
    return Status::OK();
}

namespace {

class PosixSequentialFile final : public SequentialFile {
public:
    explicit PosixSequentialFile(FILE *fp) : fp_(DCHECK_NOTNULL(fp)) {}
    ~PosixSequentialFile() override { ::fclose(fp_); }
    
    Status Read(size_t n, std::string_view *result, std::string *scratch) override {
        scratch->resize(n);
        size_t rs = ::fread(&(*scratch)[0], 1, n, fp_);
        if (::feof(fp_)) {
            return Status::Eof();
        }
        if (::ferror(fp_)) {
            return ERR_PERROR("Read sequential file fail");
        }
        scratch->resize(rs);
        *result = *scratch;
        return Status::OK();
    }

    Status Skip(size_t n) override {
        if (::fseek(fp_, n, SEEK_CUR) < 0) {
            return ERR_PERROR("Skip sequenial file fail");
        }
        return Status::OK();
    }

    Status Available(size_t *size) override {
        long current = ::ftell(fp_);
        if (current < 0) {
            return ERR_PERROR("Get file size fail");
        }
        if (::fseek(fp_, 0, SEEK_END) < 0) {
            return ERR_PERROR("Get file size fail");
        }
        long end = ::ftell(fp_);
        if (end < 0) {
            return ERR_PERROR("Get file size fail");
        }
        *size = end - current;
        if (::fseek(fp_, current, SEEK_SET) < 0) {
            return ERR_PERROR("Get file size fail");
        }
        return Status::OK();
    }
private:
    FILE *fp_;
}; // class PosixSequentialFile


class PosixWritableFile final : public WritableFile {
public:
    explicit PosixWritableFile(FILE *fp) : fp_(DCHECK_NOTNULL(fp)) {}
    ~PosixWritableFile() override { ::fclose(fp_); }
    
    Status Append(std::string_view data) override {
        auto rs = ::fwrite(data.data(), 1, data.size(), fp_);
        USE(rs);
        assert(rs == data.size());
        if (::ferror(fp_)) {
            return ERR_PERROR("Write file fail");
        }
        return Status::OK();
    }
    Status PositionedAppend(std::string_view data, uint64_t offset) override {
        if (::fseek(fp_, offset, SEEK_SET)) {
            return ERR_PERROR("Seek file fail");
        }
        return Append(data);
    }
    Status Truncate(uint64_t size) override {
        if (::ftruncate(fileno(fp_), size) < 0) {
            return ERR_PERROR("Truncate file fail");
        }
        return Status::OK();
    }
    std::string ToString() const override {
        return "PosixWritableFile";
    }
private:
    FILE *fp_;
}; // class PosixWritableFile

} // namespace

Status Env::NewSequentialFile(const std::string &name, std::unique_ptr<SequentialFile> *file) {
    FILE *fp = ::fopen(name.c_str(), "rb");
    if (fp == nullptr) {
        return ERR_PERROR("Can not open file");
    }
    file->reset(new PosixSequentialFile(fp));
    return Status::OK();
}

Status Env::NewWritableFile(const std::string &name, bool append, std::unique_ptr<WritableFile> *file) {
    FILE *fp = ::fopen(name.c_str(), append ? "ab" : "wb");
    if (fp == nullptr) {
        return ERR_PERROR("Can not open file");
    }
    file->reset(new PosixWritableFile(fp));
    return Status::OK();
}

} // namespace base

} // namespace yalx
