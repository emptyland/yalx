#pragma once
#ifndef YALX_BACKEND_ARM64_CODE_GENERATE_ARM64_H_
#define YALX_BACKEND_ARM64_CODE_GENERATE_ARM64_H_

#include "base/arena-utils.h"

namespace yalx {
namespace base {
class PrintingWriter;
} // namespace base
namespace ir {
class Module;
} // namespace ir
namespace backend {

class ConstantsPool;
class LinkageSymbols;
class InstructionFunction;

class Arm64CodeGenerator final {
public:
    class FunctionGenerator;
    
    Arm64CodeGenerator(const base::ArenaMap<std::string_view, InstructionFunction *> &funs,
                       ir::Module *module,
                       ConstantsPool *const_pool,
                       LinkageSymbols *symbols,
                       base::PrintingWriter *printer);
    
    void EmitAll();
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Arm64CodeGenerator);
private:
    const base::ArenaMap<std::string_view, InstructionFunction *> &funs_;
    ir::Module *const module_;
    ConstantsPool *const const_pool_;
    LinkageSymbols *const symbols_;
    base::PrintingWriter *const printer_;
}; // class Arm64CodeGenerator

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_ARM64_CODE_GENERATE_ARM64_H_