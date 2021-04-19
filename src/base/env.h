#pragma once
#ifndef YALX_BASE_ENV_H_
#define YALX_BASE_ENV_H_

#include "base/status.h"
#include <memory>

namespace yalx {

namespace base  {

class SequentialFile;

class Env {
public:
    // std::unique_ptr<SequentialFile> file;
    // auto rs = Env::NewSequentialFile("file", &file);
    // if (rs.fail()) {
    //     return rs;
    // }
    static Status NewSequentialFile(const std::string &name, std::unique_ptr<SequentialFile> *file);
};



} // namespace base

} // namespace yalx



#endif // YALX_BASE_ENV_H_