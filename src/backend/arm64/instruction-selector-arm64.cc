#include "backend/instruction-selector.h"
#include "backend/registers-configuration.h"
#include "ir/condition.h"
#include "ir/metadata.h"
#include "ir/utils.h"
#include "ir/node.h"

namespace yalx {

namespace backend {

class Arm64InstructionSelector final : public InstructionSelector {
public:
    Arm64InstructionSelector(base::Arena *arena, Linkage *linkage, ConstantsPool *const_pool)
    : InstructionSelector(arena, RegistersConfiguration::of_arm64(), linkage, const_pool) {}
    
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
        VisitArithOperands(op, DefineAsRegister(instr), instr->InputValue(0), instr->InputValue(1));
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
    
    void VisitArithOperands(InstructionCode op, InstructionOperand output, ir::Value *input0, ir::Value *input1) {
        std::vector<std::tuple<InstructionOperand, InstructionOperand>> moves;

        InstructionOperand lhs;
        if (IsAnyConstant(input0)) {
            if (auto imm = TryUseAsIntegralImmediate(input0, 16); imm.IsInvalid()) {
                // TODO: Load constants
                UNREACHABLE();
            } else {
                lhs = UseAsRegister(input0);
                moves.push_back({lhs, imm});
            }
        } else {
            lhs = UseAsRegister(input0);
        }
        InstructionOperand rhs;
        if (IsAnyConstant(input1)) {
            if (auto imm = TryUseAsIntegralImmediate(input1, 12); !imm.IsInvalid()) {
                rhs = imm;
            } else if (auto imm = TryUseAsIntegralImmediate(input1, 16); !imm.IsInvalid()) {
                rhs = UseAsRegister(input1);
                moves.push_back({rhs, imm});
            } else {
                // TODO: Load constants
                UNREACHABLE();
            }
        } else {
            rhs = UseAsRegister(input1);
        }
        auto instr = Emit(op, output, lhs, rhs);
        for (auto [dest, src] : moves) {
            instr->GetOrNewParallelMove(Instruction::kStart, arena())->AddMove(dest, src, arena());
        }
    }
}; // class Arm64InstructionSelector

InstructionFunction *Arm64SelectFunctionInstructions(base::Arena *arena, Linkage *linkage, ConstantsPool *const_pool,
                                                     ir::Function *fun) {
    Arm64InstructionSelector selector(arena, linkage, const_pool);
    return selector.VisitFunction(fun);
}

} // namespace backend

} // namespace yalx

