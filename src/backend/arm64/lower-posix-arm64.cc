#include "backend/arm64/lower-posix-arm64.h"
#include "backend/registers-configuration.h"
#include "backend/constants-pool.h"
#include "ir/metadata.h"
#include "ir/utils.h"
#include "ir/node.h"
#include "ir/condition.h"
#include "base/utils.h"
#include "base/io.h"

namespace yalx::backend {

Arm64PosixLower::Arm64PosixLower(base::Arena *arena, const RegistersConfiguration *profile, Linkage *linkage,
                                 ConstantsPool *const_pool, BarrierSet *barrier_set)
: InstructionSelector(arena, profile, linkage, const_pool, barrier_set) {

}

void Arm64PosixLower::VisitCondBr(ir::Value *instr) {
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

void Arm64PosixLower::VisitAddOrSub(ir::Value *instr) {
    InstructionCode op = ArchNop;
    if (instr->Is(ir::Operator::kAdd)) {
        op = Arm64Add;
    } else {
        DCHECK(instr->Is(ir::Operator::kSub));
        op = Arm64Sub;
    }
    VisitArithOperands(op, DefineAsRegister(instr), instr->InputValue(0),
                       instr->InputValue(1));
}

void Arm64PosixLower::VisitICmp(ir::Value *instr) {
    if (MatchCmpOnlyUsedByBr(instr)) {
        VisitArithOperands(Arm64Cmp, NoOutput(), instr->InputValue(0), instr->InputValue(1));
        return;
    }

    UNREACHABLE(); // TODO:
}

void Arm64PosixLower::VisitLoadAddress(ir::Value *ir) {
    InstructionSelector::VisitLoadAddress(ir);
}

InstructionOperand Arm64PosixLower::TryUseAsConstantOrImmediate(ir::Value *value) {
    if (auto imm = TryUseAsIntegralImmediate(value); !imm.IsInvalid()) {
        return imm;
    }

    switch (value->op()->value()) {
        case ir::Operator::kStringConstant: {
            auto kz = ir::OperatorWith<String const *>::Data(value);
            auto index = const_pool()->FindOrInsertString(kz);
            return ConstantOperand{ConstantOperand::kString, index};
        }
        case ir::Operator::kNilConstant:
            return ImmediateOperand{static_cast<int64_t>(0)};
        case ir::Operator::kF32Constant: {
            auto kf = ir::OperatorWith<float>::Data(value);
            auto index = const_pool()->FindOrInsertFloat32(kf);
            return ConstantOperand{ConstantOperand::kNumber, index};
        }
        case ir::Operator::kF64Constant: {
            auto kf = ir::OperatorWith<double>::Data(value);
            auto index = const_pool()->FindOrInsertFloat64(kf);
            return ConstantOperand{ConstantOperand::kNumber, index};
        }
        default:
            break;
    }

    return {};
}

void Arm64PosixLower::VisitArithOperands(InstructionCode op, InstructionOperand output, ir::Value *input0, ir::Value *input1) {
    std::vector<std::tuple<InstructionOperand, InstructionOperand>> moves;

    InstructionOperand lhs;
    if (IsAnyConstant(input0)) {
        if (auto imm = TryUseAsIntegralImmediate(input0, 16); imm.IsInvalid()) {
            // TODO: Load constants
            UNREACHABLE();
        } else {
            lhs = UseAsRegister(input0);
            moves.emplace_back(lhs, imm);
        }
    } else {
        lhs = UseAsRegister(input0);
    }
    InstructionOperand rhs;
    if (IsAnyConstant(input1)) {
        if (auto imm12 = TryUseAsIntegralImmediate(input1, 12); !imm12.IsInvalid()) {
            rhs = imm12;
        } else if (auto imm16 = TryUseAsIntegralImmediate(input1, 16); !imm16.IsInvalid()) {
            rhs = UseAsRegister(input1);
            moves.emplace_back(rhs, imm16);
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

} // namespace yalx

