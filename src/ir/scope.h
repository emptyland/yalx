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
        kHandle
    };
    Kind kind;
    cpl::Statement *node;
    union {
        Model *model;
        Value *value;
        Function *fun;
        ptrdiff_t field_offset; // offset of field
    } core;
    NamespaceScope *owns;
    
    bool IsFound() const { return !IsNotFound(); }
    bool IsNotFound() const { return kind == kNotFound; }
    
    static Symbol NotFound() { return {.kind = kNotFound}; }
    
    static Symbol Val(NamespaceScope *owns, Value *value, cpl::Statement *node = nullptr) {
        return {
            .kind = kValue,
            .node = node,
            .core = {
                .value = value
            },
            .owns = owns,
        };
    }
    
    static Symbol Udt(NamespaceScope *owns, Model *model, cpl::Statement *node = nullptr) {
        return {
            .kind = kModel,
            .node = node,
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
            .core = {
                .fun = fun
            },
            .owns = owns,
        };
    }
};

struct GlobalSymbols {
    const base::ArenaMap<std::string_view, Model *> *udts;
    const base::ArenaMap<std::string_view, Value *> *vars;
    const base::ArenaMap<std::string_view, Function *> *funs;
};

class NamespaceScope {
public:
    template<class T> class Keeper {
    public:
        Keeper(T *ns): ns_(ns) { ns_->Enter(); }
        ~Keeper() { ns_->Exit(); }
        
        T *ns() const { return ns_; }
    private:
        T *ns_;
    };

    virtual ~NamespaceScope() {};
    
    DEF_PTR_GETTER(NamespaceScope, prev);
    DEF_VAL_GETTER(int, level);

    BasicBlock *current_block() const { return DCHECK_NOTNULL(current_block_); }
    
    virtual PackageScope *NearlyPackageScope() { return !prev_ ? nullptr : prev_->NearlyPackageScope(); }
    virtual FileUnitScope *NearlyFileUnitScope() { return !prev_ ? nullptr : prev_->NearlyFileUnitScope(); }
    virtual StructureScope *NearlyStructureScope() { return !prev_ ? nullptr : prev_->NearlyStructureScope(); }
    virtual FunctionScope *NearlyFunctionScope() { return !prev_ ? nullptr : prev_->NearlyFunctionScope(); }
    virtual BranchScope *NearlyBranchScope() { return !prev_ ? nullptr : prev_->NearlyBranchScope(); }
    
    virtual Symbol FindLocalSymbol(std::string_view name) const;
    virtual Symbol FindSymbol(std::string_view name) const;
    void PutSymbol(std::string_view name, Value *value, cpl::Statement *node = nullptr) {
        PutSymbol(name, Symbol::Val(this, value, node));
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
    NamespaceScope(NamespaceScope **location, BasicBlock *current_block)
        : location_(location)
        , current_block_(current_block) {}
    
    NamespaceScope **location_;
    NamespaceScope *prev_ = nullptr;
    int level_ = 0;
    BasicBlock *current_block_;
    std::unordered_map<std::string_view, Symbol> symbols_;
}; // class IRCodeEnvScope

class PackageScope : public NamespaceScope {
public:
    PackageScope(NamespaceScope **location, BasicBlock *current_block, cpl::Package *pkg, GlobalSymbols global);
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
    FileUnitScope(NamespaceScope **location, BasicBlock *current_block, cpl::FileUnit *file_unit, GlobalSymbols symobls);
    ~FileUnitScope() override;
    
    DEF_PTR_GETTER(cpl::FileUnit, file_unit);
    
    FileUnitScope *NearlyFileUnitScope() override;
    
    Symbol FindLocalSymbol(std::string_view name) const override;

    Symbol FindExportSymbol(std::string_view prefix, std::string_view name) const;
    
private:
    Symbol Lookup(std::string_view name) const {
        if (auto iter = global_udts_->find(name); iter != global_udts_->end()) {
            return Symbol::Udt(const_cast<FileUnitScope *>(this), iter->second);
        }
        if (auto iter = global_funs_->find(name); iter != global_funs_->end()) {
            return Symbol::Fun(const_cast<FileUnitScope *>(this), iter->second);
        }
        if (auto iter = global_vars_->find(name); iter != global_vars_->end()) {
            return Symbol::Val(const_cast<FileUnitScope *>(this), iter->second);
        }
        return Symbol::NotFound();
    }
    
    cpl::FileUnit *file_unit_;
    std::map<std::string_view, std::string> alias_;
    std::vector<std::string> implicit_alias_;
    const base::ArenaMap<std::string_view, Model *> *global_udts_;
    const base::ArenaMap<std::string_view, Value *> *global_vars_;
    const base::ArenaMap<std::string_view, Function *> *global_funs_;
}; // class IRCodeFileScope

class StructureScope : public NamespaceScope {
public:
    StructureScope *NearlyStructureScope() override { return this; }
    FunctionScope *NearlyFunctionScope() override { return nullptr; }
}; // class IRCodeStructureScope

class FunctionScope : public NamespaceScope {
public:
    FunctionScope(NamespaceScope **location, const cpl::FunctionDeclaration *ast, Function *fun);
    ~FunctionScope() override;
    
    FunctionScope *NearlyFunctionScope() override;
    BranchScope *NearlyBranchScope() override;
    
private:
    const cpl::FunctionDeclaration *ast_;
    Function *fun_;
}; // class FunctionScope

// TODO add branch scope
class BranchScope : public NamespaceScope {
public:
    BranchScope(NamespaceScope **location, BasicBlock *current_block, cpl::Statement *ast,
                BranchScope *trunk = nullptr);
    ~BranchScope() override;
    
    BranchScope *NearlyBranchScope() override { return this; }
    
    //Symbol FindLocalSymbol(std::string_view name) override;
    
    NamespaceScope *Trunk() const override { return trunk_; }
    
    bool IsTrunk() const;
    
    bool IsBranch() const { return !IsTrunk(); }
    
    bool NotInBranchs(const BranchScope *branch) const { return !InBranchs(branch); }
    
    bool InBranchs(const BranchScope *branch) const;
    
    BranchScope *Branch(cpl::Statement *ast, BasicBlock *block) {
        auto br = new BranchScope(location_, block, ast, this);
        assert(InBranchs(br));
        return br;
    }
    
    void Update(std::string_view name, NamespaceScope *owns, Value *value);
private:
    cpl::Statement *ast_;
    BranchScope *trunk_;
    std::vector<BranchScope *> branchs_;
}; // class BranchScope

} // namespace ir

} // namespace yalx

#endif // YALX_IR_SCOPE_H_
