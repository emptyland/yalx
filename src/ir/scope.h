#pragma once
#ifndef YALX_IR_SCOPE_H_
#define YALX_IR_SCOPE_H_

#include "compiler/ast.h"
#include "base/arena-utils.h"
#include "base/status.h"
#include "base/checking.h"
#include "base/base.h"

namespace yalx {

namespace ir {

class IRCodeEnvScope;
class IRCodePkgScope;
class IRCodeFileScope;
class IRCodeStructureScope;
class IRCodeFunScope;
class IRCodeBranchScope;

class Model;
class Value;
class Function;
class BasicBlock;

struct Symbol {
    enum Kind {
        kNotFound,
        kUnknown,
        kModel,
        kVal,
        kFun,
        kField,
    };
    Kind kind;
    cpl::Statement *node;
    union {
        Model *model;
        Value *value;
        Function *fun;
        ptrdiff_t field_offset; // offset of field
    } core;
    IRCodeEnvScope *owns;
};

class IRCodeEnvScope {
public:
    virtual ~IRCodeEnvScope() {};
    
    DEF_PTR_GETTER(IRCodeEnvScope, prev);
    DEF_VAL_GETTER(int, level);
    
    virtual IRCodePkgScope *NearlyPkgScope() { return !prev_ ? nullptr : prev_->NearlyPkgScope(); }
    virtual IRCodeFileScope *NearlyFileScope() { return !prev_ ? nullptr : prev_->NearlyFileScope(); }
    virtual IRCodeStructureScope *NearlyStructureScope() { return !prev_ ? nullptr : prev_->NearlyStructureScope(); }
    virtual IRCodeFunScope *NearlyFunScope() { return !prev_ ? nullptr : prev_->NearlyFunScope(); }
    virtual IRCodeBranchScope *NearlyBranchScope() { return !prev_ ? nullptr : prev_->NearlyBranchScope(); }
    
    virtual Symbol FindLocalSymbol(std::string_view name) = 0;
    virtual Symbol FindSymbol(std::string_view name);
    
    virtual int BranchId() const { return 0; }
    
    void Enter() {
        assert(location_ != nullptr);
        prev_ = *location_;
        assert(*location_ != this);
        *location_ = this;
        level_++;
    }
    
    void Exit() {
        assert(*location_ == this);
        *location_ = prev_;
        level_--;
    }
protected:
    IRCodeEnvScope(IRCodeEnvScope **location): location_(location) {}
    
    IRCodeEnvScope **location_;
    IRCodeEnvScope *prev_ = nullptr;
    int level_ = 0;
}; // class IRCodeEnvScope

class IRCodePkgScope : public IRCodeEnvScope {
public:
    IRCodePkgScope(IRCodeEnvScope **location): IRCodeEnvScope(location) { Enter(); }
    ~IRCodePkgScope() override { Exit(); }

    IRCodePkgScope *NearlyPkgScope() override { return this; }
private:
//    struct Slot {
//        cpl::FileUnit *file_unit;
//        cpl::Statement *node;
//        Model *model;
//        Value *value;
//        Function *fun;
//    };

    std::vector<IRCodeFileScope *> file_scopes_;
}; // class IRCodePkgScope

class IRCodeFileScope : public IRCodeEnvScope {
public:
    IRCodeFileScope *NearlyFileScope() override { return this; };
    
private:
    cpl::FileUnit *file_unit_;
}; // class IRCodeFileScope

class IRCodeStructureScope : public IRCodeEnvScope {
public:
    IRCodeStructureScope *NearlyStructureScope() override { return this; }
    IRCodeFunScope *NearlyFunScope() override { return nullptr; }
}; // class IRCodeStructureScope

class IRCodeFunScope : public IRCodeEnvScope {
public:
    IRCodeFunScope *NearlyFunScope() override { return this; }
    IRCodeBranchScope *NearlyBranchScope() override { return nullptr; }
}; // class IRCodeStructureScope

// TODO add branch scope
class IRCodeBranchScope : public IRCodeEnvScope {
public:
    IRCodeBranchScope(IRCodeEnvScope **location, cpl::Statement *ast, BasicBlock *block,
                      IRCodeBranchScope *trunk = nullptr);
    ~IRCodeBranchScope() override;
    
    IRCodeBranchScope *NearlyBranchScope() override { return this; }
    
    Symbol FindLocalSymbol(std::string_view name) override;
    
    bool IsTrunk() const {
        if (auto prev = prev_->NearlyBranchScope(); prev == prev_) {
            return prev->NotInBranchs(this);
        }
        return false;
    }
    
    bool IsBranch() const { return !IsTrunk(); }
    
    bool NotInBranchs(const IRCodeBranchScope *branch) const { return !InBranchs(branch); }

    bool InBranchs(const IRCodeBranchScope *branch) const {
        for (auto br : branchs_) {
            if (br == branch) {
                return br;
            }
        }
        return false;
    }
    
    IRCodeBranchScope *Branch(cpl::Statement *ast, BasicBlock *block) {
        auto br = new IRCodeBranchScope(location_, ast, block, this);
        assert(InBranchs(br));
        return br;
    }
    
    void Update(std::string_view name, IRCodeEnvScope *owns, Value *value);
    
    void Register(std::string_view name, Value *value) {
        assert(values_.find(name) == values_.end());
        values_[name] = value;
    }
private:
    cpl::Statement *ast_;
    BasicBlock *block_;
    int branch_id_;
    std::map<std::string_view, Value *> values_;
    std::vector<IRCodeBranchScope *> branchs_;
}; // class IRCodeBlockScope

} // namespace ir

} // namespace yalx

#endif // YALX_IR_SCOPE_H_
