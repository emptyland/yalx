#pragma once
#ifndef YALX_COMPILER_SCOPE_H_
#define YALX_COMPILER_SCOPE_H_

#include "compiler/node.h"

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
    virtual Statement *FindLocalSymbol(std::string_view name) const = 0;
    
    
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
}; // class NamespaceScope



} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_SCOPE_H_
