#include "backend/register-allocator.h"
#include "backend/registers-configuration.h"
#include "backend/instruction.h"
#include "backend/frame.h"
#include "ir/metadata.h"
#include "ir/type.h"
#include "ir/node.h"
#include <numeric>

namespace yalx {

namespace backend {

void BlockLivenssState::Reset(size_t capacity) {
    live_kill_.clear();
    live_gen_.clear();
    live_in_.clear();
    live_out_.clear();
    capacity_ = capacity;
}

LivenessInterval::LivenessInterval(ir::Type type)
: type_(type)
, rep_(ToMachineRepresentation(type)) {
}

LivenessInterval::~LivenessInterval() {
    
}

RegisterAllocator::RegisterAllocator(base::Arena *arena, const RegistersConfiguration *regconf, InstructionFunction *fun)
: arena_(arena)
, regconf_(regconf)
, fun_(fun) {
}

RegisterAllocator::~RegisterAllocator() {
    for (auto interval : intervals_) { delete interval; }
}

void RegisterAllocator::Prepare() {

}

void RegisterAllocator::ComputeBlocksOrder() {
    std::deque<InstructionBlock *> work_list;
    std::map<InstructionBlock *, int> incoming_forward_branchs;
    for (auto block : fun_->blocks()) {
        incoming_forward_branchs[block] = static_cast<int>(block->predecessors_size());
    }
    work_list.push_back(fun_->entry());
    
    while (!work_list.empty()) {
        auto block = work_list.front();
        work_list.pop_front();
        auto iter = std::find(blocks_.begin(), blocks_.end(), block);
        if (iter == blocks_.end()) {
            blocks_.push_back(block);
        }
        
        if (block->successors_size() == 1) {
            auto succ = block->successors().front();
            --incoming_forward_branchs[succ];
            work_list.push_back(succ);
        } else {
            for (auto succ : block->successors()) {
                auto current = incoming_forward_branchs[succ];
                if (current == 0) {
                    continue;
                }
                if (--incoming_forward_branchs[succ] == 0) {
                    work_list.push_back(succ);
                }
            }
        }
    }
}

void RegisterAllocator::NumberizeAllInstructions() {
    int next_id = 0;
    for (auto block : blocks_) {
        for (auto phi : block->phis()) {
            phi->set_id(next_id);
            next_id += 2;
        }
        for (auto instr : block->instructions()) {
            instr->set_id(next_id);
            next_id += 2;
        }
    }
}

void RegisterAllocator::ComputeLocalLiveSets() {
    blocks_liveness_state_.resize(blocks_.size());
    for (auto &state : blocks_liveness_state_) {
        state.Reset(fun_->frame()->virtual_registers_size());
    }
    
    for (int i = 0; i < blocks_.size(); i++) {
        auto block = blocks_[i];
        auto state = &blocks_liveness_state_[i];
        
        for (auto phi : block->phis()) {
            for (auto vr : phi->operands()) {
                if (state->DoesNotInLiveKill(vr)) {
                    state->AddLiveGen(vr);
                }
            }
            state->AddLiveKill(phi->virtual_register());
        }
        
        for (auto instr : block->instructions()) {
            for (int j = 0; j < instr->inputs_count(); j++) {
                if (auto opd = instr->InputAt(j)->AsUnallocated()) {
                    if (state->DoesNotInLiveKill(opd->virtual_register())) {
                        state->AddLiveGen(opd->virtual_register());
                    }
                }
            }
            for (int j = 0; j < instr->temps_count(); j++) {
                if (auto opd = instr->TempAt(j)->AsUnallocated()) {
                    state->AddLiveKill(opd->virtual_register());
                }
            }
            for (int j = 0; j < instr->outputs_count(); j++) {
                if (auto opd = instr->OutputAt(j)->AsUnallocated()) {
                    state->AddLiveKill(opd->virtual_register());
                }
            }
        }
    }
}

void RegisterAllocator::ComputeGlobalLiveSets() {
    for (int i = static_cast<int>(blocks_.size()) - 1; i >= 0; i--) {
        auto block = blocks_[i];
        auto block_state = &blocks_liveness_state_[i];
        
        block_state->ClearLiveOut();
        
        for (auto succ : block->successors()) {

            auto succ_state = BlockLivenssStateOf(succ);
            block_state->Union(block_state->mutable_live_out(), succ_state->live_in());
        }
        
        block_state->Subtract(block_state->mutable_live_in(), block_state->live_out(), block_state->live_kill());
        block_state->Union(block_state->mutable_live_in(), block_state->live_gen());
    }
}

void RegisterAllocator::BuildIntervals() {
    auto size = fun_->frame()->virtual_registers_size();
    size += regconf_->max_gp_register();
    size += regconf_->max_fp_register();
    intervals_.resize(size , nullptr);
    
    for (int i = static_cast<int>(blocks_.size()) - 1; i >= 0; i--) {
        auto block = blocks_[i];
        auto block_state = &blocks_liveness_state_[i];
        
        auto local_from = block->instructions().front()->id();
        auto local_to   = block->instructions().back()->id();
        
        BlockLivenssState::Walk(block_state->live_out(), [this, local_from, local_to](int virtual_register) {
            IntervalOfVR(virtual_register)->AddRange(local_from, local_to);
        });
        
        for (int j = static_cast<int>(block->instructions_size()) - 1; j >= 0; j--) {
            auto instr = block->instruction(j);
            
            // TODO: process calling
            
            for (int k = 0; k < instr->outputs_count(); k++) {
                if (auto opd = instr->OutputAt(k)->AsUnallocated()) {
                    auto interval = IntervalOf(opd);
                    interval->TouchFirstRange()->from = instr->id();
                    interval->AddUsePosition(instr->id(), opd->policy()/*used_kind*/);
                    
                    if (opd->has_fixed_reigster_policy()) {
                        interval = IntervalOfGP(opd->fixed_register_id());
                        interval->TouchFirstRange()->from = instr->id();
                        interval->AddUsePosition(instr->id(), opd->policy()/*used_kind*/);
                    }
                    
                    if (opd->has_fixed_fp_reigster_policy()) {
                        interval = IntervalOfFP(opd->fixed_fp_register_id());
                        interval->TouchFirstRange()->from = instr->id();
                        interval->AddUsePosition(instr->id(), opd->policy()/*used_kind*/);
                    }
                }
            }
            
            for (int k = 0; k < instr->temps_count(); k++) {
                if (auto opd = instr->TempAt(k)->AsUnallocated()) {
                    auto interval = IntervalOf(opd);
                    interval->AddRange(instr->id(), instr->id() + 1);
                    interval->AddUsePosition(instr->id(), opd->policy()/*used_kind*/);
                    
                    if (opd->has_fixed_reigster_policy()) {
                        interval = IntervalOfGP(opd->fixed_register_id());
                        interval->AddRange(instr->id(), instr->id() + 1);
                        interval->AddUsePosition(instr->id(), opd->policy()/*used_kind*/);
                    }
                    
                    if (opd->has_fixed_fp_reigster_policy()) {
                        interval = IntervalOfFP(opd->fixed_fp_register_id());
                        interval->AddRange(instr->id(), instr->id() + 1);
                        interval->AddUsePosition(instr->id(), opd->policy()/*used_kind*/);
                    }
                }
            }
            
            for (int k = 0; k < instr->inputs_count(); k++) {
                if (auto opd = instr->InputAt(k)->AsUnallocated()) {
                    auto interval = IntervalOf(opd);
                    interval->AddRange(local_from, instr->id());
                    interval->AddUsePosition(instr->id(), opd->policy()/*used_kind*/);
                    
                    if (opd->has_fixed_reigster_policy()) {
                        interval = IntervalOfGP(opd->fixed_register_id());
                        interval->AddRange(local_from, instr->id());
                        interval->AddUsePosition(instr->id(), opd->policy()/*used_kind*/);
                    }
                    
                    if (opd->has_fixed_fp_reigster_policy()) {
                        interval = IntervalOfFP(opd->fixed_fp_register_id());
                        interval->AddRange(local_from, instr->id());
                        interval->AddUsePosition(instr->id(), opd->policy()/*used_kind*/);
                    }
                }
            }
        }
    }
}

void RegisterAllocator::WalkIntervals() {
    LivenessIntervalSet unhanded, active, inactive;
    for (int i = 0; i < fun_->frame()->virtual_registers_size(); i++) {
        if (!intervals_[i]) {
            continue;
        }
        unhanded.insert(intervals_[i]);
    }
    
    
    // note: new intervals may be sorted into the unhandled list during
    // allocation when intervals are split
    while (!unhanded.empty()) {
        auto current = *unhanded.begin();
        unhanded.erase(unhanded.begin());
        //const auto position = current->first_range().from;
        const auto position = current->earliest_range().from;
        
        // check for intervals in active that are expired or inactive
        std::vector<LivenessInterval *> incoming_removed;
        for (auto it : active) {
            if (it->latest_range().to < position) {
                incoming_removed.push_back(it);
                //unhanded.insert(it);
            } else if (it->DoesNotCovers(position)) {
                incoming_removed.push_back(it);
                inactive.insert(it);
            }
        }
        for (auto it : incoming_removed) {
            active.erase(it);
        }
        incoming_removed.clear();
        
        // check for intervals in inactive that are expired or active
        for (auto it : inactive) {
            if (it->latest_range().to < position) {
                incoming_removed.push_back(it);
                //unhanded.insert(it);
            } else if (it->DoesCovers(position)) {
                incoming_removed.push_back(it);
                active.insert(it);
            }
        }
        for (auto it : incoming_removed) {
            inactive.erase(it);
        }
        incoming_removed.clear();
        
        
        // find a register for current
        if (!TryAllocateFreeRegister(current, active, inactive)) {
            // fail to allocation
            // TODO:
            UNREACHABLE();
        }

        if (current->has_assigned_any_register()) {
            active.insert(current);
        }
    }
}

static inline std::tuple<int, int> GetHighest(const std::vector<int> &free_reg_position) {
    int highest = 0;
    int reg = 0;
    for (int i = 0; i < free_reg_position.size(); i++) {
        if (free_reg_position[i] > highest) {
            highest = free_reg_position[i];
            reg = i;
            break;
        }
    }
    return std::make_tuple(reg, highest);
}

bool RegisterAllocator::TryAllocateFreeRegister(LivenessInterval *current, const LivenessIntervalSet &active,
                                                const LivenessIntervalSet &inactive) {
    //const auto max_register_slots = regconf_->max_fp_register() + regconf_->max_gp_register();
    std::vector<int> free_gp_position(regconf_->max_gp_register(), std::numeric_limits<int>::max());
    std::vector<int> free_fp_position(regconf_->max_fp_register(), std::numeric_limits<int>::max());
    
    for (auto it : active) {
        DCHECK(it->has_assigned_any_register());
        if (it->has_assigned_gp_register()) {
            free_gp_position[it->assigned_register()] = 0;
        } else {
            free_fp_position[it->assigned_register()] = 0;
        }
    }
    
    for (auto it : inactive) {
        if (it->DoesNotIntersects(current)) {
            continue;
        }
        DCHECK(it->has_assigned_any_register());
        
        // TODO:
        UNREACHABLE();
    }
    
    auto [reg, pos] = GetHighest(current->should_gp_register() ? free_gp_position: free_fp_position);
    if (pos == 0) {
        return false; // allocate fail
    }

    if (pos > current->last_range().to) {
        // register available for whole current
        current->AssignRegister(reg);
    } else {
        // register available for first part of current
        current->AssignRegister(reg);
        // split
        UNREACHABLE();
    }
    return true;
}

LivenessInterval *RegisterAllocator::IntervalOf(UnallocatedOperand *opd) {
    return IntervalOfVR(opd->virtual_register());
}

LivenessInterval *RegisterAllocator::IntervalOf(ir::Value *value) {
    auto vr = fun_->frame()->GetVirtualRegister(value);
    return IntervalOfVR(vr);
}

LivenessInterval *RegisterAllocator::IntervalOfVR(int virtual_register) {
    DCHECK(virtual_register >= 0);
    DCHECK(virtual_register < fun_->frame()->virtual_registers_size());
    return IntervalOfIndex(virtual_register);
}

LivenessInterval *RegisterAllocator::IntervalOfGP(int gp) {
    DCHECK(gp >= 0 && gp < regconf_->max_gp_register());
    return IntervalOfIndex(offset_of_gp_register() + gp);
}

LivenessInterval *RegisterAllocator::IntervalOfFP(int fp) {
    DCHECK(fp >= 0 && fp < regconf_->max_fp_register());
    return IntervalOfIndex(offset_of_fp_register() + fp);
}

LivenessInterval *RegisterAllocator::IntervalOfIndex(int index) {
    DCHECK(index >= 0);
    DCHECK(index < fun_->frame()->virtual_registers_size() + regconf_->max_gp_register() + regconf_->max_fp_register());
    if (intervals_[index]) {
        return intervals_[index];
    }

    if (index >= fun_->frame()->virtual_registers_size()) {
        LivenessInterval *rs = nullptr;
        if (index >= offset_of_gp_register() && index < offset_of_gp_register() + regconf_->max_gp_register()) {
            rs = new LivenessInterval(ir::Types::Word64); // TODO:
        } else {
            rs = new LivenessInterval(ir::Types::Float64); // TODO:
        }

        intervals_[index] = rs;
        return intervals_[index];
    }
    
    if (auto value = fun_->frame()->GetValue(index)) {
        intervals_[index] = new LivenessInterval(value->type());
    } else {
        intervals_[index] = new LivenessInterval(ir::Types::Word64); // TODO:
    }
    return intervals_[index];
}

int RegisterAllocator::offset_of_gp_register() const {
    return offset_of_virtual_register() + static_cast<int>(fun_->frame()->virtual_registers_size());
}

int RegisterAllocator::offset_of_fp_register() const {
    return offset_of_gp_register() + regconf_->max_gp_register();
}

} // namespace backend

} // namespace yalx

