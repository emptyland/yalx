#include "base/env.h"
#include "base/io.h"
#if defined(YALX_OS_POSIX)
#include <unistd.h>
#endif

#include <stdio.h>

#include <memory>

namespace yalx::base {

int Env::kOSPageSize = 0;

Status Env::Init() {
#if defined(YALX_OS_POSIX)
    kOSPageSize = ::getpagesize();
#endif

#if defined(YALX_OS_WINDOWS)
    kOSPageSize = 4 * base::kKB;
#endif
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
        // ::SetEndOfFile()
    #if defined(YALX_OS_WINDOWS)
        if (!::SetEndOfFile(os_handle())) {
    #else
        if (::ftruncate(fileno(fp_), size) < 0) {
    #endif
            return ERR_PERROR("Truncate file fail");
        }
        return Status::OK();
    }
    [[nodiscard]] std::string ToString() const override {
        return "PosixWritableFile";
    }

#if defined(YALX_OS_WINDOWS)
    HANDLE os_handle() {
        int stdc_handle = _fileno(fp_);
        HANDLE h = reinterpret_cast<HANDLE>(_get_osfhandle(stdc_handle));
        DCHECK(h != INVALID_HANDLE_VALUE);
        return h;
    }
#endif
private:
    FILE *fp_;
}; // class PosixWritableFile

} // namespace

Status Env::NewSequentialFile(const std::string &name, std::unique_ptr<SequentialFile> *file) {
#if defined(YALX_OS_WINDOWS)
    FILE *fp = nullptr;
    if (::fopen_s(&fp, name.c_str(), "rb") != 0) {
        return ERR_PERROR("Can not open file");
    }
#else
    FILE *fp = ::fopen(name.c_str(), "rb");
    if (fp == nullptr) {
        return ERR_PERROR("Can not open file");
    }
#endif

    *file = std::make_unique<PosixSequentialFile>(fp);
    return Status::OK();
}

Status Env::NewWritableFile(const std::string &name, bool append, std::unique_ptr<WritableFile> *file) {
#if defined(YALX_OS_WINDOWS)
    FILE *fp = nullptr;
    if (::fopen_s(&fp, name.c_str(), append ? "ab" : "wb") != 0) {
        return ERR_PERROR("Can not open file");
    }
#else
    FILE *fp = ::fopen(name.c_str(), append ? "ab" : "wb");
    if (fp == nullptr) {
        return ERR_PERROR("Can not open file");
    }
#endif
    *file = std::make_unique<PosixWritableFile>(fp);
    return Status::OK();
}

} // namespace yalx
