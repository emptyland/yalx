#include "backend/register-allocator.h"
#include "backend/registers-configuration.h"
#include "backend/instruction.h"
#include "backend/frame.h"
#include "ir/metadata.h"
#include "ir/type.h"
#include "ir/node.h"

namespace yalx {

namespace backend {

RegisterAllocator::RegisterAllocator(base::Arena *arena, const RegistersConfiguration *regconf, InstructionFunction *fun)
: arena_(arena)
, regconf_(regconf)
, fun_(fun)
, live_ranges_(new TopTierLiveRange[fun->frame()->virtual_registers_size()]) {
}

RegisterAllocator::~RegisterAllocator() {
    delete[] live_ranges_;
}

void RegisterAllocator::Prepare() {
    for (int i = 0; i < regconf_->number_of_allocatable_gp_registers(); i++) {
        allocatable_gp_registers_.insert(regconf_->allocatable_gp_register(i));
    }
    for (int i = 0; i < regconf_->number_of_allocatable_fp_registers(); i++) {
        allocatable_fp_registers_.insert(regconf_->allocatable_fp_register(i));
    }
}

void RegisterAllocator::ScanLiveRange() {
    auto entry = fun_->entry();
    
    for (auto param : fun_->frame()->fun()->paramaters()) {
        auto vid = fun_->frame()->GetVirtualRegister(param);
        live_ranges_[vid].Update(entry->label(), 0);
    }
    
    ScanBlockGraph(entry);

    for (auto block : fun_->blocks()) {
        DCHECK(tracing_.find(block) != tracing_.end());
    }
}

void RegisterAllocator::ScanBlockGraph(InstructionBlock *block) {
    if (TryTrace(block)) {
        return;
    }

    int step = 0;
    for (auto instr : block->instructions()) {
        for (int i = 0; i < instr->outputs_count(); i++) {
            if (auto opd = instr->OutputAt(i)->AsUnallocated()) {
                Define(opd->virtual_register(), block->label(), step);
            }
        }
        for (int i = 0; i < instr->inputs_count(); i++) {
            if (auto opd = instr->InputAt(i)->AsUnallocated()) {
                Use(opd->virtual_register(), block->label(), step);
            }
        }
//        for (int i = 0; i < instr->temps_count(); i++) {
//            if (auto opd = instr->Te(i)->AsUnallocated()) {
//                Use(opd->virtual_register(), block->label(), step);
//            }
//        }
        step++;
    }
    
    for (auto next_block : block->successors()) {
        ScanBlockGraph(next_block);
    }
}

void RegisterAllocator::Define(int virtual_register, int label, int step) {
    DCHECK(virtual_register >= 0 && virtual_register < fun_->frame()->virtual_registers_size());
    DCHECK(live_ranges_[virtual_register].used_at_start.IsInvliad());
    live_ranges_[virtual_register].used_at_start = {label, step};
}

void RegisterAllocator::Use(int virtual_register, int label, int step) {
    DCHECK(virtual_register >= 0 && virtual_register < fun_->frame()->virtual_registers_size());
    DCHECK(live_ranges_[virtual_register].used_at_end.IsInvliad());
    live_ranges_[virtual_register].used_at_end = {label, step};
}

} // namespace backend

} // namespace yalx

