#pragma once
#ifndef YALX_COMPILER_COMPILER_H_
#define YALX_COMPILER_COMPILER_H_

#include "base/status.h"
#include "base/base.h"
#include <string>
#include <string_view>
#include <vector>

namespace yalx {
    
namespace cpl {


class Compiler final {
public:
    /**
     * Pragma entry
     */
    static int Main(int argc, char *argv[]);
    
    static base::Status Build(const std::string &project_dir, const std::string &base_lib, int optimization);

    static base::Status ParseAllSourceFiles(const std::vector<std::string> files);
    
    static constexpr char kSourceExtendedName[] = ".yalx";
    
    DISALLOW_ALL_CONSTRUCTORS(Compiler);
}; // class Compiler


} // namespace cpl

} // namespace yalx


#endif // YALX_COMPILER_COMPILER_H_
