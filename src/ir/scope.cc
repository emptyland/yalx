#include "ir/scope.h"
#include "ir/node.h"
#include <stack>
#include <set>

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
    if (symbol.owns != this) {
        const_cast<NamespaceScope *>(symbol.owns)->symbols_[name] = symbol;
    } else {
        symbols_[name] = symbol;
    }
}

PackageScope::PackageScope(NamespaceScope **location, cpl::Package *pkg,
                           base::ArenaMap<std::string_view, Symbol> *global)
: NamespaceScope(location)
, pkg_(pkg) {
    //Enter();
    for (auto file_unit : pkg_->source_files()) {
        auto scope = new FileUnitScope(location_, file_unit, global);
        files_ptrs_[file_unit] = files_.size();
        files_.push_back(scope);
    }
}

PackageScope::~PackageScope() {
    for (auto scope : files_) { delete scope; }
}

void PackageScope::Enter(NamespaceScope **location) {
    location_ = DCHECK_NOTNULL(location);
    for (auto file : files_) {
        static_cast<decltype(this)>(static_cast<NamespaceScope *>(file))->location_ = DCHECK_NOTNULL(location);
    }
    NamespaceScope::Enter();
}

PackageScope *PackageScope::NearlyPackageScope() { return this; }

FileUnitScope::FileUnitScope(NamespaceScope **location, cpl::FileUnit *file_unit,
                             base::ArenaMap<std::string_view, Symbol> *symobls)
: NamespaceScope(location)
, file_unit_(file_unit)
, proxy_symbols_(symobls) {
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

Symbol FileUnitScope::Lookup(std::string_view name) const {
    if (auto iter = proxy_symbols_->find(name); iter != proxy_symbols_->end()) {
        auto symbol = iter->second;
        symbol.owns = const_cast<FileUnitScope *>(this);
        return symbol;
    }
    
    return Symbol::NotFound();
}


FunctionScope::FunctionScope(NamespaceScope **location, const cpl::FunctionDeclaration *ast, Function *fun)
: NamespaceScope(location)
, ast_(ast)
, fun_(fun) { Enter(); }

FunctionScope::~FunctionScope() { Exit(); }

FunctionScope *FunctionScope::NearlyFunctionScope() { return this; }


StructureScope::StructureScope(NamespaceScope **location, const cpl::IncompletableDefinition *definition,
                               StructureModel *model)
: NamespaceScope(location)
, ast_(definition)
, model_(model) {
    Enter();
}

StructureScope::~StructureScope() { Exit(); }

void StructureScope::InstallAncestorsSymbols() {
    std::stack<StructureModel *> ancestors;
    for (auto base = model()->base_of(); base != nullptr; base = base->base_of()) {
        ancestors.push(base);
    }
    while (!ancestors.empty()) {
        auto ancestor = ancestors.top();
        ancestors.pop();
        
        for (auto [name, handle] : ancestor->member_handles()) {
            PutSymbol(name, Symbol::Had(this, handle));
        }
    }
    
}

StructureScope *StructureScope::NearlyStructureScope() { return this; }

FunctionScope *StructureScope::NearlyFunctionScope() { return nullptr; }

BranchScope *FunctionScope::NearlyBranchScope() { return nullptr; }

BranchScope::BranchScope(NamespaceScope **location, cpl::Statement *ast, BranchScope *trunk)
    : NamespaceScope(location)
    , ast_(ast)
    , trunk_(trunk) {
    if (trunk) {
        trunk->branchs_.push_back(this);
    } else {
        
    }
}
BranchScope::~BranchScope() {
    for (auto br : branchs_) {
        delete br;
    }
}

BranchScope *BranchScope::NearlyBranchScope() { return this; }

NamespaceScope *BranchScope::Trunk() const { return trunk_; }

int BranchScope::MergeConflicts(std::function<MergingHandler> &&callback) {
    
    std::set<std::string_view> keys;
    for (auto [name, _] : conflicts_) {
        keys.insert(name);
    }
    for (auto br : branchs()) {
        for (auto [name, _] : br->conflicts_) {
            keys.insert(name);
        }
    }
    
    std::vector<Conflict> paths;
    for (auto name : keys) {
        paths.clear();
        for (auto br : branchs()) {
            if (auto iter = br->conflicts_.find(name); iter != br->conflicts_.end()) {
                for (auto it : iter->second) {
                    //paths[it.path] = it.value;
                    auto iter = std::find_if(paths.begin(), paths.end(), [it](auto c) {return c.path == it.path;});
                    if (iter == paths.end()) {
                        paths.push_back(it);
                    } else {
                        *iter = it;
                    }
                }
            }
        }
        if (auto iter = conflicts_.find(name); iter != conflicts_.end()) {
            for (auto it : iter->second) {
                auto iter = std::find_if(paths.begin(), paths.end(), [it](auto c) {return c.path == it.path;});
                if (iter == paths.end()) {
                    paths.push_back(it);
                } else {
                    *iter = it;
                }
            }
        }
        
        callback(name, std::move(paths));
    }
    return static_cast<int>(keys.size());
}

void BranchScope::PutSymbol(std::string_view name, const Symbol &symbol) {
//    if (/*symbol.owns == this ||*/ IsTrunk()) { // Itself or trunk
//        NamespaceScope::PutSymbol(name, symbol);
//        return;
//    }
    
    BranchScope *br = nullptr;
    for (br = this; br && br->prev() != br->Trunk(); br = br->prev()->NearlyBranchScope()) {
        // Loop
    }
    if (!br) {
        NamespaceScope::PutSymbol(name, symbol);
    } else {
        br->PutSymbolAndRecordConflict(name, symbol);
    }
}

void BranchScope::PutSymbolAndRecordConflict(std::string_view name, const Symbol &symbol) {
    // Trunk v0
    //  +-- Branch-1 v1
    //  +-- Branch-2/Trunk v2
    //       +-- Branch-2/1/Trunk v4
    //            +-- Branch-2/1/1 v5
    //            +-- Branch-2/1/2 v6
    //       +-- Branch-2/2 v7
    auto const limit = DCHECK_NOTNULL(Trunk())->NearlyFunctionScope(); // Function is limit side
    assert(prev() == Trunk());
    for (auto ns = Trunk(); ns != nullptr; ns = ns->prev()) {
        auto exists_one = ns->FindLocalSymbol(name);
        if (exists_one.IsFound()) {
            auto & items = conflicts_[name];
            if (items.empty()) {
                items.push_back({exists_one.core.value, exists_one.block}); // origin path
                originals_[name] = ns;
            }
            auto iter = std::find_if(items.begin(), items.end(), [&symbol](auto item){
                return symbol.block == item.path;
            });
            if (iter == items.end()) {
                items.push_back({symbol.core.value, symbol.block});
            } else {
                iter->value = symbol.core.value;
            }
            break;
        }
        if (ns == limit) {
            break;
        }
    }
    symbols_[name] = symbol;
}

bool BranchScope::IsTrunk() const {
    if (trunk_) {
        return false;
    }
    if (!prev_ || !branchs().empty()) {
        return true;
    }
    auto that = this;
    auto it = that->prev()->NearlyBranchScope();
    while (it) {
        if (it->InBranchs(that)) {
            return false;
        }

        that = it;
        if (!that->prev()) {
            break;
        }
        it = that->prev()->NearlyBranchScope();
    }
    return true;
}

bool BranchScope::InBranchs(const BranchScope *branch) const {
    for (auto br : branchs_) {
        if (br == branch) {
            return br;
        }
    }
    return false;
}

} // namespace ir

} // namespace yalx
