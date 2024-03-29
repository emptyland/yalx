#pragma once
#ifndef YALX_COMPILER_GLOBAL_SYMBOL_H_
#define YALX_COMPILER_GLOBAL_SYMBOL_H_

#include "compiler/node.h"

namespace yalx::cpl {

struct GlobalSymbol {
    String *symbol;
    Statement *ast;
    Package *owns;

    [[nodiscard]] bool IsFound() const { return !IsNotFound(); }

    [[nodiscard]] bool IsNotFound() const { return ast == nullptr; }

    static GlobalSymbol NotFound() { return {nullptr, nullptr, nullptr}; }
}; // struct GlobalSymbol

} // namespace yalx

#endif // YALX_COMPILER_GLOBAL_SYMBOL_H_
