#pragma once
#ifndef YALX_IR_PASS_CONSTANTS_FOLDING_H_
#define YALX_IR_PASS_CONSTANTS_FOLDING_H_

#include "ir/pass/pass.h"

namespace yalx {

namespace ir {

class ConstantsFoldingPass : public Pass<ConstantsFoldingPass> {
public:
    constexpr static const char kPassName[] = "constants-folding";
    constexpr static const int kPassLevel = 1;
    
    ConstantsFoldingPass(base::Arena *arena, OperatorsFactory *ops, ModulesMap *modules, cpl::SyntaxFeedback *feedback);
    
    void RunModule(Module *module);
    void RunFun(Function *fun);
    void RunUdt(StructureModel *udt) { ForeachMethod(udt); }
    void RunBasicBlock(BasicBlock *block);
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(ConstantsFoldingPass);
private:
    Value *FoldGlobalValue(Value *value);
    Value *FoldValueIfNeeded(Value *input, bool *folded);
}; // class ConstantsFoldingPass

} // namespace ir

} // namespace yalx

#endif // YALX_IR_PASS_CONSTANTS_FOLDING_H_
