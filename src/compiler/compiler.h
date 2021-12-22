#pragma once
#ifndef YALX_COMPILER_COMPILER_H_
#define YALX_COMPILER_COMPILER_H_

#include "compiler/global-symbol.h"
#include "base/arena-utils.h"
#include "base/status.h"
#include "base/base.h"
#include <string>
#include <string_view>
#include <vector>
#include <set>

namespace yalx {
namespace ir {
class Module;
} // namespace ir
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
    //   +- github.com
    //     +- emptyland
    //       +- demo
    //         |- demo.yalx
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
    static base::Status FindAndParseProjectSourceFiles(const std::string &project_dir,
                                                       const std::string &base_lib,
                                                       base::Arena *arena,
                                                       SyntaxFeedback *error_feedback,
                                                       Package **main_pkg,
                                                       base::ArenaVector<Package *> *entries,
                                                       base::ArenaMap<std::string_view, Package *> *all);
    
    static base::Status FindAndParseMainSourceFiles(const std::string &project_dir,
                                                    base::Arena *arena,
                                                    SyntaxFeedback *error_feedback,
                                                    Package **receiver);
    
    static base::Status FindAndParseAllDependencesSourceFiles(const std::vector<std::string> &search_paths,
                                                              base::Arena *arena,
                                                              SyntaxFeedback *error_feedback,
                                                              Package *root,
                                                              base::ArenaMap<std::string_view, Package *> *all);
    
    static base::Status ParsePackageSourceFiles(std::string_view pkg_dir,
                                                std::string_view import_path,
                                                base::Arena *arena,
                                                SyntaxFeedback *error_feedback,
                                                Package **receiver);
    
    static base::Status ReducePackageDependencesType(Package *entry,
                                                     base::Arena *arena,
                                                     SyntaxFeedback *error_feedback,
                                                     std::unordered_map<std::string_view, GlobalSymbol> *symbols);
    
     // intermediate representation
    static base::Status
    GenerateIntermediateRepresentationCode(const std::unordered_map<std::string_view, GlobalSymbol> &symbols,
                                           base::Arena *arena,
                                           Package *entry,
                                           SyntaxFeedback *error_feedback,
                                           base::ArenaVector<ir::Module *> *modules);
    
    static constexpr char kSourceExtendedName[] = ".yalx";
    static constexpr char kSourceDirName[] = "src";
    static constexpr char kPackageDirName[] = "pkg";
    static constexpr char kMainPkgName[] = "main";
    static constexpr char kDefaultImport[] = "yalx/lang";
    
    DISALLOW_ALL_CONSTRUCTORS(Compiler);
}; // class Compiler


} // namespace cpl

} // namespace yalx


#endif // YALX_COMPILER_COMPILER_H_
