#include "backend/zero-slot-allocator.h"
#include "backend/registers-configuration.h"
#include "backend/instruction-code.h"
#include "backend/instruction.h"
#include "backend/frame.h"
#include "ir/type.h"
#include "base/arena-utils.h"
#include "base/arena.h"

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
    auto instr = entry->instruction(0);
    DCHECK(instr->op() == ArchFrameEnter);
    //auto moving = instr->GetOrNewParallelMove(Instruction::kStart, arena_);
    for (int i = 0; i < fun_->frame()->parameters_size(); i++) {
        auto val = fun_->frame()->parameter_value(i);
        auto vr = fun_->frame()->GetVirtualRegister(val);

        USE(vr);
        // TODO:
    }
}

void ZeroSlotAllocator::VisitBlock(InstructionBlock *block) {
    for (auto instr : block->instructions()) {
        if (instr->op() == ArchFrameEnter) {
            DCHECK(frame_enter_ == nullptr);
            frame_enter_ = instr;
        } else if (instr->op() == ArchFrameExit) {
            frame_exit_.push_back(instr);
        } else {
            VisitInstruction(instr);
        }
    }
}

void ZeroSlotAllocator::VisitInstruction(Instruction *instr) {
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
}

void ZeroSlotAllocator::AllocateSlot(Instruction *instr, UnallocatedOperand *unallocated) {
    const auto mr = ToMachineRepresentation(fun_->frame()->GetType(unallocated->virtual_register()));

    switch (unallocated->policy()) {
        case UnallocatedOperand::kRegisterOrSlot:
        case UnallocatedOperand::kRegisterOrSlotOrConstant:
        case UnallocatedOperand::kMustHaveSlot:
            AllocateSlot(unallocated, unallocated->virtual_register());
            break;
        case UnallocatedOperand::kFixedRegister: {
            auto tmp = AllocatedOperand{AllocatedOperand::kRegister, mr, unallocated->fixed_register_id()};
            InstructionOperand slot;
            AllocateSlot(&slot, unallocated->virtual_register());
            instr->GetOrNewParallelMove(Instruction::kEnd, arena_)
                 ->AddMove(slot, tmp, arena_);
            memcpy(unallocated, &tmp, sizeof(tmp));
        } break;
        case UnallocatedOperand::kFixedFPRegister: {
            auto tmp = AllocatedOperand{AllocatedOperand::kRegister, mr, unallocated->fixed_fp_register_id()};
            InstructionOperand slot;
            AllocateSlot(&slot, unallocated->virtual_register());
            instr->GetOrNewParallelMove(Instruction::kEnd, arena_)
                 ->AddMove(slot, tmp, arena_);
            memcpy(unallocated, &tmp, sizeof(tmp));
        } break;
        case UnallocatedOperand::kFixedSlot: {
            int fixed_slot_offset = unallocated->fixed_slot_offset();
            AllocateSlot(unallocated, unallocated->virtual_register(), &fixed_slot_offset);
        } break;
        case UnallocatedOperand::kMustHaveRegister: {
            auto tmp = (mr == MachineRepresentation::kFloat32 || mr == MachineRepresentation::kFloat64)
                    ? AllocatedOperand{AllocatedOperand::kRegister, mr, profile_->allocatable_fp_register(0)}
                    : AllocatedOperand{AllocatedOperand::kRegister, mr, profile_->allocatable_gp_register(0)};
            InstructionOperand slot;
            AllocateSlot(&slot, unallocated->virtual_register());
            instr->GetOrNewParallelMove(Instruction::kEnd, arena_)
                 ->AddMove(slot, tmp, arena_);
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
            auto dest = AllocatedOperand{AllocatedOperand::kSlot, mr, allocated.value};
            memcpy(unallocated, &dest, sizeof(dest));
        } break;
        case UnallocatedOperand::kFixedRegister: {
            auto dest = AllocatedOperand{AllocatedOperand::kRegister, mr, unallocated->fixed_register_id()};
            auto src = AllocatedOperand{AllocatedOperand::kSlot, mr, allocated.value};
            instr->GetOrNewParallelMove(Instruction::kStart, arena_)
                 ->AddMove(dest, src, arena_);
            memcpy(unallocated, &dest, sizeof(dest));
        } break;
        case UnallocatedOperand::kFixedFPRegister: {
            auto dest = AllocatedOperand{AllocatedOperand::kRegister, mr, unallocated->fixed_fp_register_id()};
            auto src = AllocatedOperand{AllocatedOperand::kSlot, mr, allocated.value};
            instr->GetOrNewParallelMove(Instruction::kStart, arena_)
                 ->AddMove(dest, src, arena_);
            memcpy(unallocated, &dest, sizeof(dest));
        } break;
        case UnallocatedOperand::kFixedSlot: {
            if (unallocated->fixed_slot_offset() == allocated.value) {
                break;
            }
            auto dest = AllocatedOperand{AllocatedOperand::kSlot, mr, unallocated->fixed_slot_offset()};
            auto src = AllocatedOperand{AllocatedOperand::kSlot, mr, allocated.value};
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

int ZeroSlotAllocator::AllocateSlot(InstructionOperand *opd, int vr, const int *hint) {
    //auto ir_val = fun_->frame()->GetValue(vr);
    auto mr = ToMachineRepresentation(fun_->frame()->GetType(vr));
    auto size = fun_->frame()->GetType(vr).ReferenceSizeInBytes();
    auto alignment = size < 2 ? 2 : size;
    DCHECK(alignment % 2 == 0);
    DCHECK(alignment > 0 && size > 0);
    int index = INT32_MAX;
    if (!hint) {
        stack_top_ = RoundUp(stack_top_ + static_cast<int>(size), static_cast<intptr_t>(alignment));
        index = -stack_top_;
    } else {
        index = *hint;
    }
    *opd = AllocatedOperand{AllocatedOperand::kSlot, mr, index};
    virtual_allocated_[vr] = {kSlot, index };
    return index;
}


} // namespace yalx::backend
