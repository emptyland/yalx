#pragma once
#ifndef YALX_BACKEND_ARM64_CODE_GENERATE_ARM64_H_
#define YALX_BACKEND_ARM64_CODE_GENERATE_ARM64_H_

#include "backend/gnu-asm-generator.h"
#include "base/arena-utils.h"

namespace yalx {
namespace backend {

class Arm64CodeGenerator final : public GnuAsmGenerator {
public:
    class FunctionGenerator;
    
    Arm64CodeGenerator(const base::ArenaMap<std::string_view, InstructionFunction *> &funs,
                       ir::Module *module,
                       ConstantsPool *const_pool,
                       LinkageSymbols *symbols,
                       base::PrintingWriter *printer);
    ~Arm64CodeGenerator() override;

    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Arm64CodeGenerator);
private:
    void EmitFunction(InstructionFunction *fun) override;
}; // class Arm64CodeGenerator

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_ARM64_CODE_GENERATE_ARM64_H_
