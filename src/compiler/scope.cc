#include "compiler/scope.h"
#include "compiler/ast.h"

namespace yalx {

namespace cpl {

NamespaceScope::NamespaceScope(NamespaceScope **location)
    : location_(location) {
    //Enter();
}

NamespaceScope::~NamespaceScope() {
    //Exit();
}

PackageScope *NamespaceScope::NearlyPackageScope() {
    return !prev_ ? nullptr : prev_->NearlyPackageScope();
}

FileUnitScope *NamespaceScope::NearlyFileUnitScope() {
    return !prev_ ? nullptr : prev_->NearlyFileUnitScope();
}

StructureScope *NamespaceScope::NearlyStructureScope() {
    return !prev_ ? nullptr : prev_->NearlyStructureScope();
}

FunctionScope *NamespaceScope::NearlyFunctionScope() {
    return !prev_ ? nullptr : prev_->NearlyFunctionScope();
}


std::tuple<Statement *, NamespaceScope *> NamespaceScope::FindSymbol(std::string_view name) const {
    for (auto node = this; node != nullptr; node = node->prev_) {
        if (auto symbol = node->FindLocalSymbol(name); symbol != nullptr) {
            return std::make_tuple(symbol, const_cast<NamespaceScope *>(node));
        }
    }
    return std::make_tuple(nullptr, nullptr);
}

Statement *NamespaceScope::FindLocalSymbol(std::string_view name) const {
    if (auto iter = symbols_.find(name); iter == symbols_.end()) {
        return nullptr;
    } else {
        return iter->second;
    }
}

Statement *NamespaceScope::FindOrInsertSymbol(std::string_view name, Statement *ast) {
    if (auto iter = symbols_.find(name); iter != symbols_.end()) {
        return iter->second;
    } else {
        symbols_[name] = ast;
    }
    return nullptr;
}

PackageScope::PackageScope(NamespaceScope **location, Package *pkg)
    : NamespaceScope(location)
    , pkg_(DCHECK_NOTNULL(pkg)) {
    Enter();
    for (auto file_unit : pkg_->source_files()) {
        auto scope = new FileUnitScope(location, file_unit);
        files_.push_back(scope);
    }
}

PackageScope::~PackageScope() {
    for (auto scope : files_) { delete scope; }
    Exit();
}

PackageScope *PackageScope::NearlyPackageScope() {
    return this;
}

FileUnitScope *PackageScope::NearlyFileUnitScope() {
    return nullptr;
}

StructureScope *PackageScope::NearlyStructureScope() {
    return nullptr;
}

FunctionScope *PackageScope::NearlyFunctionScope() {
    return nullptr;
}

FileUnitScope::FileUnitScope(NamespaceScope **location, FileUnit *file_unit)
    : NamespaceScope(location)
    , file_unit_(file_unit) {
}

FileUnitScope::~FileUnitScope() {}


FileUnitScope *FileUnitScope::NearlyFileUnitScope() {
    return this;
}

StructureScope *FileUnitScope::NearlyStructureScope() {
    return nullptr;
}

FunctionScope *FileUnitScope::NearlyFunctionScope() {
    return nullptr;
}

Statement *FileUnitScope::FindOrInsertSymbol(std::string_view name, Statement *ast) {
    return NearlyPackageScope()->FindOrInsertSymbol(name, ast);
}

} // namespace cpl

} // namespace yalx
