#include "ir/scope.h"


namespace yalx {

namespace ir {

Symbol NamespaceScope::FindLocalSymbol(std::string_view name) {
    auto iter = symbols_.find(name);
    return iter == symbols_.end() ? Symbol::NotFound() : iter->second;
}

Symbol NamespaceScope::FindSymbol(std::string_view name) {
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

void NamespaceScope::PutSymbol(std::string_view name, const Symbol &symbol) {
    auto iter = symbols_.find(name);
    assert(iter == symbols_.end());
    symbols_[name] = symbol;
}

BranchScope::BranchScope(NamespaceScope **location, cpl::Statement *ast, BasicBlock *block, BranchScope *trunk)
    : NamespaceScope(location)
    , ast_(ast)
    , block_(block)
    , trunk_(trunk) {
    if (trunk) {
        trunk->branchs_.push_back(this);
    } else {
        Enter();
    }
}
BranchScope::~BranchScope() {
    for (auto br : branchs_) {
        delete br;
    }
    if (IsTrunk()) {
        Exit();
    }
}

bool BranchScope::IsTrunk() const {
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

bool BranchScope::InBranchs(const BranchScope *branch) const {
    for (auto br : branchs_) {
        if (br == branch) {
            return br;
        }
    }
    return false;
}

void BranchScope::Update(std::string_view name, NamespaceScope *owns, Value *value) {
    if (owns == this) { // Itself
        assert(symbols_.find(name) != symbols_.end());
        assert(symbols_[name].kind == Symbol::kValue);
        symbols_[name] = Symbol::Val(this, value);
        return;
    }
    
    for (auto node = this; node->prev() != nullptr && node != owns; node = node->prev()->NearlyBranchScope()) {
        if (node->IsBranch()) {
            node->symbols_[name] = Symbol::Val(node, value);
            return;
        }
    }

    if (auto br = owns->NearlyBranchScope(); br == owns) {
        assert(br->symbols_.find(name) != br->symbols_.end());
        assert(br->symbols_[name].kind == Symbol::kValue);
        br->symbols_[name] = Symbol::Val(br, value);
        return;
    }
    UNREACHABLE();
}

} // namespace ir

} // namespace yalx
