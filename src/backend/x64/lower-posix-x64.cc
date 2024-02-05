#include "backend/x64/lower-posix-x64.h"
#include "backend/registers-configuration.h"
#include "backend/constants-pool.h"
#include "backend/linkage-symbols.h"
#include "ir/metadata.h"
#include "ir/utils.h"
#include "ir/node.h"
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

static const InstructionCode machine_representation_move[] = {
    kMaxInstructionCodes, // None
    X64Movb,              // Bit
    X64Movb,              // Word8
    X64Movw,              // Word16
    X64Movl,              // Word32
    X64Movq,              // Word64
    X64Movq,              // Pointer
    X64Movq,              // Reference
    X64Movss,             // Float32
    X64Movsd,             // Float64
};

class X64PosixLower::YalxHandleSelector final : public InstructionSelector::SecondaryStubSelectable {
public:
    YalxHandleSelector(InstructionSelector *owns, ir::Function *fun)
        : SecondaryStubSelectable(owns, fun)
        , stack_size_in_bytes_(owns->YalxHandleStackSizeInBytes(fun)) {}
    ~YalxHandleSelector() override = default;

    void SetUp() override {
        InstructionOperand temps[1];
        temps[0] = ImmediateOperand{static_cast<int>(stack_size_in_bytes_)}; // stack size
        std::vector<InstructionOperand> parameters;
        int gp_idx = 0, fp_idx = 0;
        for (auto arg : fun_->paramaters()) {
            auto rep = ToMachineRepresentation(arg->type());
            if (arg->type().IsFloating()) {
                parameters.push_back(AllocatedOperand::Register(rep, registers()->argument_fp_register(fp_idx++)));
            } else {
                parameters.push_back(AllocatedOperand::Register(rep, registers()->argument_gp_register(gp_idx++)));
            }
            DCHECK(fp_idx <= registers()->number_of_argument_fp_registers());
            DCHECK(gp_idx <= registers()->number_of_argument_gp_registers());
        }
        owns_->Emit(ArchFrameEnter,
             static_cast<int>(parameters.size()), &parameters.front(),
             0, nullptr,
             arraysize(temps), temps);
        int stack_offset = 0;
        for (int i = 0; i < registers()->number_of_callee_save_gp_registers(); i++) {
            stack_offset += kPointerSize;
            auto slot = AllocatedOperand::Slot(MachineRepresentation::kWord64, registers()->fp(), -stack_offset);
            auto gp = AllocatedOperand::Register(MachineRepresentation::kWord64, registers()->callee_save_gp_register(i));
            owns_->Emit(X64Movq, slot, gp);
        }
        for (int i = 0; i < registers()->number_of_callee_save_fp_registers(); i++) {
            stack_offset += kPointerSize;
            auto slot = AllocatedOperand::Slot(MachineRepresentation::kWord64, registers()->fp(), -stack_offset);
            auto fp = AllocatedOperand::Register(MachineRepresentation::kWord64, registers()->callee_save_fp_register(i));
            owns_->Emit(X64Movsd, slot, fp);
        }
    }
    void Call() override {
        int stack_offset = (registers()->number_of_argument_gp_registers() << kPointerShift)
                         + (registers()->number_of_argument_fp_registers() << kPointerShift);
        std::vector<AllocatedOperand> slots;
        std::vector<AllocatedOperand> args;
        int gp_idx = 0, fp_idx = 0;
        for (auto arg : fun_->paramaters()) {
            auto rep = ToMachineRepresentation(arg->type());
            stack_offset = RoundUp(stack_offset, arg->type().AlignmentSizeInBytes()) + arg->type().ReferenceSizeInBytes();
            auto slot = AllocatedOperand::Slot(MachineRepresentation::kWord64, registers()->fp(), -stack_offset);
            auto reg = AllocatedOperand::Register(MachineRepresentation::kNone, -1);
            if (arg->type().IsFloating()) {
                reg = AllocatedOperand::Register(rep, registers()->argument_fp_register(fp_idx++));
            } else {
                reg = AllocatedOperand::Register(rep, registers()->argument_gp_register(gp_idx++));
            }
            DCHECK(fp_idx <= registers()->number_of_argument_fp_registers());
            DCHECK(gp_idx <= registers()->number_of_argument_gp_registers());

            owns_->Emit(machine_representation_move[static_cast<int>(rep)], slot, reg);
            slots.push_back(slot);
            args.push_back(reg);
        }

        auto rt_current_root = InstructionSelector::UseAsExternalCFunction(kRt_current_root);
        auto rs = AllocatedOperand::Register(MachineRepresentation::kPointer, registers()->returning0_register());
        owns_->Emit(ArchCallNative, rs, rt_current_root);
        auto root = AllocatedOperand::Register(MachineRepresentation::kPointer, registers()->root());
        owns_->Emit(X64Movq, root, rs);

        for (size_t i = 0; i < args.size(); i++) {
            auto code = machine_representation_move[static_cast<int>(args[i].machine_representation())];
            DCHECK(code != kMaxInstructionCodes);
            owns_->Emit(code, args[i], slots[i]);
        }

        InstructionOperand symbol[2] = {
            InstructionSelector::UseAsExternalCFunction(stub_name_),
            ImmediateOperand{0}
        };
        // TODO:
//        owns_->Emit(AndBits(ArchCall, CallDescriptorField::Encode(kCallDirectly)),
//                    static_cast<int>(outputs.size()), &outputs[0],
//                    static_cast<int>(args.size()), &args[0],
//                    arraysize(symbol), symbol);
    }
    void TearDown() override {

    }

private:
    const size_t stack_size_in_bytes_ = 0;
}; // class X64PosixLower::YalxHandleSelector

InstructionSelector::SecondaryStubSelectable *X64PosixLower::NewYalxHandleSelector(ir::Function *fun) {
    return new YalxHandleSelector(this, fun);
}

InstructionSelector::SecondaryStubSelectable *X64PosixLower::NewNativeStubSelector(ir::Function *fun) {
    UNREACHABLE();
}


} // namespace yalx::backend