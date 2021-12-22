#pragma once
#ifndef YALX_IR_CODEGEN_H_
#define YALX_IR_CODEGEN_H_

#include "base/arena-utils.h"
#include "base/status.h"
#include "base/base.h"

namespace yalx {
namespace cpl {
class Package;
class SyntaxFeedback;
} // namespace cpl
namespace ir {

class Module;
class Model;
class Value;
class IRGeneratorAstVisitor;

class IntermediateRepresentationGenerator {
public:
    IntermediateRepresentationGenerator(base::Arena *arena, cpl::Package *entry, cpl::SyntaxFeedback *error_feedback);
    
    base::Status Run();
    
    friend class IRGeneratorAstVisitor;
    DISALLOW_IMPLICIT_CONSTRUCTORS(IntermediateRepresentationGenerator);
private:
    
    base::Status RecursiveGeneratePackage(cpl::Package *root);
    
    base::Arena *const arena_;
    cpl::Package *entry_;
    cpl::SyntaxFeedback *error_feedback_;
    base::ArenaMap<std::string_view, Model *> global_udts_;
    base::ArenaMap<std::string_view, Value *> global_vars_;
    base::ArenaMap<std::string_view, Module *> modules_;
    //base::ArenaVector<Module *> modules_;
}; // class IntermediateRepresentationGenerator


} // namespace ir

} // namespace yalx

#endif // YALX_IR_CODEGEN_H_
