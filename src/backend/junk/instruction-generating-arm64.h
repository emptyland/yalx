#pragma once
#ifndef YALX_BACKEND_ARM64_INSTRUCTION_GENERATING_ARM64_H_
#define YALX_BACKEND_ARM64_INSTRUCTION_GENERATING_ARM64_H_

#include "base/arena-utils.h"
#include "base/base.h"
#include <tuple>
#include <memory>

namespace yalx {
namespace ir {
class StructureModel;
class Function;
class Module;
class Value;
} // namespace ir
namespace base {
class Arena;
} // namespace base
namespace backend {

class ConstantsPool;
class LinkageSymbols;
class StackConfiguration;
class RegisterConfiguration;
class InstructionFunction;
class InstructionBlockLabelGenerator;

class Arm64InstructionGenerator final {
public:
    Arm64InstructionGenerator(base::Arena *arena, ir::Module *module, ConstantsPool *const_pool, LinkageSymbols *symbols,
                              int optimizing_level);
    ~Arm64InstructionGenerator();
    
    void Run();
    
    void MoveFuns(base::ArenaMap<std::string_view, InstructionFunction *> *funs) {
        *funs = std::move(funs_);
    }
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Arm64InstructionGenerator);
private:
    void GenerateFun(ir::StructureModel *owns, ir::Function *fun);
    void PrepareGlobalValues();
    std::tuple<int, bool> UniquifyConstant(ir::Value *kval);

    base::Arena *const arena_;
    ir::Module *const module_;
    ConstantsPool *const const_pool_;
    LinkageSymbols *const symbols_;
    const int optimizing_level_;
    base::ArenaMap<std::string_view, InstructionFunction *> funs_;
    std::unique_ptr<InstructionBlockLabelGenerator> lables_;
}; // class X64InstructionGenerator

const StackConfiguration *Arm64StackConf();
const RegisterConfiguration *Arm64RegisterConf();

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_ARM64_INSTRUCTION_GENERATING_ARM64_H_
