#include "backend/x64/lower-posix-x64.h"
#include "backend/registers-configuration.h"
#include "backend/constants-pool.h"
#include "ir/metadata.h"
#include "ir/utils.h"
#include "ir/node.h"
#include "ir/type.h"
#include "base/utils.h"
#include "base/io.h"

namespace yalx::backend {

X64PosixLower::X64PosixLower(base::Arena *arena, const RegistersConfiguration *profile, Linkage *linkage,
                             ConstantsPool *const_pool, BarrierSet *barrier_set)
                             : InstructionSelector(arena, profile, linkage, const_pool, barrier_set) {
}


void X64PosixLower::VisitCondBr(ir::Value *instr) {
    UNREACHABLE();
}

// v3 = v1 + v2
// --------------
// mov v3, v1
// add v3, v2
//
// v3 = v1 - v2
// --------------
// mov v3, v1
// sub v3, v2

void X64PosixLower::VisitAddOrSub(ir::Value *ir) {
    DCHECK(ir->op()->value_in() == 2);
    auto rv = DefineAsRegister(ir->OutputValue(0));

    InstructionOperand lhs = TryUseAsIntegralImmediate(ir->InputValue(0));
    if (lhs.IsInvalid()) {
        lhs = DefineAsRegisterOrSlot(ir->InputValue(0));
    }
    InstructionOperand rhs = TryUseAsIntegralImmediate(ir->InputValue(1));
    if (rhs.IsInvalid()) {
        rhs = DefineAsRegisterOrSlot(ir->InputValue(1));
    }
    Instruction *instr = nullptr;
    auto mr = ToMachineRepresentation(ir->InputValue(0)->type());
    switch (mr) {
        case MachineRepresentation::kWord8:
            if (ir->Is(ir::Operator::kAdd)) {
                instr = Emit(X64Add8, rv, rhs);
            } else {
                instr = Emit(X64Sub8, rv, rhs);
            }
            break;
        case MachineRepresentation::kWord16:
            if (ir->Is(ir::Operator::kAdd)) {
                instr = Emit(X64Add16, rv, rhs);
            } else {
                instr = Emit(X64Sub16, rv, rhs);
            }
            break;
        case MachineRepresentation::kWord32:
            //Emit(X64Movl, tmp, lhs);
            if (ir->Is(ir::Operator::kAdd)) {
                instr = Emit(X64Add32, rv, rhs);
            } else {
                instr = Emit(X64Sub32, rv, rhs);
            }
            break;
        case MachineRepresentation::kWord64:
            if (ir->Is(ir::Operator::kAdd)) {
                instr = Emit(X64Add, rv, rhs);
            } else {
                instr = Emit(X64Sub, rv, rhs);
            }
            break;
        case MachineRepresentation::kFloat32:
            Emit(X64Movss, lhs, rhs);
            UNREACHABLE();
            break;
        case MachineRepresentation::kFloat64:
            Emit(X64Movsd, lhs, rhs);
            UNREACHABLE();
            break;
        default:
            UNREACHABLE();
            break;
    }
    USE(instr);
    instr->GetOrNewParallelMove(Instruction::kStart, arena())
         ->AddMove(rv, lhs, arena());
}

void X64PosixLower::VisitICmp(ir::Value *instr) {
    UNREACHABLE();
}

void X64PosixLower::VisitLoadAddress(ir::Value *ir) {
    auto input = UseAsSlot(ir->InputValue(0));
    auto output = DefineAsRegister(ir);
    Emit(X64Lea, output, input);
}

//void X64PosixLower::VisitLoadInlineField(ir::Value *ir) {
//    auto address = AllocatedOperand::Register(MachineRepresentation::kPointer, config()->scratch0());
//    Emit(X64Lea, address, UseAsSlot(ir->InputValue(0)));
//
//    auto handle = ir::OperatorWith<const ir::Handle *>::Data(ir);
//    DCHECK(handle->IsField());
//    auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
//
//    auto rep = ToMachineRepresentation(ir->type());
//    auto src = AllocatedOperand::Location(rep, config()->scratch0(), field->offset);
//
//    auto output = DefineAsRegister(ir);
//    switch (rep) {
//        case MachineRepresentation::kWord8:
//            Emit(X64Movb, output, src);
//            break;
//        case MachineRepresentation::kWord16:
//            Emit(X64Movw, output, src);
//            break;
//        case MachineRepresentation::kWord32:
//            Emit(X64Movl, output, src);
//            break;
//        case MachineRepresentation::kWord64:
//        case MachineRepresentation::kPointer:
//        case MachineRepresentation::kReference:
//            Emit(X64Movq, output, src);
//            break;
//        case MachineRepresentation::kFloat32:
//            Emit(X64Movss, output, src);
//            break;
//        case MachineRepresentation::kFloat64:
//            Emit(X64Movsd, output, src);
//            break;
//        default:
//            UNREACHABLE();
//            break;
//    }
//}

InstructionOperand X64PosixLower::TryUseAsConstantOrImmediate(ir::Value *value) {
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

} // namespace yalx::backend