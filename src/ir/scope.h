#pragma once
#ifndef YALX_IR_SCOPE_H_
#define YALX_IR_SCOPE_H_

#include "ir/metadata.h"
#include "compiler/ast.h"
#include "base/arena-utils.h"
#include "base/status.h"
#include "base/checking.h"
#include "base/base.h"
#include <unordered_map>
#include <unordered_set>

namespace yalx {

namespace ir {

class NamespaceScope;
class PackageScope;
class FileUnitScope;
class StructureScope;
class FunctionScope;
class BranchScope;

class Handle;
class Model;
class Value;
class Function;
class BasicBlock;

struct Symbol {
    enum Kind {
        kNotFound,
        kUnknown,
        kModel,
        kValue,
        kFun,
        kHandle,
        kCaptured,
    };
    Kind kind;
    cpl::Statement *node;
    BasicBlock     *block;
    union {
        Model *model;
        Value *value;
        Function *fun;
        Handle *handle;
    } core;
    NamespaceScope *owns;
    
    bool IsFound() const { return !IsNotFound(); }
    bool IsNotFound() const { return kind == kNotFound; }
    
    static Symbol NotFound() { return {.kind = kNotFound}; }
    
    static Symbol Val(NamespaceScope *owns, Value *value, BasicBlock *block, cpl::Statement *node = nullptr) {
        return {
            .kind  = kValue,
            .node  = node,
            .block = block,
            .core  = {
                .value = value
            },
            .owns  = owns,
        };
    }
    
    static Symbol Udt(NamespaceScope *owns, Model *model, cpl::Statement *node = nullptr) {
        return {
            .kind = kModel,
            .node = node,
            .block = nullptr,
            .core = {
                .model = model
            },
            .owns = owns,
        };
    }
    
    static Symbol Fun(NamespaceScope *owns, Function *fun, cpl::Statement *node = nullptr) {
        return {
            .kind = kFun,
            .node = node,
            .block = nullptr,
            .core = {
                .fun = fun
            },
            .owns = owns,
        };
    }
    
    static Symbol Had(NamespaceScope *owns, Handle *handle, cpl::Statement *node = nullptr) {
        return {
            .kind = kHandle,
            .node = node,
            .block = nullptr,
            .core = {
                .handle = handle
            },
            .owns = owns,
        };
    }
    
    static Symbol Cap(NamespaceScope *owns, Handle *handle, cpl::Statement *node = nullptr) {
        return {
            .kind = kCaptured,
            .node = node,
            .block = nullptr,
            .core = {
                .handle = handle
            },
            .owns = owns,
        };
    }
};


class NamespaceScope {
public:
    template<class T> class Keeper {
    public:
        Keeper(T *ns): ns_(ns) { ns_->Enter(); }
        ~Keeper() { ns_->Exit(); }
        
        T *ns() const { return ns_; }
        
        T *operator ->() const { return ns(); }
    private:
        T *ns_;
    };

    virtual ~NamespaceScope() {};
    
    DEF_PTR_GETTER(NamespaceScope, prev);
    DEF_VAL_GETTER(int, level);
    
    inline bool IsFileUnitScope();
    inline bool IsStructureScope();
    
    virtual PackageScope *NearlyPackageScope() { return !prev_ ? nullptr : prev_->NearlyPackageScope(); }
    virtual FileUnitScope *NearlyFileUnitScope() { return !prev_ ? nullptr : prev_->NearlyFileUnitScope(); }
    virtual StructureScope *NearlyStructureScope() { return !prev_ ? nullptr : prev_->NearlyStructureScope(); }
    virtual FunctionScope *NearlyFunctionScope() { return !prev_ ? nullptr : prev_->NearlyFunctionScope(); }
    virtual BranchScope *NearlyBranchScope() { return !prev_ ? nullptr : prev_->NearlyBranchScope(); }
    
    virtual Symbol FindLocalSymbol(std::string_view name) const;
    virtual Symbol FindSymbol(std::string_view name) const;

    void PutValue(std::string_view name, Value *value, BasicBlock *block = nullptr) {
        PutSymbol(name, Symbol::Val(this, value, block));
    }
    virtual void PutSymbol(std::string_view name, const Symbol &symbol);
    
    virtual NamespaceScope *Trunk() const { return nullptr; }
    
    void Enter() {
        assert(location_ != nullptr);
        prev_ = *location_;
        assert(*location_ != this);
        *location_ = this;
        level_ = !prev_ ? 1 : prev_->level() + 1;
    }
    
    void Exit() {
        assert(*location_ == this);
        *location_ = prev_;
        level_ = -1;
    }
protected:
    NamespaceScope(NamespaceScope **location): location_(location) {}
    
    NamespaceScope **location_;
    NamespaceScope *prev_ = nullptr;
    int level_ = 0;
    std::unordered_map<std::string_view, Symbol> symbols_;
}; // class IRCodeEnvScope

class PackageScope : public NamespaceScope {
public:
    PackageScope(NamespaceScope **location, cpl::Package *pkg, base::ArenaMap<std::string_view, Symbol> *global);
    ~PackageScope() override;
    
    DEF_PTR_GETTER(cpl::Package, pkg);
    
    void Enter(NamespaceScope **location);
    PackageScope *NearlyPackageScope() override;
    
    bool Track(cpl::AstNode *ast) {
        if (auto iter = track_.find(ast); iter != track_.end()) {
            return true;
        }
        track_.insert(ast);
        return false;
    }
    
    bool HasTracked(cpl::AstNode *ast) { return track_.find(ast) != track_.end(); }
    bool HasNotTracked(cpl::AstNode *ast) { return !HasTracked(ast); }
    
    FileUnitScope *FindFileUnitScopeOrNull(cpl::Node *key) {
        auto iter = files_ptrs_.find(key);
        return iter == files_ptrs_.end() ? nullptr : files_[iter->second];
    }

private:
    cpl::Package *pkg_;
    std::unordered_map<cpl::Node *, size_t> files_ptrs_;
    std::vector<FileUnitScope *> files_;
    std::unordered_set<cpl::AstNode *> track_;
}; // class IRCodePkgScope

class FileUnitScope : public NamespaceScope {
public:
    FileUnitScope(NamespaceScope **location, cpl::FileUnit *file_unit,
                  base::ArenaMap<std::string_view, Symbol> *symobls);
    ~FileUnitScope() override;
    
    DEF_PTR_GETTER(cpl::FileUnit, file_unit);
    
    FileUnitScope *NearlyFileUnitScope() override;
    
    Symbol FindLocalSymbol(std::string_view name) const override;
    Symbol FindExportSymbol(std::string_view prefix, std::string_view name) const;
    
private:
    Symbol Lookup(std::string_view name) const;
    
    cpl::FileUnit *file_unit_;
    std::map<std::string_view, std::string> alias_;
    std::vector<std::string> implicit_alias_;
    const base::ArenaMap<std::string_view, Symbol> *proxy_symbols_;
}; // class IRCodeFileScope

class StructureScope : public NamespaceScope {
public:
    StructureScope(NamespaceScope **location, const cpl::IncompletableDefinition *definition, StructureModel *model);
    ~StructureScope() override;
    
    void InstallAncestorsSymbols();
    
    DEF_PTR_GETTER(const cpl::IncompletableDefinition, ast);
    DEF_PTR_GETTER(StructureModel, model);
    
    StructureScope *NearlyStructureScope() override;
    FunctionScope *NearlyFunctionScope() override;
    
private:
    const cpl::IncompletableDefinition *ast_;
    StructureModel *model_;
}; // class IRCodeStructureScope

class FunctionScope : public NamespaceScope {
public:
    FunctionScope(NamespaceScope **location, const cpl::FunctionDeclaration *ast, Function *fun);
    ~FunctionScope() override;
    
    DEF_PTR_GETTER(Function, fun);
    
    FunctionScope *NearlyFunctionScope() override;
    BranchScope *NearlyBranchScope() override;
    
private:
    const cpl::FunctionDeclaration *ast_;
    Function *fun_;
}; // class FunctionScope

struct Conflict {
    Value      *value;
    BasicBlock *path;
};

class BranchScope : public NamespaceScope {
public:
    BranchScope(NamespaceScope **location, cpl::Statement *ast, BranchScope *trunk = nullptr);
    ~BranchScope() override;
    
    const std::vector<BranchScope *> &branchs() const { return branchs_; }
    
    BranchScope *NearlyBranchScope() override;
    
    void PutSymbol(std::string_view name, const Symbol &symbol) override;
    
    NamespaceScope *Trunk() const override;
    
    bool IsTrunk() const;
    
    bool IsBranch() const { return !IsTrunk(); }
    
    bool NotInBranchs(const BranchScope *branch) const { return !InBranchs(branch); }
    
    bool InBranchs(const BranchScope *branch) const;
    
    BranchScope *Branch(cpl::Statement *ast) {
        auto br = new BranchScope(location_, ast, this);
        assert(InBranchs(br));
        return br;
    }
    
    
    using MergingHandler = void(std::string_view, // Name of value
                                std::vector<Conflict> && // Paths of values
                                );
    
    const std::vector<Conflict> &conflict(std::string_view name) const {
        auto iter = conflicts_.find(name);
        assert(iter != conflicts_.end());
        return iter->second;
    }
    
    int MergeConflicts(std::function<MergingHandler> &&callback);
private:
    void PutSymbolAndRecordConflict(std::string_view name, const Symbol &symbol);
    
    cpl::Statement *ast_;
    BranchScope *trunk_;
    std::vector<BranchScope *> branchs_;
    std::unordered_map<std::string_view, std::vector<Conflict>> conflicts_;
    std::unordered_map<std::string_view, NamespaceScope *> originals_;
}; // class BranchScope

inline bool NamespaceScope::IsFileUnitScope() { return NearlyFileUnitScope() == this; }

inline bool NamespaceScope::IsStructureScope() { return NearlyStructureScope() == this; }

} // namespace ir

} // namespace yalx

#endif // YALX_IR_SCOPE_H_
