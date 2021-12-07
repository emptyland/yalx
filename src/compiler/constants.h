#pragma once
#ifndef YALX_COMPILER_CONSTANTS_H_
#define YALX_COMPILER_CONSTANTS_H_

#include "base/base.h"

namespace yalx {

namespace cpl {

class Constants final {
public:
    constexpr static char kLangPackageFullName[] = "yalx/lang:lang";
    constexpr static char kAnyClassName[] = "Any";
    
    DISALLOW_ALL_CONSTRUCTORS(Constants);
}; // class Constants

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_CONSTANTS_H_
