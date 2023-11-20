#include "backend/x64/lower-posix-x64.h"
#include "backend/constants-pool.h"
#include "ir/metadata.h"
#include "ir/utils.h"
#include "ir/node.h"
#include "ir/type.h"
#include "base/utils.h"
#include "base/io.h"

namespace yalx::backend {

X64PosixLower::X64PosixLower(base::Arena *arena, const RegistersConfiguration *config, Linkage *linkage,
                             ConstantsPool *const_pool)
                             : InstructionSelector(arena, config, linkage, const_pool) {
}


void X64PosixLower::VisitCondBr(ir::Value *instr) {
    UNREACHABLE();
}

void X64PosixLower::VisitAddOrSub(ir::Value *instr) {
    UNREACHABLE();
}

void X64PosixLower::VisitICmp(ir::Value *instr) {
    UNREACHABLE();
}

InstructionOperand X64PosixLower::Select(ir::Value *instr, InstructionOperand hint) {
    switch (instr->op()->value()) {
        case ir::Operator::kStringConstant: {
            auto value = ir::OperatorWith<String const *>::Data(instr);
            auto offset = const_pool()->FindOrInsertString(value);
            auto opd = ConstantOperand{ConstantOperand::kString, offset};
            if (hint.IsInvalid()) {
                return opd;
            } else {
                Emit(X64Movq, hint, opd);
                return hint;
            }
        }
        case ir::Operator::kWord8Constant:
        case ir::Operator::kU8Constant: {
            auto value = ir::OperatorWith<uint8_t>::Data(instr);
            auto opd = ImmediateOperand{value};
            if (hint.IsInvalid()) {
                return opd;
            } else {
                Emit(X64Movb, hint, opd);
                return hint;
            }
        }
        case ir::Operator::kWord16Constant:
            break;
        case ir::Operator::kWord32Constant:
        case ir::Operator::kU32Constant:
        case ir::Operator::kI32Constant: {
            auto value = ir::OperatorWith<int32_t>::Data(instr);
            auto opd = ImmediateOperand{value};
            if (hint.IsInvalid()) {
                return opd;
            } else {
                Emit(X64Movl, hint, opd);
                return hint;
            }
        }
        case ir::Operator::kWord64Constant:
            break;
        case ir::Operator::kI8Constant:
            break;
        case ir::Operator::kI16Constant:
            break;
        case ir::Operator::kI64Constant:
            break;
        case ir::Operator::kU16Constant:
            break;
        case ir::Operator::kU64Constant:
            break;
        case ir::Operator::kF32Constant:
            break;
        case ir::Operator::kF64Constant:
            break;
        case ir::Operator::kNilConstant:
            break;

        default:
            if (hint.IsInvalid()) {
                return UseAsRegisterOrSlot(instr);
            }
            break;
    }

    UNREACHABLE();
}

} // namespace yalx::backend