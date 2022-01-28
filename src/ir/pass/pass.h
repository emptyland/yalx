#pragma once
#ifndef YALX_IR_PASS_PASS_H_
#define YALX_IR_PASS_PASS_H_

#include "compiler/syntax-feedback.h"
#include "ir/node.h"
#include "ir/metadata.h"
#include "base/arena-utils.h"
#include "base/base.h"

namespace yalx {
namespace base {
class Arena;
} // namespace base
namespace ir {

class OperatorsFactory;

template<class T>
class Pass {
public:
    using ModulesMap = base::ArenaMap<std::string_view, Module *>;
    
    Pass(base::Arena *arena, OperatorsFactory *ops, ModulesMap *modules, cpl::SyntaxFeedback *feedback)
    : arena_(arena)
    , ops_(ops)
    , modules_(modules)
    , feedback_(feedback) {}

    DEF_PTR_GETTER(base::Arena, arena);
    DEF_PTR_GETTER(OperatorsFactory, ops);
    DEF_PTR_GETTER(ModulesMap, modules);
    DEF_PTR_GETTER(Module, current_module);
    DEF_PTR_GETTER(StructureModel, current_udt);
    DEF_PTR_GETTER(Function, current_fun);
    DEF_PTR_GETTER(BasicBlock, current_block);
    DEF_PTR_GETTER(cpl::SyntaxFeedback, feedback);
    
    void Run() {
        for (auto [name, module] : *modules()) {
            current_module_ = module;
            static_cast<T *>(this)->RunModule(module);
            current_module_ = nullptr;
        }
    }
    
    void ForeachUdt(Module *module) {
        for (auto udt : module->structures()) {
            current_udt_ = udt;
            static_cast<T *>(this)->RunUdt(udt);
            current_udt_ = nullptr;
        }
    }
    
    void ForeachMethod(StructureModel *udt) {
        for (auto method : udt->methods()) {
            current_fun_ = method.fun;
            static_cast<T *>(this)->RunFun(method.fun);
            current_fun_ = nullptr;
        }
    }
    
    void ForeachFunction(Module *module) {
        for (auto fun : module->funs()) {
            current_fun_ = fun;
            static_cast<T *>(this)->RunFun(fun);
            current_fun_ = nullptr;
        }
    }
    
    void ForeachBasicBlock(Function *fun) {
        for (auto blk : fun->blocks()) {
            current_block_ = blk;
            static_cast<T *>(this)->RunBasicBlock(blk);
            current_block_ = nullptr;
        }
    }
    
    constexpr const char *name() const { return T::kPassName; }
    constexpr const int level() const { return T::kPassLevel; }
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Pass);
private:
    base::Arena *const arena_;
    OperatorsFactory *const ops_;
    ModulesMap *modules_;
    cpl::SyntaxFeedback *feedback_;
    
    Module *current_module_ = nullptr;
    StructureModel *current_udt_ = nullptr;
    Function *current_fun_ = nullptr;
    BasicBlock *current_block_ = nullptr;
};

} // namespace ir

} // namespace yalx

#endif // YALX_IR_PASS_PASS_H_
