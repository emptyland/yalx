#pragma once
#ifndef YALX_BACKEND_INSTRUCTION_SELECTOR_TEST_H_
#define YALX_BACKEND_INSTRUCTION_SELECTOR_TEST_H_


#include "backend/linkage-symbols.h"
#include "backend/constants-pool.h"
#include "backend/frame.h"
#include "ir/operators-factory.h"
#include "ir/operator.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/type.h"
#include <gtest/gtest.h>

namespace yalx::backend {

class InstructionSelectorTest : public ::testing::Test {
public:
    static constexpr ir::SourcePosition kUnknown = ir::SourcePosition::Unknown();
    
    InstructionSelectorTest();
    
    base::Arena *arena() { return &arena_; }
    Linkage *linkage() { return &linkage_; }
    ConstantsPool *const_pool() { return &const_pool_; }
    ir::OperatorsFactory *ops() { return &ops_; }
    DEF_PTR_GETTER(ir::Module, module);

    ir::Function *NewFun(const char *name, const char *sign, std::initializer_list<ir::Type> args,
                         std::initializer_list<ir::Type> rets);

    ir::StructureModel *NewFooClass();

protected:
    base::Arena arena_;
    Linkage linkage_;
    ConstantsPool const_pool_;
    ir::Module *module_;
    ir::OperatorsFactory ops_;
}; // class InstructionSelectorTest

} // namespace yalx


#endif // YALX_BACKEND_INSTRUCTION_SELECTOR_TEST_H_
