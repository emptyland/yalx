#include "backend/zero-slot-allocator.h"
#include "backend/registers-configuration.h"
#include "backend/instruction-code.h"
#include "backend/instruction.h"
#include "backend/frame.h"
#include "ir/metadata.h"
#include "ir/type.h"
#include "ir/node.h"
#include "base/arena.h"
#include "base/format.h"

namespace yalx::backend {

ZeroSlotAllocator::ZeroSlotAllocator(base::Arena *arena, const RegistersConfiguration *profile, InstructionFunction *fun)
: arena_(arena)
, profile_(profile)
, fun_(fun)
, stack_top_(fun->frame()->stack_size()) {

}

void ZeroSlotAllocator::Run() {
    auto entry = fun_->entry();
    VisitParameters(entry);
    ProcessBlock(entry);
    ProcessFrame();
}

void ZeroSlotAllocator::ProcessBlock(InstructionBlock *block) {
    VisitBlock(block);
    for (auto child : block->successors()) {
        ProcessBlock(child);
    }
}

void ZeroSlotAllocator::ProcessFrame() {
    const auto stack_frame_size = RoundUp(stack_top_, Frame::kStackAlignmentSize);
    if (frame_enter_) {
        auto tmp = frame_enter_->TempAt(0);
        *tmp = ImmediateOperand{stack_frame_size};
    }
    for (auto frame_exit : frame_exit_) {
        auto tmp = frame_exit->TempAt(0);
        *tmp = ImmediateOperand{stack_frame_size};
    }
}

void ZeroSlotAllocator::VisitParameters(InstructionBlock *entry) {
    DCHECK(entry->instructions_size() > 0);
    auto frame_enter = entry->instruction(0);
    DCHECK(frame_enter->op() == ArchFrameEnter);
    //auto moving = instr->GetOrNewParallelMove(Instruction::kStart, arena_);
//    int gp_index = 0;
//    int fp_index = 0;
    for (int i = 0; i < fun_->frame()->parameters_size(); i++) {
        auto val = fun_->frame()->parameter_value(i);
        auto vr = fun_->frame()->GetVirtualRegister(val);

        InstructionOperand dest{};
        AllocateSlot(&dest, vr);
    }
}

void ZeroSlotAllocator::VisitBlock(InstructionBlock *block) {
    for (auto instr : block->instructions()) {
        if (instr->op() == ArchFrameEnter) {
            DCHECK(frame_enter_ == nullptr);
            frame_enter_ = instr;
            //continue;
        } else if (instr->op() == ArchFrameExit) {
            frame_exit_.push_back(instr);
        }
        VisitInstruction(instr);
        if (instr->op() == ArchFrameExit) {
            AddReturningForFrameExit(instr);
        }
    }
}

void ZeroSlotAllocator::VisitInstruction(Instruction *instr) {
    if (auto moving = instr->parallel_move(Instruction::kStart)) {
        for (auto opd : moving->moves()) {
            if (auto src = opd->src().AsUnallocated()) {
                AllocateSlotIfNotExists(opd->mutable_src(), src->virtual_register());
            }
            if (auto dest = opd->dest().AsUnallocated()) {
                AllocateSlotIfNotExists(opd->mutable_dest(), dest->virtual_register());
            }
        }
    }

    for (int i = 0; i < instr->outputs_count(); i++) {
        auto out = instr->OutputAt(i);
        if (!out->IsUnallocated()) {
            break;
        }
        AllocateSlot(instr, out->AsUnallocated());
    }

    for (int i = 0; i < instr->inputs_count(); i++) {
        auto in = instr->InputAt(i);
        if (!in->IsUnallocated()) {
            break;
        }

        auto unallocated = in->AsUnallocated();
        auto iter = virtual_allocated_.find(unallocated->virtual_register());
        if (iter == virtual_allocated_.end()) {
            AllocateSlot(instr, unallocated);
        } else {
            ReallocatedSlot(instr, unallocated, iter->second);
        }
    }

    if (auto moving = instr->parallel_move(Instruction::kEnd)) {
        for (auto opd : moving->moves()) {
            if (auto src = opd->src().AsUnallocated()) {
                AllocateSlotIfNotExists(opd->mutable_src(), src->virtual_register());
            }
            if (auto dest = opd->dest().AsUnallocated()) {
                AllocateSlotIfNotExists(opd->mutable_dest(), dest->virtual_register());
            }
        }
    }
}

void ZeroSlotAllocator::AddReturningForFrameExit(Instruction *instr) {
    auto frame = fun_->frame();
    auto returning_val_offset = frame->returning_val_offset();
    auto prototype = frame->fun()->prototype();
    int input_at = instr->inputs_count() - 1;
    for (int64_t i = static_cast<int64_t>(prototype->return_types_size()) - 1; i >= 0; i--) {
        auto ty = prototype->return_type(i);
        if (ty.kind() == ir::Type::kVoid) {
            continue;
        }
        auto input = instr->InputAt(input_at--);

        auto rep = ToMachineRepresentation(ty);
        auto dest = AllocatedOperand::Slot(rep, profile_->fp(), returning_val_offset);
        AddParallelMove(dest, *input, ty, instr->GetOrNewParallelMove(Instruction::kStart, arena_));

        returning_val_offset += RoundUp(ty.ReferenceSizeInBytes(), static_cast<intptr_t>(ty.AlignmentSizeInBytes()));
    }
}

void ZeroSlotAllocator::AllocateSlot(Instruction *instr, UnallocatedOperand *unallocated) {
    auto ty = fun_->frame()->GetType(unallocated->virtual_register());
    auto mr = ToMachineRepresentation(ty);
    if (mr == MachineRepresentation::kNone) {
        mr = MachineRepresentation::kPointer;
    }

    switch (unallocated->policy()) {
        case UnallocatedOperand::kRegisterOrSlot:
        case UnallocatedOperand::kRegisterOrSlotOrConstant:
        case UnallocatedOperand::kMustHaveSlot:
            AllocateSlotIfNotExists(unallocated, unallocated->virtual_register());
            break;
        case UnallocatedOperand::kFixedRegister: {
            auto tmp = AllocatedOperand::Register(mr, unallocated->fixed_register_id());
            InstructionOperand slot;
            AllocateSlotIfNotExists(&slot, unallocated->virtual_register());
            AddParallelMove(slot, tmp, ty, instr->GetOrNewParallelMove(Instruction::kEnd, arena_));
            memcpy(unallocated, &tmp, sizeof(tmp));
        } break;
        case UnallocatedOperand::kFixedFPRegister: {
            auto tmp = AllocatedOperand::Register(mr, unallocated->fixed_fp_register_id());
            InstructionOperand slot;
            AllocateSlotIfNotExists(&slot, unallocated->virtual_register());
            instr->GetOrNewParallelMove(Instruction::kEnd, arena_)
                 ->AddMove(slot, tmp, arena_);
            memcpy(unallocated, &tmp, sizeof(tmp));
        } break;
        case UnallocatedOperand::kFixedSlot: {
            int fixed_slot_offset = unallocated->fixed_slot_offset();
            AllocateSlotIfNotExists(unallocated, unallocated->virtual_register(), &fixed_slot_offset);
        } break;
        case UnallocatedOperand::kMustHaveRegister: {
            auto tmp = (mr == MachineRepresentation::kFloat32 || mr == MachineRepresentation::kFloat64)
                    ? AllocatedOperand::Register(mr, profile_->allocatable_fp_register(0))
                    : AllocatedOperand::Register(mr, profile_->allocatable_gp_register(0));
            InstructionOperand slot;
            AllocateSlotIfNotExists(&slot, unallocated->virtual_register());
            AddParallelMove(slot, tmp, ty, instr->GetOrNewParallelMove(Instruction::kEnd, arena_));
            memcpy(unallocated, &tmp, sizeof(tmp));
        } break;
        case UnallocatedOperand::kSameAsInput:
        default:
            UNREACHABLE();
            break;
    }
}

int ZeroSlotAllocator::ReallocatedSlot(Instruction *instr, UnallocatedOperand *unallocated, Allocated allocated) {
    const auto mr = ToMachineRepresentation(fun_->frame()->GetType(unallocated->virtual_register()));

    switch (unallocated->policy()) {
        case UnallocatedOperand::kRegisterOrSlot:
        case UnallocatedOperand::kRegisterOrSlotOrConstant:
        case UnallocatedOperand::kMustHaveSlot: {
            auto dest = AllocatedOperand::Slot(mr, profile_->fp(),allocated.value);
            memcpy(unallocated, &dest, sizeof(dest));
        } break;
        case UnallocatedOperand::kFixedRegister: {
            auto dest = AllocatedOperand::Register(mr, unallocated->fixed_register_id());
            auto src = AllocatedOperand::Slot(mr, profile_->fp(), allocated.value);
            instr->GetOrNewParallelMove(Instruction::kStart, arena_)
                 ->AddMove(dest, src, arena_);
            memcpy(unallocated, &dest, sizeof(dest));
        } break;
        case UnallocatedOperand::kFixedFPRegister: {
            auto dest = AllocatedOperand::Register(mr, unallocated->fixed_fp_register_id());
            auto src = AllocatedOperand::Slot(mr, profile_->fp(), allocated.value);
            instr->GetOrNewParallelMove(Instruction::kStart, arena_)
                 ->AddMove(dest, src, arena_);
            memcpy(unallocated, &dest, sizeof(dest));
        } break;
        case UnallocatedOperand::kFixedSlot: {
            if (unallocated->fixed_slot_offset() == allocated.value) {
                break;
            }
            auto dest = AllocatedOperand::Slot(mr, profile_->fp(), unallocated->fixed_slot_offset());
            auto src = AllocatedOperand::Slot(mr, profile_->fp(), allocated.value);
            instr->GetOrNewParallelMove(Instruction::kStart, arena_)
                 ->AddMove(dest, src, arena_);
            memcpy(unallocated, &dest, sizeof(dest));
        } break;
        case UnallocatedOperand::kMustHaveRegister:
            // TODO:
        case UnallocatedOperand::kSameAsInput:
        default:
            UNREACHABLE();
            break;
    }
    return 0;
}

int ZeroSlotAllocator::AllocateSlotIfNotExists(InstructionOperand *opd, int vr, const int *hint) {
    if (auto iter = virtual_allocated_.find(vr); iter != virtual_allocated_.end()) {
        auto mr = ToMachineRepresentation(fun_->frame()->GetType(vr));
        if (mr == MachineRepresentation::kNone) {
            mr = MachineRepresentation::kPointer;
        }
        *opd = AllocatedOperand::Slot(mr, profile_->fp(), iter->second.value);
        return iter->second.value;
    }
    return AllocateSlot(opd, vr, hint);
}

int ZeroSlotAllocator::AllocateSlot(InstructionOperand *opd, int vr, const int *hint) {
    auto mr = ToMachineRepresentation(fun_->frame()->GetType(vr));
    if (mr == MachineRepresentation::kNone) {
        mr = MachineRepresentation::kPointer;
    }
    auto ty = fun_->frame()->GetType(vr);
    auto size = ty.ReferenceSizeInBytes();
    auto alignment = ty.AlignmentSizeInBytes();
    DCHECK(alignment % 2 == 0);
    DCHECK(alignment > 0 && size > 0);

    int index = INT32_MAX;
    if (!hint) {
        stack_top_ = RoundUp(stack_top_ + static_cast<int>(size), static_cast<intptr_t>(alignment));
        index = -stack_top_;
    } else {
        index = *hint;
    }
    *opd = AllocatedOperand::Slot(mr, profile_->fp(), index);
    virtual_allocated_[vr] = {kSlot, index };
    return index;
}

void ZeroSlotAllocator::AddParallelMove(const InstructionOperand &dest, const InstructionOperand &src, const ir::Type &ty,
                                        ParallelMove *moving) {
    if (ty.ReferenceSizeInBytes() <= kPointerSize) {
        moving->AddMove(dest, src, arena_);
        return;
    }

    auto out = dest.AsAllocated();
    auto in = src.AsAllocated();

    if (out->IsSlot() && in->IsRegisterLocation()) {

        auto offset = out->index();
        for (auto i = 0; i < ty.ReferenceSizeInBytes() / kPointerSize; i++) {
            auto a = AllocatedOperand::Slot(MachineRepresentation::kWord64, out->register_id(), offset);
            auto b = AllocatedOperand::Location(MachineRepresentation::kWord64, in->register_id(), i * kPointerSize);
            auto comment = base::Sprintf("move value: %s", ty.ToString().data());
            moving->AddMove(a, b, comment.c_str(), arena_);
            offset += kPointerSize;
        }
    } else if (out->IsSlot() && in->IsSlot()) {

        auto out_offset = out->index();
        auto in_offset = in->index();
        for (auto i = 0; i < ty.ReferenceSizeInBytes() / kPointerSize; i++) {
            auto a = AllocatedOperand::Slot(MachineRepresentation::kWord64, out->register_id(), out_offset);
            auto b = AllocatedOperand::Slot(MachineRepresentation::kWord64, in->register_id(), in_offset);
            auto comment = base::Sprintf("move value: %s", ty.ToString().data());
            moving->AddMove(a, b, comment.c_str(), arena_);
            out_offset += kPointerSize;
            in_offset += kPointerSize;
        }
    } else {
        UNREACHABLE();
    }
}


} // namespace yalx::backend
