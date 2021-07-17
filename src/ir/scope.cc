#include "ir/scope.h"


namespace yalx {

namespace ir {

Symbol IRCodeEnvScope::FindSymbol(std::string_view name) {
    auto node = this;
    do {
        auto symbol = node->FindLocalSymbol(name);
        if (symbol.kind != Symbol::kNotFound) {
            return symbol;
        }
        node = node->prev();
    } while (node != nullptr);
    return Symbol::NotFound();
}

IRCodeBranchScope::IRCodeBranchScope(IRCodeEnvScope **location, cpl::Statement *ast, BasicBlock *block,
                                     IRCodeBranchScope *trunk)
    : IRCodeEnvScope(location)
    , ast_(ast)
    , block_(block)
    , trunk_(trunk) {
    if (trunk) {
        trunk->branchs_.push_back(this);
    } else {
        Enter();
    }
}
IRCodeBranchScope::~IRCodeBranchScope() {
    for (auto br : branchs_) {
        delete br;
    }
    if (IsTrunk()) {
        Exit();
    }
}

bool IRCodeBranchScope::IsTrunk() const {
    if (trunk_) {
        return false;
    }
    if (!prev_) {
        return true;
    }
    if (auto prev = prev_->NearlyBranchScope(); prev == prev_) {
        return prev->NotInBranchs(this);
    }
    return false;
}

bool IRCodeBranchScope::InBranchs(const IRCodeBranchScope *branch) const {
    for (auto br : branchs_) {
        if (br == branch) {
            return br;
        }
    }
    return false;
}

Symbol IRCodeBranchScope::FindLocalSymbol(std::string_view name) {
    auto iter = values_.find(name);
    if (iter == values_.end()) {
        return Symbol::NotFound();
    }
    return iter->second.Constraint() == Model::kVar
        ? Symbol::Var(this, iter->second.Value())
        : Symbol::Val(this, iter->second.Value());
}

void IRCodeBranchScope::Update(std::string_view name, IRCodeEnvScope *owns, Value *value) {
    if (owns == this) { // Itself
        assert(values_.find(name) != values_.end());
        assert(values_[name].Constraint() == Model::kVar);
        values_[name] = Item::Make(value, Model::kVar);
        return;
    }
    
    for (auto node = this; node->prev() != nullptr && node != owns; node = node->prev()->NearlyBranchScope()) {
        if (node->IsBranch()) {
            node->values_[name] = Item::Make(value, Model::kVar);
            return;
        }
    }

    if (auto br = owns->NearlyBranchScope(); br == owns) {
        assert(br->values_.find(name) != br->values_.end());
        assert(br->values_[name].Constraint() == Model::kVar);
        br->values_[name] = Item::Make(value, Model::kVar);
        return;
    }
    UNREACHABLE();
}

} // namespace ir

} // namespace yalx
