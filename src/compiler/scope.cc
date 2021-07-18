#include "compiler/scope.h"

namespace yalx {

namespace cpl {

NamespaceScope::NamespaceScope(NamespaceScope **location)
    : location_(location) {
    Enter();
}

NamespaceScope::~NamespaceScope() {
    Exit();
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


} // namespace cpl

} // namespace yalx
