#pragma once
#ifndef YALX_COMPILER_COMPILER_H_
#define YALX_COMPILER_COMPILER_H_

#include "base/status.h"
#include "base/base.h"


namespace yalx {
    
namespace cpl {


class Compiler final {
public:
    /**
     * Pragma entry
     */
    static int Main(int argc, char *argv[]);
    
    DISALLOW_ALL_CONSTRUCTORS(Compiler);
}; // class Compiler


} // namespace cpl

} // namespace yalx


#endif // YALX_COMPILER_COMPILER_H_
