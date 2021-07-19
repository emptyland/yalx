#pragma once
#ifndef YALX_COMPILER_SCOPE_H_
#define YALX_COMPILER_SCOPE_H_

#include "compiler/node.h"
#include <map>
#include <vector>
#include <string>

namespace yalx {

namespace cpl {

class NamespaceScope;
class PackageScope;
class FileUnitScope;
class StructureScope;
class FunctionScope;
class BlockScope;

class NamespaceScope {
public:
    virtual ~NamespaceScope();
    
    virtual PackageScope *NearlyPackageScope();
    virtual FileUnitScope *NearlyFileUnitScope();
    virtual StructureScope *NearlyStructureScope();
    virtual FunctionScope *NearlyFunctionScope();
    

    virtual std::tuple<Statement *, NamespaceScope *> FindSymbol(std::string_view name) const;
    virtual Statement *FindLocalSymbol(std::string_view name) const;
    virtual Statement *FindOrInsertSymbol(std::string_view name, Statement *ast);
    
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
    PackageScope(NamespaceScope **location, Package *pkg);
    ~PackageScope() override;
    
    DEF_PTR_GETTER(Package, pkg);
    const std::vector<FileUnitScope *> &files() { return files_; }

    PackageScope *NearlyPackageScope() override;
    FileUnitScope *NearlyFileUnitScope() override;
    StructureScope *NearlyStructureScope() override;
    FunctionScope *NearlyFunctionScope() override;
    
private:
    Package *pkg_;
    std::vector<FileUnitScope *> files_;
}; // class PackageScope


class FileUnitScope : public NamespaceScope {
public:
    FileUnitScope(NamespaceScope **location, FileUnit *file_unit);
    ~FileUnitScope() override;
    
    DEF_PTR_GETTER(FileUnit, file_unit);
    
    FileUnitScope *NearlyFileUnitScope() override;
    StructureScope *NearlyStructureScope() override;
    FunctionScope *NearlyFunctionScope() override;
    Statement *FindOrInsertSymbol(std::string_view name, Statement *ast) override;
    
    //std::tuple<Statement *, NamespaceScope *> FindSymbol(std::string_view name) const override;
private:
    FileUnit *file_unit_;
    std::map<std::string_view, std::string_view> alias_;
}; // class FileUnitScope

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_SCOPE_H_
