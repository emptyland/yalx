#include "ir/scope.h"
#include "ir/node.h"

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

PackageScope::PackageScope(NamespaceScope **location, BasicBlock *current_block, cpl::Package *pkg, GlobalSymbols global)
: NamespaceScope(location, nullptr/*current_block*/)
, pkg_(pkg) {
    //Enter();
    for (auto file_unit : pkg_->source_files()) {
        auto scope = new FileUnitScope(location_, current_block, file_unit, global);
        files_ptrs_[file_unit] = files_.size();
        files_.push_back(scope);
    }
}

PackageScope::~PackageScope() {
    for (auto scope : files_) { delete scope; }
    //Exit();
}

void PackageScope::Enter(NamespaceScope **location) {
    location_ = DCHECK_NOTNULL(location);
    for (auto file : files_) {
        static_cast<decltype(this)>(static_cast<NamespaceScope *>(file))->location_ = DCHECK_NOTNULL(location);
    }
    NamespaceScope::Enter();
}

PackageScope *PackageScope::NearlyPackageScope() { return this; }

FileUnitScope::FileUnitScope(NamespaceScope **location, BasicBlock *current_block, cpl::FileUnit *file_unit,
                             GlobalSymbols symobls)
: NamespaceScope(location, current_block)
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


FunctionScope::FunctionScope(NamespaceScope **location, const cpl::FunctionDeclaration *ast, Function *fun)
: NamespaceScope(location, fun->entry())
, ast_(ast)
, fun_(fun) { Enter(); }

FunctionScope::~FunctionScope() { Exit(); }

FunctionScope *FunctionScope::NearlyFunctionScope() { return this; }

BranchScope *FunctionScope::NearlyBranchScope() { return nullptr; }

BranchScope::BranchScope(NamespaceScope **location, BasicBlock *current_block, cpl::Statement *ast, BranchScope *trunk)
    : NamespaceScope(location, current_block)
    , ast_(ast)
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
