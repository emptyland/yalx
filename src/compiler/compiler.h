#pragma once
#ifndef YALX_COMPILER_COMPILER_H_
#define YALX_COMPILER_COMPILER_H_

#include "base/status.h"
#include "base/base.h"
#include <string>
#include <string_view>
#include <vector>

namespace yalx {
namespace base {
class Arena;
template <class T> class ArenaVector;
} // namespace base
namespace cpl {

class Package;
class SyntaxFeedback;

class Compiler final {
public:
    /**
     * Pragma entry
     */
    static int Main(int argc, char *argv[]);
    
    static base::Status Build(const std::string &project_dir, const std::string &base_lib, int optimization);

    static base::Status ParseAllSourceFiles(const std::vector<std::string> &files);
    
    // Source Files Structure:
    // 1. pkg // external package files.
    // 2. bin // target files.
    // 3. src // project source files.
    //
    // Example:
    //
    // pkg
    // bin
    //   |- project
    // src
    //   +- main
    //     |- main.yalx
    //     |- major.yalx
    //     |- moon.yalx
    //   +- foo
    //     |- foo.yalx
    //   +- bar
    //     |- bar.yalx
    static base::Status FindAndParseMainSourceFiles(const std::string &project_dir,
                                                    base::Arena *arena,
                                                    SyntaxFeedback *error_feedback,
                                                    Package **receiver);
    
    static base::Status FindAndParseAllDependencesSourceFiles(const std::string &project_dir,
                                                              const std::string &base_lib,
                                                              base::Arena *arena,
                                                              SyntaxFeedback *error_feedback,
                                                              Package *root,
                                                              base::ArenaVector<Package *> *entries);
    
    static constexpr char kSourceExtendedName[] = ".yalx";
    static constexpr char kSourceDirName[] = "src";
    static constexpr char kMainPkgName[] = "main";
    
    DISALLOW_ALL_CONSTRUCTORS(Compiler);
}; // class Compiler


} // namespace cpl

} // namespace yalx


#endif // YALX_COMPILER_COMPILER_H_
