#include "backend/barrier-set.h"
#include "backend/linkage-symbols.h"
#include "backend/instruction-selector.h"
#include "backend/registers-configuration.h"
#include "ir/node.h"
#include "ir/metadata.h"
#include "base/format.h"

namespace yalx::backend {

class NoGCBarrierSet final : public BarrierSet {
public:
    NoGCBarrierSet() = default;
    ~NoGCBarrierSet() override = default;

    void PostLoad(InstructionSelector *selector, ir::Value *ir) override {}
    void PreStore(InstructionSelector *selector) override {}
    void PostStore(InstructionSelector *selector) override {}
}; //class NoGCBarrierSet final

class X64PosixYGCBarrierSet final : public BarrierSet {
public:
    X64PosixYGCBarrierSet() = default;
    ~X64PosixYGCBarrierSet() override = default;

    void PostLoad(InstructionSelector *selector, ir::Value *ir) override {
        auto profile = selector->registers();
        auto bad_mask = AllocatedOperand::Register(MachineRepresentation::kWord64, profile->scratch0());
        auto bad_mask_symbol = ReloactionOperand{kRt_YGC_ADDRESS_BAD_MASK};
        selector->Emit(X64Movq, bad_mask, bad_mask_symbol);
        selector->Emit(X64Test, {}, bad_mask, selector->UseAsRegisterOrSlot(ir));
        auto label = selector->Emit(X64Jz, InstructionSelector::NoOutput(),
                                    InstructionSelector::NoOutput()); // Good if ZF = 0

        InstructionOperand saved_regs[] = {
            AllocatedOperand::Register(MachineRepresentation::kWord64, profile->returning0_register()),
            AllocatedOperand::Register(MachineRepresentation::kWord64, profile->argument_gp_register(0)),
            AllocatedOperand::Register(MachineRepresentation::kWord64, profile->argument_gp_register(1)),
            AllocatedOperand::Register(MachineRepresentation::kWord64, profile->root()),
        };

        selector->Emit(ArchBeforeCall, 0, nullptr, arraysize(saved_regs),
                       saved_regs, 0, nullptr);

        auto arg0 = AllocatedOperand::Register(MachineRepresentation::kPointer,
                                               profile->argument_gp_register(0));
        switch (ir->op()->value()) {
            case ir::Operator::kLoadInlineField: {
                auto handle = ir::OperatorWith<const ir::Handle *>::Data(ir);
                auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
                selector->Emit(X64Lea, arg0, selector->UseAsSlot(ir->InputValue(0)));
                selector->Emit(X64Add, arg0, ImmediateOperand{field->offset});
            } break;
            default:
                UNREACHABLE();
                break;
        }

        selector->Emit(ArchCallNative,
                       selector->UseAsFixedRegister(ir, profile->returning0_register()),
                       InstructionSelector::UseAsExternalCFunction(kRt_ygc_barrier_load_on_field),
                       arg0);

        selector->Emit(ArchAfterCall, 0, nullptr, arraysize(saved_regs),
                       saved_regs, 0, nullptr);

        auto label_id = selector->PutJumpingPositionToCurrentBlock(next_id_++);
        auto buf = base::Sprintf("Jpt_%d", label_id);
        auto label_name = String::New(selector->arena(), buf.data(), buf.size());
        *label->InputAt(0) = ReloactionOperand{label_name};
    }

    void PreStore(InstructionSelector *selector) override {}
    void PostStore(InstructionSelector *selector) override {}

private:
    int next_id_ = 0;
}; // class X64PosixYGCBarrierSet


class Arm64PosixYGCBarrierSet final : public BarrierSet {
public:
    Arm64PosixYGCBarrierSet() = default;
    ~Arm64PosixYGCBarrierSet() override = default;

    void PostLoad(InstructionSelector *selector, ir::Value *ir) override {
        auto profile = selector->registers();
        auto bad_mask = AllocatedOperand::Register(MachineRepresentation::kWord64,
                                                   profile->scratch0());
        auto bad_mask_symbol = ReloactionOperand{kRt_YGC_ADDRESS_BAD_MASK};
        selector->Emit(Arm64Adrp, bad_mask, bad_mask_symbol);
        selector->Emit(Arm64AddOff, bad_mask, bad_mask, bad_mask_symbol);
        auto bad_mask_addr = AllocatedOperand::Location(MachineRepresentation::kWord64, profile->scratch0(), 0);
        selector->Emit(Arm64Ldr, bad_mask, bad_mask_addr);
        selector->Emit(Arm64Tst, InstructionSelector::NoOutput(), bad_mask, selector->UseAsRegister(ir));
        auto label = selector->Emit(Arm64B_eq, InstructionSelector::NoOutput(),
                                               InstructionSelector::NoOutput()); // Good if ZF = 0

        InstructionOperand saved_regs[] = {
                AllocatedOperand::Register(MachineRepresentation::kWord64, profile->returning0_register()),
                AllocatedOperand::Register(MachineRepresentation::kWord64, profile->argument_gp_register(0)),
                AllocatedOperand::Register(MachineRepresentation::kWord64, profile->argument_gp_register(1)),
                AllocatedOperand::Register(MachineRepresentation::kWord64, profile->root()),
        };

        selector->Emit(ArchBeforeCall, 0, nullptr, arraysize(saved_regs),
                       saved_regs, 0, nullptr);

        auto arg0 = AllocatedOperand::Register(MachineRepresentation::kPointer,
                                               profile->argument_gp_register(0));
        switch (ir->op()->value()) {
            case ir::Operator::kLoadInlineField: {
                auto handle = ir::OperatorWith<const ir::Handle *>::Data(ir);
                auto field = std::get<const ir::Model::Field *>(handle->owns()->GetMember(handle));
                selector->Emit(ArchLoadEffectAddress, arg0,
                               selector->UseAsSlot(ir->InputValue(0)),
                               ImmediateOperand{field->offset});
            } break;
            default:
                UNREACHABLE();
                break;
        }

        selector->Emit(ArchCallNative,
                       selector->UseAsFixedRegister(ir, profile->returning0_register()),
                       InstructionSelector::UseAsExternalCFunction(kRt_ygc_barrier_load_on_field),
                       arg0);

        selector->Emit(ArchAfterCall, 0, nullptr, arraysize(saved_regs),
                       saved_regs, 0, nullptr);

        auto label_id = selector->PutJumpingPositionToCurrentBlock(next_id_++);
        auto buf = base::Sprintf("Jpt_%d", label_id);
        auto label_name = String::New(selector->arena(), buf.data(), buf.size());
        *label->InputAt(0) = ReloactionOperand{label_name};
    }

    void PreStore(InstructionSelector *selector) override {}
    void PostStore(InstructionSelector *selector) override {}

private:
    int next_id_ = 0;
}; // class Arm64PosixYGCBarrierSet

BarrierSet *BarrierSet::OfNoGC() {
    return new NoGCBarrierSet();
}

BarrierSet *BarrierSet::OfYGCPosixX64() {
    return new X64PosixYGCBarrierSet();
}

BarrierSet *BarrierSet::OfYGCPosixArm64() {
    return new Arm64PosixYGCBarrierSet();
}

} //namespace yalx::backend
