#pragma once
#ifndef YALX_BACKEND_ARM64_INSTRUCTION_GENERATING_ARM64_H_
#define YALX_BACKEND_ARM64_INSTRUCTION_GENERATING_ARM64_H_

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
class StackConfiguration;

class Arm64InstructionGenerator final {
public:
    Arm64InstructionGenerator(base::Arena *arena, ir::Module *module, ConstantsPool *const_pool);
    
    void Run();
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Arm64InstructionGenerator);
private:    
    base::Arena *const arena_;
    ir::Module *const module_;
    ConstantsPool *const const_pool_;
}; // class X64InstructionGenerator

const StackConfiguration *Arm64StackConf();

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_ARM64_INSTRUCTION_GENERATING_ARM64_H_
