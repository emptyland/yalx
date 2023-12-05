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
#include <set>

namespace yalx::backend {

class ZeroSlotAllocator::InstrScope {
public:
    explicit InstrScope(ZeroSlotAllocator *owns): owns_(owns) {
        DCHECK(owns->current_ == nullptr);
        owns_->current_ = this;

        for (int i = 0; i < owns_->profile_->number_of_argument_gp_registers(); i++) {
            allocatable_gp_registers_.insert(owns_->profile_->allocatable_gp_register(i));
        }
        for (int i = 0; i < owns_->profile_->number_of_argument_fp_registers(); i++) {
            allocatable_fp_registers_.insert(owns_->profile_->allocatable_fp_register(i));
        }
    }

    AllocatedOperand AllocateRegister(MachineRepresentation rep) {
        int id = -1;
        switch (rep) {
            case MachineRepresentation::kFloat32:
            case MachineRepresentation::kFloat64: {
                DCHECK(!allocatable_fp_registers_.empty());
                auto iter = allocatable_fp_registers_.begin();
                id = *iter;
                allocatable_fp_registers_.erase(iter);
            } break;

            default: {
                DCHECK(!allocatable_gp_registers_.empty());
                auto iter = allocatable_gp_registers_.begin();
                id = *iter;
                allocatable_gp_registers_.erase(iter);
            } break;
        }

        return AllocatedOperand::Register(rep, id);
    }

    AllocatedOperand AllocateFixedRegister(MachineRepresentation rep, int id) {
        switch (rep) {
            case MachineRepresentation::kFloat32:
            case MachineRepresentation::kFloat64: {
                allocatable_fp_registers_.erase(id);
            } break;

            default: {
                allocatable_gp_registers_.erase(id);
            } break;
        }

        return AllocatedOperand::Register(rep, id);
    }

    void Allocated(int vr, AllocatedOperand opd) {
        //allocated_.emplace(vr, opd);
        //allocated_[vr] = opd;
        allocated_.insert(std::make_pair(vr, opd));
        //printf("registered: %d\n", vr);
        DCHECK(allocated_.find(vr) != allocated_.end());
    }

    void Ignore(int vr) { ignore_vr_.insert(vr); }

    bool ShouldIgnore(int vr) const { return ignore_vr_.find(vr) != ignore_vr_.end(); }

    void InvalidIgnore() { ignore_vr_.clear(); }

    bool FindAllocated(InstructionOperand *opd, int vid) const {
        if (auto iter = allocated_.find(vid); iter != allocated_.end()) {
            *opd = iter->second;
            return true;
        }
        if (auto iter = owns_->virtual_allocated_.find(vid); iter != owns_->virtual_allocated_.end()) {
            auto mr = ToMachineRepresentation(owns_->fun_->frame()->GetType(vid));
            if (mr == MachineRepresentation::kNone) {
                mr = MachineRepresentation::kPointer;
            }
            *opd = AllocatedOperand::Slot(mr, owns_->profile_->fp(), iter->second.value);
            return true;
        }
        return false;
    }

    ~InstrScope() {
        DCHECK(owns_->current_ == this);
        owns_->current_ = nullptr;
    }

    DISALLOW_IMPLICIT_CONSTRUCTORS(InstrScope);
private:
    ZeroSlotAllocator *const owns_;
    std::set<int> allocatable_gp_registers_;
    std::set<int> allocatable_fp_registers_;
    std::unordered_map<int, AllocatedOperand> allocated_;
    std::set<int> ignore_vr_;
}; // class ZeroSlotAllocator::InstrScope

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

    DCHECK(call_prepare_.size() % 2 == 0);
    for (size_t i = 0; i < call_prepare_.size(); i += 2) {
        auto before_call = call_prepare_[i];
        if (before_call->temps_count() < 3) {
            continue;
        }
        auto after_call = call_prepare_[i + 1];

        auto current_stack_top = PrepareCallHint::GetCurrentStackTop(before_call);
        auto overflow_args_size = PrepareCallHint::GetOverflowArgsSize(before_call);
        auto returning_val_size = PrepareCallHint::GetReturningValSize(before_call);

        current_stack_top += returning_val_size;
        current_stack_top += overflow_args_size;
        current_stack_top = RoundUp(current_stack_top, Frame::kStackAlignmentSize);

        auto adjust_stack_size = stack_frame_size - current_stack_top;
        PrepareCallHint::SetAdjustStackSize(before_call, adjust_stack_size);
        PrepareCallHint::SetAdjustStackSize(after_call, adjust_stack_size);
    }
}

void ZeroSlotAllocator::VisitParameters(InstructionBlock *entry) {
    DCHECK(entry->instructions_size() > 0);
    auto frame_enter = entry->instruction(0);
    DCHECK(frame_enter->op() == ArchFrameEnter);
    //auto moving = instr->GetOrNewParallelMove(Instruction::kStart, arena_);
//    int gp_index = 0;
//    int fp_index = 0;
    InstrScope scope(this);
    USE(scope);

    for (int i = 0; i < fun_->frame()->parameters_size(); i++) {
        auto opd = frame_enter->OutputAt(i);

        AllocateSlot(frame_enter, opd->AsUnallocated());
    }
}

void ZeroSlotAllocator::VisitBlock(InstructionBlock *block) {
    for (auto instr : block->instructions()) {
        InstrScope scope(this);
        USE(scope);

        switch (instr->op()) {
            case ArchFrameEnter:
                DCHECK(frame_enter_ == nullptr);
                frame_enter_ = instr;
                break;

            case ArchFrameExit:
                frame_exit_.push_back(instr);
                break;

            case ArchBeforeCall:

                DCHECK(call_prepare_.empty() || call_prepare_.back()->op() == ArchAfterCall);
                if (instr->temps_count() >= 3) {
                    *instr->TempAt(2) = ImmediateOperand{stack_top_};
                }
                call_prepare_.push_back(instr);
                break;

            case ArchAfterCall:
                DCHECK(!call_prepare_.empty() && call_prepare_.back()->op() == ArchBeforeCall);
                if (instr->temps_count() >= 3) {
                    *instr->TempAt(2) = *call_prepare_.back()->TempAt(2);
                }
                call_prepare_.push_back(instr);
                break;

            case ArchCall:
                stack_top_ = RoundUp(stack_top_, Frame::kStackAlignmentSize);
                break;

            default:
                break;
        }

        VisitInstruction(instr);

        switch (instr->op()) {
            case ArchFrameExit:
                AddReturningForFrameExit(instr);
                break;
            case ArchCall:
                if (instr->temps_count() >= 2) {
                    auto stack_size = instr->TempAt(1)->AsImmediate()->word32_value();
                    auto padding = (RoundUp(stack_size, Frame::kStackAlignmentSize) - stack_size);
                    //printf("padding = %d\n", padding);
                    stack_top_ += padding;
                }
                break;
            default:
                break;
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

    if (auto moving = instr->parallel_move(Instruction::kStart)) {
        for (auto opd : moving->moves()) {
            if (auto dest = opd->dest().AsUnallocated()) {
                if ((dest->has_must_have_reigster_policy() ||
                    dest->has_fixed_reigster_policy() ||
                    dest->has_fixed_reigster_policy()) &&
                    (opd->src().IsImmediate() ||
                    opd->src().IsConstant() ||
                    opd->src().IsReloaction())) {
                    DCHECK_NOTNULL(current_)->Ignore(dest->virtual_register());
                }
            }
        }
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

    DCHECK_NOTNULL(current_)->InvalidIgnore();

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
    auto rep = ToMachineRepresentation(ty);

    auto dont_ignore_moving = !DCHECK_NOTNULL(current_)->ShouldIgnore(unallocated->virtual_register());

    switch (unallocated->policy()) {
        case UnallocatedOperand::kRegisterOrSlot:
        case UnallocatedOperand::kRegisterOrSlotOrConstant:
        case UnallocatedOperand::kMustHaveSlot:
            AllocateSlotIfNotExists(unallocated, unallocated->virtual_register());
            break;
        case UnallocatedOperand::kFixedRegister: {
            auto tmp = DCHECK_NOTNULL(current_)->AllocateFixedRegister(rep, unallocated->fixed_register_id());
            if (dont_ignore_moving) {
                InstructionOperand slot;
                AllocateSlotIfNotExists(&slot, unallocated->virtual_register());
                AddParallelMove(slot, tmp, ty, instr->GetOrNewParallelMove(Instruction::kEnd, arena_));
            }
            DCHECK_NOTNULL(current_)->Allocated(unallocated->virtual_register(), tmp);
            memcpy(unallocated, &tmp, sizeof(tmp));
        } break;
        case UnallocatedOperand::kFixedFPRegister: {
            auto tmp = DCHECK_NOTNULL(current_)->AllocateFixedRegister(rep, unallocated->fixed_fp_register_id());
            if (dont_ignore_moving) {
                InstructionOperand slot;
                AllocateSlotIfNotExists(&slot, unallocated->virtual_register());
                instr->GetOrNewParallelMove(Instruction::kEnd, arena_)
                        ->AddMove(slot, tmp, arena_);
            }
            DCHECK_NOTNULL(current_)->Allocated(unallocated->virtual_register(), tmp);
            memcpy(unallocated, &tmp, sizeof(tmp));
        } break;
        case UnallocatedOperand::kFixedSlot: {
            int fixed_slot_offset = unallocated->fixed_slot_offset();
            AllocateSlotIfNotExists(unallocated, unallocated->virtual_register(), &fixed_slot_offset);
        } break;
        case UnallocatedOperand::kMustHaveRegister: {
            auto tmp = DCHECK_NOTNULL(current_)->AllocateRegister(rep);
            if (dont_ignore_moving) {
                InstructionOperand slot;
                AllocateSlotIfNotExists(&slot, unallocated->virtual_register());
                AddParallelMove(slot, tmp, ty, instr->GetOrNewParallelMove(Instruction::kEnd, arena_));
            }
            DCHECK_NOTNULL(current_)->Allocated(unallocated->virtual_register(), tmp);
            memcpy(unallocated, &tmp, sizeof(tmp));
        } break;
        case UnallocatedOperand::kSameAsInput:
        default:
            UNREACHABLE();
            break;
    }
}

int ZeroSlotAllocator::ReallocatedSlot(Instruction *instr, UnallocatedOperand *unallocated, Allocated allocated) {
    const auto ty = fun_->frame()->GetType(unallocated->virtual_register());
    const auto rep = ToMachineRepresentation(ty);

    switch (unallocated->policy()) {
        case UnallocatedOperand::kRegisterOrSlot:
        case UnallocatedOperand::kRegisterOrSlotOrConstant:
        case UnallocatedOperand::kMustHaveSlot: {
            auto dest = AllocatedOperand::Slot(rep, profile_->fp(), allocated.value);
            memcpy(unallocated, &dest, sizeof(dest));
        } break;
        case UnallocatedOperand::kFixedRegister: {
            auto dest = DCHECK_NOTNULL(current_)->AllocateFixedRegister(rep, unallocated->fixed_register_id());
            auto src = AllocatedOperand::Slot(rep, profile_->fp(), allocated.value);

            instr->GetOrNewParallelMove(Instruction::kStart, arena_)
                    ->AddMove(dest, src, arena_)
                    ->should_load_address(ty.ShouldValuePassing());

            DCHECK_NOTNULL(current_)->Allocated(unallocated->virtual_register(), dest);
            memcpy(unallocated, &dest, sizeof(dest));
        } break;
        case UnallocatedOperand::kFixedFPRegister: {
            auto dest = DCHECK_NOTNULL(current_)->AllocateFixedRegister(rep, unallocated->fixed_fp_register_id());
            auto src = AllocatedOperand::Slot(rep, profile_->fp(), allocated.value);
            instr->GetOrNewParallelMove(Instruction::kStart, arena_)
                 ->AddMove(dest, src, arena_);

            DCHECK_NOTNULL(current_)->Allocated(unallocated->virtual_register(), dest);
            memcpy(unallocated, &dest, sizeof(dest));
        } break;
        case UnallocatedOperand::kFixedSlot: {
            if (unallocated->fixed_slot_offset() == allocated.value) {
                break;
            }
            auto dest = AllocatedOperand::Slot(rep, profile_->fp(), unallocated->fixed_slot_offset());
            auto src = AllocatedOperand::Slot(rep, profile_->fp(), allocated.value);
            instr->GetOrNewParallelMove(Instruction::kStart, arena_)
                 ->AddMove(dest, src, arena_);
            memcpy(unallocated, &dest, sizeof(dest));
        } break;
        case UnallocatedOperand::kMustHaveRegister: {
            auto dest = DCHECK_NOTNULL(current_)->AllocateRegister(rep);
            auto src = AllocatedOperand::Slot(rep, profile_->fp(), allocated.value);
            instr->GetOrNewParallelMove(Instruction::kStart, arena_)
                 ->AddMove(dest, src, arena_);
            memcpy(unallocated, &dest, sizeof(dest));
        } break;
        case UnallocatedOperand::kSameAsInput:
        default:
            UNREACHABLE();
            break;
    }
    return 0;
}

bool ZeroSlotAllocator::AllocateSlotIfNotExists(InstructionOperand *opd, int vr, const int *hint) {
//    if (auto iter = virtual_allocated_.find(vr); iter != virtual_allocated_.end()) {
//        auto mr = ToMachineRepresentation(fun_->frame()->GetType(vr));
//        if (mr == MachineRepresentation::kNone) {
//            mr = MachineRepresentation::kPointer;
//        }
//        *opd = AllocatedOperand::Slot(mr, profile_->fp(), iter->second.value);
//        return true;
//    }
    if (DCHECK_NOTNULL(current_)->FindAllocated(opd, vr)) {
        return true;
    }
    AllocateSlot(opd, vr, hint);
    return false;
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
