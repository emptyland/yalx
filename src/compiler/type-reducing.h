#pragma once
#ifndef YALX_COMPILER_TYPE_REDUCING_H_
#define YALX_COMPILER_TYPE_REDUCING_H_

#include "compiler/node.h"
#include "base/status.h"
#include "base/arena.h"
#include <unordered_map>
#include <string_view>

namespace yalx {

namespace cpl {

class SyntaxFeedback;

struct GlobalSymbol {
    String    *symbol;
    Statement *ast;
    Package   *owns;
    
    bool IsFound() { return !IsNotFound(); }
    bool IsNotFound() { return ast == nullptr; }
    
    static GlobalSymbol NotFound() { return {nullptr, nullptr, nullptr}; }
};

base::Status ReducePackageDependencesType(Package *entry,
                                          base::Arena *arena,
                                          SyntaxFeedback *error_feedback,
                                          std::unordered_map<std::string_view, GlobalSymbol> *symbols);

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_TYPE_REDUCING_H_
