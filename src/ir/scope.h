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
};

class NamespaceScope {
public:
    virtual ~NamespaceScope() {};
    
    DEF_PTR_GETTER(NamespaceScope, prev);
    DEF_VAL_GETTER(int, level);
    
    virtual PackageScope *NearlyPackageScope() { return !prev_ ? nullptr : prev_->NearlyPackageScope(); }
    virtual FileUnitScope *NearlyFileUnitScope() { return !prev_ ? nullptr : prev_->NearlyFileUnitScope(); }
    virtual StructureScope *NearlyStructureScope() { return !prev_ ? nullptr : prev_->NearlyStructureScope(); }
    virtual FunctionScope *NearlyFunctionScope() { return !prev_ ? nullptr : prev_->NearlyFunctionScope(); }
    virtual BranchScope *NearlyBranchScope() { return !prev_ ? nullptr : prev_->NearlyBranchScope(); }
    
    virtual Symbol FindLocalSymbol(std::string_view name);
    virtual Symbol FindSymbol(std::string_view name);
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
    NamespaceScope(NamespaceScope **location): location_(location) {}
    
    NamespaceScope **location_;
    NamespaceScope *prev_ = nullptr;
    int level_ = 0;
    std::unordered_map<std::string_view, Symbol> symbols_;
}; // class IRCodeEnvScope

class PackageScope : public NamespaceScope {
public:
    PackageScope(NamespaceScope **location): NamespaceScope(location) { Enter(); }
    ~PackageScope() override { Exit(); }
    
    PackageScope *NearlyPackageScope() override { return this; }
private:
    //    struct Slot {
    //        cpl::FileUnit *file_unit;
    //        cpl::Statement *node;
    //        Model *model;
    //        Value *value;
    //        Function *fun;
    //    };
    
    std::vector<FileUnitScope *> file_scopes_;
}; // class IRCodePkgScope

class FileUnitScope : public NamespaceScope {
public:
    FileUnitScope *NearlyFileUnitScope() override { return this; };
    
private:
    cpl::FileUnit *file_unit_;
}; // class IRCodeFileScope

class StructureScope : public NamespaceScope {
public:
    StructureScope *NearlyStructureScope() override { return this; }
    FunctionScope *NearlyFunctionScope() override { return nullptr; }
}; // class IRCodeStructureScope

class FunctionScope : public NamespaceScope {
public:
    FunctionScope *NearlyFunctionScope() override { return this; }
    BranchScope *NearlyBranchScope() override { return nullptr; }
}; // class IRCodeStructureScope

// TODO add branch scope
class BranchScope : public NamespaceScope {
public:
    BranchScope(NamespaceScope **location, cpl::Statement *ast, BasicBlock *block, BranchScope *trunk = nullptr);
    ~BranchScope() override;
    
    BranchScope *NearlyBranchScope() override { return this; }
    
    //Symbol FindLocalSymbol(std::string_view name) override;
    
    NamespaceScope *Trunk() const override { return trunk_; }
    
    bool IsTrunk() const;
    
    bool IsBranch() const { return !IsTrunk(); }
    
    bool NotInBranchs(const BranchScope *branch) const { return !InBranchs(branch); }
    
    bool InBranchs(const BranchScope *branch) const;
    
    BranchScope *Branch(cpl::Statement *ast, BasicBlock *block) {
        auto br = new BranchScope(location_, ast, block, this);
        assert(InBranchs(br));
        return br;
    }
    
    void Update(std::string_view name, NamespaceScope *owns, Value *value);
private:
    cpl::Statement *ast_;
    BasicBlock *block_;
    BranchScope *trunk_;
    std::vector<BranchScope *> branchs_;
}; // class BranchScope

} // namespace ir

} // namespace yalx

#endif // YALX_IR_SCOPE_H_
