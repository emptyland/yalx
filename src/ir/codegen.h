#pragma once
#ifndef YALX_IR_CODEGEN_H_
#define YALX_IR_CODEGEN_H_

#include "compiler/global-symbol.h"
#include "ir/type.h"
#include "ir/scope.h"
#include "base/arena-utils.h"
#include "base/status.h"
#include "base/base.h"

namespace yalx {
namespace cpl {
class Package;
class FunctionPrototype;
class Type;
class SyntaxFeedback;
} // namespace cpl
namespace ir {

class Module;
class Function;
class PrototypeModel;
class StructureModel;
class Model;
class Value;
class IRGeneratorAstVisitor;
class OperatorsFactory;
class PackageScope;

class IntermediateRepresentationGenerator {
public:
    IntermediateRepresentationGenerator(const std::unordered_map<std::string_view, cpl::GlobalSymbol> &symbols,
                                        base::Arena *arena,
                                        cpl::Package *entry,
                                        cpl::SyntaxFeedback *error_feedback);
    
    base::Status Run();
    
    void MoveModules(base::ArenaMap<std::string_view, Module *> *modules) { *modules = std::move(modules_); }
    
    friend class IRGeneratorAstVisitor;
    DISALLOW_IMPLICIT_CONSTRUCTORS(IntermediateRepresentationGenerator);
private:
    base::Status Prepare0();
    base::Status Prepare1();
    void PreparePackage0(cpl::Package *pkg);
    void PreparePackage1(cpl::Package *pkg);
    Function *InstallInitFun(Module *module);
    Type BuildType(const cpl::Type *type);
    Model *Boxing(Type type);
    PrototypeModel *BuildPrototype(const cpl::FunctionPrototype *ast, StructureModel *owns = nullptr);
    base::Status RecursivePackage(cpl::Package *root, std::function<void(cpl::Package *)> &&callback);
    
    Module *AssertedGetModule(std::string_view name) const {
        auto iter = modules_.find(name);
        assert(iter != modules_.end());
        return iter->second;
    }
    
    Value *AssertedGetVal(std::string_view name) const { return DCHECK_NOTNULL(FindValOrNull(name)); }
    
    Value *FindValOrNull(std::string_view name) const {
        auto iter = symbols_.find(name);
        return (iter == symbols_.end() || iter->second.kind != Symbol::kValue) ? nullptr : iter->second.core.value;
    }
    
    Model *AssertedGetUdt(std::string_view name) const { return DCHECK_NOTNULL(FindUdtOrNull(name)); }
    
    Model *FindUdtOrNull(std::string_view name) const {
        auto iter = symbols_.find(name);
        return (iter == symbols_.end() || iter->second.kind != Symbol::kModel) ? nullptr : iter->second.core.model;
    }
    
    Function *AssertedGetFun(std::string_view name) const { return DCHECK_NOTNULL(FindFunOrNull(name)); }
    
    Function *FindFunOrNull(std::string_view name) const {
        auto iter = symbols_.find(name);
        return (iter == symbols_.end() || iter->second.kind != Symbol::kFun) ? nullptr : iter->second.core.fun;
    }
    
    Symbol AssertedGetSymbol(std::string_view name) const {
        auto iter = symbols_.find(name);
        assert(iter != symbols_.end());
        return iter->second;
    }
    
    PackageScope *AssertedGetPackageScope(cpl::Package *key) const {
        auto iter = pkg_scopes_.find(key);
        assert(iter != pkg_scopes_.end());
        return iter->second;
    }
    
    bool Track(Module *module, int dest) {
        if (auto iter = track_.find(module); iter != track_.end()) {
            return true;
        }
        track_[module] = dest;
        return false;
    }
    
    const std::unordered_map<std::string_view, cpl::GlobalSymbol> &ast_nodes_;
    base::Arena *const arena_;
    cpl::Package *entry_;
    cpl::SyntaxFeedback *error_feedback_;
//    base::ArenaMap<std::string_view, Model *> global_udts_;
//    base::ArenaMap<std::string_view, Value *> global_vars_;
//    base::ArenaMap<std::string_view, Function *> global_funs_;
    base::ArenaMap<std::string_view, Symbol> symbols_;
    base::ArenaMap<std::string_view, Module *> modules_;
    base::ArenaMap<cpl::Package *, PackageScope *> pkg_scopes_;
    base::ArenaMap<Module *, int> track_;
    OperatorsFactory *ops_;
    Value *nil_val_ = nullptr;
    Value *unit_val_ = nullptr;
    Value *true_val_ = nullptr;
    Value *false_val_ = nullptr;
}; // class IntermediateRepresentationGenerator


} // namespace ir

} // namespace yalx

#endif // YALX_IR_CODEGEN_H_
