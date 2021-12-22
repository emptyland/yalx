#ifndef YALX_COMPILER_GLOBAL_SYMBOL_H_
#define YALX_COMPILER_GLOBAL_SYMBOL_H_

#include "compiler/node.h"

namespace yalx {

namespace cpl {

struct GlobalSymbol {
    String    *symbol;
    Statement *ast;
    Package   *owns;
    
    bool IsFound() { return !IsNotFound(); }
    bool IsNotFound() { return ast == nullptr; }
    
    static GlobalSymbol NotFound() { return {nullptr, nullptr, nullptr}; }
}; // struct GlobalSymbol

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_GLOBAL_SYMBOL_H_
