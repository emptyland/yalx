#include "compiler/scope.h"
#include "compiler/ast.h"
#include "base/arena-utils.h"

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

DataDefinitionScope *NamespaceScope::NearlyDataDefinitionScope() {
    return !prev_ ? nullptr : prev_->NearlyDataDefinitionScope();
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

DataDefinitionScope *PackageScope::NearlyDataDefinitionScope() {
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

DataDefinitionScope *FileUnitScope::NearlyDataDefinitionScope() {
    return nullptr;
}

FunctionScope *FileUnitScope::NearlyFunctionScope() {
    return nullptr;
}

Statement *FileUnitScope::FindOrInsertSymbol(std::string_view name, Statement *ast) {
    return NearlyPackageScope()->FindOrInsertSymbol(name, ast);
}


DataDefinitionScope::DataDefinitionScope(NamespaceScope **location, IncompletableDefinition *definition)
: NamespaceScope(location)
, definition_(DCHECK_NOTNULL(definition)) {
    Enter();
}

DataDefinitionScope::~DataDefinitionScope() {
    Exit();
}

//VariableDeclaration(base::Arena *arena, bool is_volatile, Constraint constraint,
//                    const String *identifier, class Type *type,
//                    const SourcePosition &source_position)
VariableDeclaration *DataDefinitionScope::ThisStub(base::Arena *arena) {
    if (this_stub()) {
        return this_stub();
    }
    auto id = base::ArenaString::New(arena, "this", 4);
    Type *type = nullptr;
    if (IsClassOrStruct()) {
        type = new (arena) ClassType(arena, AsClass(), definition()->source_position());
    } else {
        type = new (arena) StructType(arena, AsStruct(), definition()->source_position());
    }
    this_stub_ = new (arena) VariableDeclaration(arena, false, VariableDeclaration::kVal, id, type,
                                                 definition()->source_position());
    return this_stub();
}

bool DataDefinitionScope::IsClassOrStruct() const { return definition()->IsClassDefinition(); }

StructDefinition *DataDefinitionScope::AsStruct() const { return definition()->AsStructDefinition(); }

ClassDefinition *DataDefinitionScope::AsClass() const { return definition()->AsClassDefinition(); }

DataDefinitionScope *DataDefinitionScope::NearlyDataDefinitionScope() { return this; }

FunctionScope *DataDefinitionScope::NearlyFunctionScope() { return nullptr; }

FunctionScope::FunctionScope(NamespaceScope **location, FunctionDeclaration *fun)
: NamespaceScope(location)
, fun_(DCHECK_NOTNULL(fun)) {
    Enter();
}

FunctionScope::~FunctionScope() {
    Exit();
}

FunctionScope *FunctionScope::NearlyFunctionScope() { return this; }

} // namespace cpl

} // namespace yalx
