#pragma once
#ifndef YALX_BACKEND_X64_CODEGEN_X64_H_
#define YALX_BACKEND_X64_CODEGEN_X64_H_

#include "base/arena-utils.h"
#include "base/base.h"

namespace yalx {
namespace base {
class Arena;
class ArenaString;
class PrintingWriter;
} // namespace base
namespace ir {
class Module;
} // namespace ir
namespace backend {

using ModulesMap = base::ArenaMap<std::string_view, ir::Module *>;

class X64AssemblyCodeGenerator {
public:
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(X64AssemblyCodeGenerator);
private:
    base::Arena *const arena_;
    base::PrintingWriter *const emitter_;
    ModulesMap *modules_;
}; // class X64NativeAssemblyCodeGenerator

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_X64_CODEGEN_X64_H_
