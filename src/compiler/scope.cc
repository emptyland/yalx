#include "compiler/scope.h"
#include "compiler/ast.h"
#include "base/arena-utils.h"
#include <stack>

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

PackageScope::PackageScope(NamespaceScope **location, Package *pkg, GlobalSymbols *symbols)
: NamespaceScope(location)
, pkg_(DCHECK_NOTNULL(pkg)) {
    //Enter();
    for (auto file_unit : pkg_->source_files()) {
        auto scope = new FileUnitScope(location_, file_unit, symbols);
        files_ptrs_[file_unit] = files_.size();
        files_.push_back(scope);
    }
}

PackageScope::~PackageScope() {
    for (auto scope : files_) { delete scope; }
    //Exit();
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

FileUnitScope::FileUnitScope(NamespaceScope **location, FileUnit *file_unit, GlobalSymbols *symobls)
: NamespaceScope(location)
, file_unit_(file_unit)
, symobls_(DCHECK_NOTNULL(symobls)) {
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


FileUnitScope *FileUnitScope::NearlyFileUnitScope() {
    return this;
}

DataDefinitionScope *FileUnitScope::NearlyDataDefinitionScope() {
    return nullptr;
}

FunctionScope *FileUnitScope::NearlyFunctionScope() {
    return nullptr;
}

Statement *FileUnitScope::FindLocalSymbol(std::string_view name) const {
    auto owns = DCHECK_NOTNULL(const_cast<FileUnitScope *>(this)->NearlyPackageScope());
    std::string full_name(owns->pkg()->path()->ToString());
    full_name.append(":")
        .append(file_unit()->package_name()->ToString())
        .append(".")
        .append(name);
    auto iter = symobls_->find(full_name);
    if (iter != symobls_->cend()) {
        return iter->second.ast;
    }
//    for (auto ias : implicit_alias_) {
//        full_name = ias.append(":")
//        
//    }
    return nullptr;
}

Statement *FileUnitScope::FindOrInsertSymbol(std::string_view name, Statement *ast) {
    auto owns = DCHECK_NOTNULL(const_cast<FileUnitScope *>(this)->NearlyPackageScope());
    std::string full_name(owns->pkg()->path()->ToString());
    full_name.append(":")
        .append(file_unit()->package_name()->ToString())
        .append(".")
        .append(name);
    auto iter = symobls_->find(full_name);
    if (iter == symobls_->cend()) {
        GlobalSymbol symbol{
            .symbol = String::New(file_unit()->arena(), full_name.data(), full_name.length()),
            .ast = ast,
            .owns = owns->pkg(),
        };
        file_unit()->Add(ast);
        (*symobls_)[symbol.symbol->ToSlice()] = symbol;
        return ast;
    }
    return iter->second.ast;
}

Statement *FileUnitScope::FindExportSymbol(std::string_view prefix, std::string_view name) const {
    auto alias_iter = alias_.find(prefix);
    if (alias_iter == alias_.cend()) {
        return nullptr;
    }
    std::string full_name(alias_iter->second);
    full_name.append(".").append(name);
    auto iter = symobls_->find(full_name);
    if (iter == symobls_->cend()) {
        return nullptr;
    }
    return iter->second.ast;
}

Statement *FileUnitScope::FindOrInsertExportSymbol(std::string_view prefix, std::string_view name, Statement *ast) {
    auto alias_iter = alias_.find(prefix);
    std::string full_name;
    if (file_unit()->package_name()->Equal(prefix.data())) {
        auto owns = DCHECK_NOTNULL(const_cast<FileUnitScope *>(this)->NearlyPackageScope());
        full_name = owns->pkg()->path()->ToString();
        full_name.append(":")
            .append(file_unit()->package_name()->ToString());
    } else {
        if (alias_iter == alias_.cend()) {
            return nullptr;
        }
        full_name.assign(alias_iter->second);
    }
    
    full_name.append(".").append(name);
    auto iter = symobls_->find(full_name);
    if (iter == symobls_->cend()) {
        GlobalSymbol symbol{
            .symbol = String::New(file_unit()->arena(), full_name.data(), full_name.length()),
            .ast = ast,
            .owns = ast->Pack(false/*force*/),
        };
        file_unit()->Add(ast);
        (*symobls_)[symbol.symbol->ToSlice()] = symbol;
        return ast;
    }
    return iter->second.ast;
}


DataDefinitionScope::DataDefinitionScope(NamespaceScope **location, IncompletableDefinition *definition)
: NamespaceScope(location)
, definition_(DCHECK_NOTNULL(definition)) {
    Enter();
}

DataDefinitionScope::~DataDefinitionScope() {
    Exit();
}

void DataDefinitionScope::InstallAncestorsSymbols() {
    std::stack<IncompletableDefinition *> ancestors;
    if (definition()->IsClassDefinition()) {
        auto def = definition()->AsClassDefinition();
        for (auto base = def->base_of(); base != nullptr; base = base->base_of()) {
            ancestors.push(base);
        }
        //def->concepts()
    } else {
        assert(definition()->IsStructDefinition());
        auto def = definition()->AsStructDefinition();
        for (auto base = def->base_of(); base != nullptr; base = base->base_of()) {
            ancestors.push(base);
        }
    }
    
    while (!ancestors.empty()) {
        auto def = ancestors.top();
        ancestors.pop();
        
        for (auto field : def->fields()) {
            base_of_symbols_[field.declaration->Identifier()->ToSlice()] = field.declaration;
        }
        for (auto method : def->methods()) {
            base_of_symbols_[method->name()->ToSlice()] = method;
        }
    }
}

void DataDefinitionScope::InstallConcepts() {
    assert(definition()->IsClassDefinition());
    auto def = definition()->AsClassDefinition();
    for (auto concept : def->concepts()) {
        auto ift = DCHECK_NOTNULL(concept->AsInterfaceType())->definition();
        for (auto method : ift->methods()) {
            Concept concept {
                method->prototype()->signature()->ToSlice(),
                ift,
                method,
                0
            };
            auto iter = concepts_symbols_.find(method->name()->ToSlice());
            if (iter == concepts_symbols_.end()) {
                concepts_symbols_[method->name()->ToSlice()] = {concept};
            } else {
                iter->second.push_back(concept);
            }
        }
    }
}

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

DataDefinitionScope::ImplementTarget
DataDefinitionScope::ImplementMethodOnce(std::string_view name, String *signature) {
    auto concepts_iter = concepts_symbols_.find(name);
    if (concepts_iter != concepts_symbols_.end()) {
        auto target = kNotFound;
        for (auto &concept : concepts_iter->second) {
            if (concept.method->prototype()->signature() == signature) {
                concept.impl_count++;
                target = kInterface;
            }
        }
        if (target != kNotFound) {
            return target;
        }
    }
    
    auto symbol_iter = base_of_symbols_.find(name);
    if (symbol_iter != base_of_symbols_.end()) {
        if (auto fun = symbol_iter->second->AsFunctionDeclaration()) {
            if (fun->prototype()->signature() == signature) {
                return kBaseClass;
            }
        }
    }
    
    return kNotFound;
}

DataDefinitionScope *DataDefinitionScope::NearlyDataDefinitionScope() { return this; }

FunctionScope *DataDefinitionScope::NearlyFunctionScope() { return nullptr; }

Statement *DataDefinitionScope::FindLocalSymbol(std::string_view name) const {
    if (auto ast = NamespaceScope::FindLocalSymbol(name)) {
        return ast;
    }
    auto iter = base_of_symbols_.find(name);
    return iter == base_of_symbols_.end() ? nullptr : iter->second;
}

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
