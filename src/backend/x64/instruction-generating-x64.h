#pragma once
#ifndef YALX_BACKEND_INSTRUCTION_GENERATING_H_
#define YALX_BACKEND_INSTRUCTION_GENERATING_H_

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
class RegisterConfiguration;

class X64InstructionGenerator final {
public:
    X64InstructionGenerator(base::Arena *arena, ir::Module *module, ConstantsPool *const_pool);
    
    void Run();
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(X64InstructionGenerator);
private:
    void GenerateFun(ir::StructureModel *owns, ir::Function *fun);
    void PrepareGlobalValues();
    std::tuple<int, bool> UniquifyConstant(ir::Value *kval);
    
    base::Arena *const arena_;
    ir::Module *const module_;
    ConstantsPool *const const_pool_;
}; // class X64InstructionGenerator

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_INSTRUCTION_GENERATING_H_
