#include "base/env.h"
#include <unistd.h>

namespace yalx {

namespace base {

int Env::kOSPageSize = 0;

Status Env::Init() {
    kOSPageSize = ::getpagesize();
    return Status::OK();
}

} // namespace base

} // namespace yalx
