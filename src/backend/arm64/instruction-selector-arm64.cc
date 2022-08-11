#include "backend/instruction-selector.h"
#include "backend/arm64/instruction-codes-arm64.h"
#include "backend/registers-configuration.h"
#include "ir/condition.h"
#include "ir/metadata.h"
#include "ir/utils.h"
#include "ir/node.h"
#include "ir/type.h"

namespace yalx {

namespace backend {

class Arm64InstructionSelector final : public InstructionSelector {
public:
    Arm64InstructionSelector(base::Arena *arena, Linkage *linkage)
    : InstructionSelector(arena, RegistersConfiguration::of_arm64(), linkage) {}
    
    void VisitCondBr(ir::Value *instr) override {
        DCHECK(instr->op()->value_in() == 1);
        
        auto if_true = GetBlock(instr->OutputControl(0));
        auto if_false = GetBlock(instr->OutputControl(1));
        ReloactionOperand label(if_true);
        ReloactionOperand output(if_false);
        
        auto cond = instr->InputValue(0);
        if (cond->Is(ir::Operator::kICmp)) {
            auto code = ir::OperatorWith<ir::IConditionId>::Data(cond->op()).value;
            switch (code) {
                case ir::IConditionId::k_sle:
                    Emit(Arm64B_gt, output);
                    break;

                default:
                    UNREACHABLE();
                    break;
            }
            
        } else if (cond->Is(ir::Operator::kFCmp)) {
            UNREACHABLE();
        } else {
            UNREACHABLE();
        }
    }
    
    void VisitAddOrSub(ir::Value *instr) override {
        InstructionCode op = ArchNop;
        if (instr->Is(ir::Operator::kAdd)) {
            op = Arm64Add;
        } else {
            DCHECK(instr->Is(ir::Operator::kSub));
            op = Arm64Sub;
        }
        Emit(op, DefineAsRegister(instr), UseAsRegister(instr->InputValue(0)), UseAsRegister(instr->InputValue(1)));
    }
    
    void VisitICmp(ir::Value *instr) override {
        if (MatchCmpOnlyUsedByBr(instr)) {
            Emit(Arm64Cmp, NoOutput(), UseAsRegister(instr->InputValue(0)), UseAsRegister(instr->InputValue(1)));
            return;
        }
        
        UNREACHABLE(); // TODO:
    }
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Arm64InstructionSelector);
private:
    
}; // class Arm64InstructionSelector

InstructionFunction *Arm64SelectFunctionInstructions(base::Arena *arena, Linkage *linkage,
                                                     ir::Function *fun) {
    Arm64InstructionSelector selector(arena, linkage);
    return selector.VisitFunction(fun);
}

} // namespace backend

} // namespace yalx

