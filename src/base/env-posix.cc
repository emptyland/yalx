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
    PosixSequentialFile(FILE *fp) : fp_(DCHECK_NOTNULL(fp)) {}
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

    Status GetFileSize(size_t *size) override {
        long saved = ::ftell(fp_);
        if (saved < 0) {
            return ERR_PERROR("Get file size fail");
        }
        if (::fseek(fp_, 0, SEEK_END) < 0) {
            return ERR_PERROR("Get file size fail");
        }
        long current = ::ftell(fp_);
        if (current < 0) {
            return ERR_PERROR("Get file size fail");
        }
        *size = current;
        if (::fseek(fp_, saved, SEEK_SET) < 0) {
            return ERR_PERROR("Get file size fail");
        }
        return Status::OK();
    }
private:
    FILE *fp_;
}; // class PosixSequentialFile



} // namespace

Status Env::NewSequentialFile(const std::string &name, std::unique_ptr<SequentialFile> *file) {
    FILE *fp = ::fopen(name.c_str(), "r");
    if (fp == nullptr) {
        return ERR_PERROR("Can not open file");
    }
    file->reset(new PosixSequentialFile(fp));
    return Status::OK();
}


} // namespace base

} // namespace yalx
