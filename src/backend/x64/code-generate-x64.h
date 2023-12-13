#pragma once
#ifndef YALX_BACKEND_X64_CODE_GENERATE_X64_H_
#define YALX_BACKEND_X64_CODE_GENERATE_X64_H_

#include "backend/gnu-asm-generator.h"
#include "base/arena-utils.h"

namespace yalx::backend {

class ConstantsPool;
class Linkage;
class InstructionFunction;

class X64CodeGenerator final : public GnuAsmGenerator {
public:
    class FunctionGenerator;
    
    X64CodeGenerator(const base::ArenaMap<std::string_view, InstructionFunction *> &funs,
                     const RegistersConfiguration *profile,
                     ir::Module *module,
                     ConstantsPool *const_pool,
                     Linkage *symbols,
                     base::PrintingWriter *printer);
    ~X64CodeGenerator() override;

    friend class X64CodeGeneratorTest;
    DISALLOW_IMPLICIT_CONSTRUCTORS(X64CodeGenerator);
private:
    void EmitFunction(InstructionFunction *fun) override;
}; // class X64CodeGenerator

} // namespace yalx

#endif // YALX_BACKEND_X64_CODE_GENERATE_X64_H_
