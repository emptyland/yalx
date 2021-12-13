#pragma once
#ifndef YALX_COMPILER_SCOPE_H_
#define YALX_COMPILER_SCOPE_H_

#include "compiler/type-reducing.h"
#include "compiler/node.h"
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <string>

namespace yalx {

namespace cpl {

class NamespaceScope;
class PackageScope;
class FileUnitScope;
class DataDefinitionScope;
class FunctionScope;
class BlockScope;
class AstNode;

using GlobalSymbols = std::unordered_map<std::string_view, GlobalSymbol>;

enum BlockScopeKind {
    kPlainBlock,
    kLoopBlock,
    kBranchBlock,
};

class NamespaceScope {
public:
    template<class T> class Keeper {
    public:
        Keeper(T *ns): ns_(ns) { ns_->Enter(); }
        ~Keeper() { ns_->Exit(); }
        
        T *ns() const { return ns_; }
    private:
        T *ns_;
    };
    
    virtual ~NamespaceScope();
    
    virtual PackageScope *NearlyPackageScope();
    virtual FileUnitScope *NearlyFileUnitScope();
    virtual DataDefinitionScope *NearlyDataDefinitionScope();
    virtual FunctionScope *NearlyFunctionScope();
    virtual BlockScope *NearlyBlockScope();
    
    BlockScope *NearlyBlockScope(BlockScopeKind kind);

    virtual std::tuple<Statement *, NamespaceScope *> FindSymbol(std::string_view name) const;
    virtual Statement *FindLocalSymbol(std::string_view name) const;
    virtual Statement *FindOrInsertSymbol(std::string_view name, Statement *ast);
    
    void InsertSymbol(std::string_view name, Statement *ast) { symbols_[name] = ast; }
    
    void Enter() {
        assert(*location_ != this);
        prev_ = *location_;
        *location_ = this;
    }
    void Exit() {
        assert(*location_ == this);
        assert(*location_ != prev_);
        *location_ = prev_;
    }
protected:
    NamespaceScope(NamespaceScope **location);
    
    NamespaceScope **location_;
    NamespaceScope *prev_;
    std::map<std::string_view, Statement *> symbols_;
}; // class NamespaceScope

 
class PackageScope : public NamespaceScope {
public:
    PackageScope(NamespaceScope **location, Package *pkg, GlobalSymbols *symbols);
    ~PackageScope() override;
    
    DEF_PTR_GETTER(Package, pkg);
    const std::vector<FileUnitScope *> &files() { return files_; }
    
    bool Track(AstNode *ast) {
        if (auto iter = track_.find(ast); iter != track_.end()) {
            return true;
        }
        track_.insert(ast);
        return false;
    }
    
    bool HasTracked(AstNode *ast) { return track_.find(ast) != track_.end(); }
    bool HasNotTracked(AstNode *ast) { return !HasTracked(ast); }
    
    FileUnitScope *FindFileUnitScopeOrNull(Node *key) {
        auto iter = files_ptrs_.find(key);
        return iter == files_ptrs_.end() ? nullptr : files_[iter->second];
    }

    PackageScope *NearlyPackageScope() override;
    FileUnitScope *NearlyFileUnitScope() override;
    DataDefinitionScope *NearlyDataDefinitionScope() override;
    FunctionScope *NearlyFunctionScope() override;
    BlockScope *NearlyBlockScope() override;
    
private:
    Package *pkg_;
    std::unordered_map<Node *, size_t> files_ptrs_;
    std::vector<FileUnitScope *> files_;
    std::set<AstNode *> track_;
}; // class PackageScope


class FileUnitScope : public NamespaceScope {
public:
    FileUnitScope(NamespaceScope **location, FileUnit *file_unit, GlobalSymbols *symobls);
    ~FileUnitScope() override;
    
    DEF_PTR_GETTER(FileUnit, file_unit);
    
    FileUnitScope *NearlyFileUnitScope() override;
    DataDefinitionScope *NearlyDataDefinitionScope() override;
    FunctionScope *NearlyFunctionScope() override;
    
    Statement *FindLocalSymbol(std::string_view name) const override;
    Statement *FindOrInsertSymbol(std::string_view name, Statement *ast) override;
    
    Statement *FindExportSymbol(std::string_view prefix, std::string_view name) const;
    Statement *FindOrInsertExportSymbol(std::string_view prefix, std::string_view name, Statement *ast);
    
    //std::tuple<Statement *, NamespaceScope *> FindSymbol(std::string_view name) const override;
private:
    FileUnit *file_unit_;
    std::map<std::string_view, std::string> alias_;
    std::vector<std::string> implicit_alias_;
    GlobalSymbols *symobls_;
}; // class FileUnitScope

class DataDefinitionScope : public NamespaceScope {
public:
    DataDefinitionScope(NamespaceScope **location, IncompletableDefinition *definition);
    ~DataDefinitionScope() override;
    
    void InstallAncestorsSymbols();
    void InstallConcepts();
    
    DEF_PTR_GETTER(IncompletableDefinition, definition);
    DEF_PTR_PROP_RW(VariableDeclaration, this_stub);
    
    VariableDeclaration *ThisStub(base::Arena *arena);

    bool IsClassOrStruct() const;
    StructDefinition *AsStruct() const;
    ClassDefinition *AsClass() const;
    
    enum ImplementTarget {
        kNotFound,
        kInterface,
        kBaseClass,
    };

    ImplementTarget ImplementMethodOnce(std::string_view name, String *signature);
    
    int UnimplementMethods(std::function<void(InterfaceDefinition *, FunctionDeclaration *)> &&callback);
    
    DataDefinitionScope *NearlyDataDefinitionScope() override;
    FunctionScope *NearlyFunctionScope() override;
    Statement *FindLocalSymbol(std::string_view name) const override;
private:
    struct Concept {
        std::string_view signature;
        InterfaceDefinition *itf;
        FunctionDeclaration *method;
        int impl_count;
    };
    
    IncompletableDefinition *definition_;
    VariableDeclaration *this_stub_ = nullptr;
    std::map<std::string_view, Statement *> base_of_symbols_;
    std::map<std::string_view, std::vector<Concept>> concepts_symbols_;
}; // class DataDefinitionScope

class FunctionScope : public NamespaceScope {
public:
    FunctionScope(NamespaceScope **location, FunctionPrototype *prototype, bool fun_is_reducing);
    ~FunctionScope() override;
    
    DEF_PTR_GETTER(FunctionPrototype, prototype);
    DEF_VAL_GETTER(bool, fun_is_reducing);
    
    FunctionScope *NearlyFunctionScope() override;
    
private:
    FunctionPrototype *prototype_;
    bool fun_is_reducing_;
}; // class FunctionScope


class BlockScope : public NamespaceScope {
public:
    using Kind = BlockScopeKind;
    
    BlockScope(NamespaceScope **location, Kind kind, Statement *owns);
    ~BlockScope() override;
    
    DEF_PTR_GETTER(Statement, owns);
    DEF_VAL_GETTER(Kind, kind);
    
    FunctionScope *NearlyFunctionScope() override;
    BlockScope *NearlyBlockScope() override;
private:
    Kind kind_;
    Statement *owns_;
}; // class BlockScope

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_SCOPE_H_
