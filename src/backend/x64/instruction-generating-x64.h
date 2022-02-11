#pragma once
#ifndef YALX_BACKEND_X64_INSTRUCTION_GENERATING_X64_H_
#define YALX_BACKEND_X64_INSTRUCTION_GENERATING_X64_H_

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
class RegisterConfiguration;
class InstructionFunction;

class X64InstructionGenerator final {
public:
    X64InstructionGenerator(base::Arena *arena, ir::Module *module, ConstantsPool *const_pool, LinkageSymbols *symbols,
                            int optimizing_level);
    
    void Run();
    
    void MoveFuns(base::ArenaMap<std::string_view, InstructionFunction *> *funs) {
        *funs = std::move(funs_);
    }
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(X64InstructionGenerator);
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
}; // class X64InstructionGenerator

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_X64_INSTRUCTION_GENERATING_X64_H_
