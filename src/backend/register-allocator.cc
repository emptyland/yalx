#include "backend/register-allocator.h"
#include "backend/registers-configuration.h"
#include "backend/instruction.h"
#include "backend/frame.h"
#include "ir/metadata.h"
#include "ir/type.h"
#include "ir/node.h"

namespace yalx {

namespace backend {

void BlockLivenssState::Reset(size_t capacity) {
    live_kill_.clear();
    live_gen_.clear();
    live_in_.clear();
    live_out_.clear();
    capacity_ = capacity;
}

RegisterAllocator::RegisterAllocator(base::Arena *arena, const RegistersConfiguration *regconf, InstructionFunction *fun)
: arena_(arena)
, regconf_(regconf)
, fun_(fun) {
}

RegisterAllocator::~RegisterAllocator() {

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


} // namespace backend

} // namespace yalx

