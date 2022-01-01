#include "ir/scope.h"


namespace yalx {

namespace ir {

Symbol NamespaceScope::FindLocalSymbol(std::string_view name) const {
    auto iter = symbols_.find(name);
    return iter == symbols_.end() ? Symbol::NotFound() : iter->second;
}

Symbol NamespaceScope::FindSymbol(std::string_view name) const {
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

PackageScope::PackageScope(NamespaceScope **location, cpl::Package *pkg, GlobalSymbols global)
: NamespaceScope(location)
, pkg_(pkg) {
    Enter();
    for (auto file_unit : pkg_->source_files()) {
        auto scope = new FileUnitScope(location_, file_unit, global);
        files_ptrs_[file_unit] = files_.size();
        files_.push_back(scope);
    }
}

PackageScope::~PackageScope() {
    for (auto scope : files_) { delete scope; }
    Exit();
}

PackageScope *PackageScope::NearlyPackageScope() { return this; }

FileUnitScope::FileUnitScope(NamespaceScope **location, cpl::FileUnit *file_unit, GlobalSymbols symobls)
: NamespaceScope(location)
, file_unit_(file_unit)
, global_udts_(symobls.udts)
, global_vars_(symobls.vars)
, global_funs_(symobls.funs) {
    for (auto import : file_unit->imports()) {
        std::string full_name(import->package_path()->ToString());
        full_name.append(":")
            .append(import->original_package_name()->ToString());
        if (import->alias()) {
            if (import->alias()->Equal("*")) {
                implicit_alias_.push_back(full_name);
            } else {
                alias_[import->alias()->ToSlice()] = full_name;
            }
        } else {
            alias_[import->original_package_name()->ToSlice()] = full_name;
        }
    }
}

FileUnitScope::~FileUnitScope() {}

FileUnitScope *FileUnitScope::NearlyFileUnitScope() { return this; }

Symbol FileUnitScope::FindLocalSymbol(std::string_view name) const {
    auto owns = DCHECK_NOTNULL(const_cast<FileUnitScope *>(this)->NearlyPackageScope());
    std::string full_name(owns->pkg()->path()->ToString());
    full_name.append(":")
        .append(file_unit()->package_name()->ToString())
        .append(".")
        .append(name);
    auto symbol = Lookup(full_name);
    if (symbol.IsFound()) {
        return symbol;
    }
    for (auto ias : implicit_alias_) {
        full_name = ias.append(".").append(name);
        symbol = Lookup(full_name);
        if (symbol.IsFound()) {
            return symbol;
        }
    }
    return Symbol::NotFound();
}

Symbol FileUnitScope::FindExportSymbol(std::string_view prefix, std::string_view name) const {
    auto alias_iter = alias_.find(prefix);
    if (alias_iter == alias_.cend()) {
        return Symbol::NotFound();
    }
    std::string full_name(alias_iter->second);
    full_name.append(".").append(name);
    auto symbol = Lookup(full_name);
    if (symbol.IsFound()) {
        return symbol;
    }
    return Symbol::NotFound();
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
