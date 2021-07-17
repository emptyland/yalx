#pragma once
#ifndef YALX_IR_SCOPE_H_
#define YALX_IR_SCOPE_H_

#include "ir/metadata.h"
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
        kVar,
        kFun,
        kValField,
        kVarField,
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
    
    static Symbol NotFound() { return {.kind = kNotFound}; }
    static Symbol Val(IRCodeEnvScope *owns, Value *value, cpl::Statement *node = nullptr) {
        return {
            .kind = kVal,
            .node = node,
            .core = {
                .value = value
            },
            .owns = owns,
        };
    }
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
    
    virtual IRCodeEnvScope *Trunk() const { return nullptr; }
    
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
    
    IRCodeEnvScope *Trunk() const override { return trunk_; }
    
    bool IsTrunk() const;
    
    bool IsBranch() const { return !IsTrunk(); }
    
    bool NotInBranchs(const IRCodeBranchScope *branch) const { return !InBranchs(branch); }

    bool InBranchs(const IRCodeBranchScope *branch) const;
    
    IRCodeBranchScope *Branch(cpl::Statement *ast, BasicBlock *block) {
        auto br = new IRCodeBranchScope(location_, ast, block, this);
        assert(InBranchs(br));
        return br;
    }
    
    void Update(std::string_view name, IRCodeEnvScope *owns, Value *value);
    
    void Register(std::string_view name, Value *value, Model::Constraint constraint = Model::kVal) {
        assert(values_.find(name) == values_.end());
        values_[name] = Item::Make(value, constraint);
    }
    
    Value *FindValueOrNull(std::string_view name) const {
        if (auto iter = values_.find(name); iter == values_.end()) {
            return nullptr;
        } else {
            return iter->second.Value();
        }
    }
private:
    static constexpr uintptr_t kValueMask = ~1L;
    
    struct Item {
        uintptr_t core;
        
        static Item Make(Value *value, Model::Constraint constraint) {
            return {reinterpret_cast<uintptr_t>(value) | (constraint == Model::kVar ? 1u : 0) };
        }
        
        class Value *Value() const { return reinterpret_cast<class Value *>(core & kValueMask); }
        Model::Constraint Constraint() const { return (core & 1u) ? Model::kVar : Model::kVal; }
    };
    
    cpl::Statement *ast_;
    BasicBlock *block_;
    IRCodeBranchScope *trunk_;
    std::map<std::string_view, Item> values_;
    std::vector<IRCodeBranchScope *> branchs_;
}; // class IRCodeBlockScope

} // namespace ir

} // namespace yalx

#endif // YALX_IR_SCOPE_H_
