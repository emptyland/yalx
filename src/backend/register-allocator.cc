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

LifetimeInterval::LifetimeInterval(int virtual_register, ir::Type type)
: virtual_register_(virtual_register)
, type_(type)
, rep_(ToMachineRepresentation(type)) {
}

LifetimeInterval::~LifetimeInterval() {
    
}

void LifetimeInterval::AddChild(LifetimeInterval *child, int pos) {
    DCHECK(std::find(split_children_.begin(), split_children_.end(), child) == split_children_.end());
    if (split_parent_) {
        child->split_parent_ = split_parent_;
    } else {
        child->split_parent_ = this;
    }
    child->split_parent_->split_children_.push_back(child);
    
    if (has_any_used()) {
        while (use_positions_.front().position >= pos) {
            child->use_positions_.push_back(use_positions_.front());
            use_positions_.erase(use_positions_.begin());
        }
    }
    
    // [0, 4) [4, 10) [10, 20)
    //           6
    //
    while (ranges_.front().to >= pos) {
        auto range = ranges_.front();
        if (range.from > pos) {
            child->ranges_.push_back(range);
            ranges_.erase(ranges_.begin());
        } else if (range.from == pos) {
            child->ranges_.push_back(range);
            ranges_.erase(ranges_.begin());
            break;
        } else if (range.from <= pos && range.to > pos) {
            ranges_.front().to = pos;
            child->AddRange(pos, range.to);
            break;
        } else {
            //UNREACHABLE();
            break;
        }
    }
    DCHECK(!child->ranges_.empty());
}

const LifetimeInterval *LifetimeInterval::ChildAt(int opid) const {
    for (auto use : use_positions_) {
        if (use.position == opid) {
            return this;
        }
    }
    for (auto child : split_children_) {
        if (auto rs = child->ChildAt(opid)) {
            return rs;
        }
    }
    UNREACHABLE();
    return nullptr;
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
    std::vector<bool> visited(fun_->blocks_size(), false);
    ComputeBlocksLoop(&visited, fun_->entry());
}

void RegisterAllocator::ComputeBlocksLoop(std::vector<bool> *visited, InstructionBlock *block) {
    DCHECK(block->id() >= 0 && block->id() < visited->size());
    (*visited)[block->id()] = true;
    for (auto succ : block->successors()) {
        if ((*visited)[succ->id()]) {
            succ->AddLoopEnd(block);
        } else {
            ComputeBlocksLoop(visited, succ);
        }
    }
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
        
        
        for (auto instr : block->instructions()) {
            if (auto moves = instr->mutable_parallel_move(Instruction::kStart)) {
                for (auto opds : moves->moves()) {
                    //opds->dest()
                    if (auto opd = opds->src().AsUnallocated()) {
                        if (state->DoesNotInLiveKill(opd->virtual_register())) {
                            state->AddLiveGen(opd->virtual_register());
                        }
                    }
                    auto opd = opds->dest().AsUnallocated();
                    state->AddLiveKill(opd->virtual_register());
                }
            }

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
            
            if (auto moves = instr->mutable_parallel_move(Instruction::kEnd)) {
                for (auto opds : moves->moves()) {
                    //opds->dest()
                    if (auto opd = opds->src().AsUnallocated()) {
                        if (state->DoesNotInLiveKill(opd->virtual_register())) {
                            state->AddLiveGen(opd->virtual_register());
                        }
                    }
                    auto opd = opds->dest().AsUnallocated();
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
    intervals_.resize(fun_->frame()->virtual_registers_size() , nullptr);
    
    for (int i = static_cast<int>(blocks_.size()) - 1; i >= 0; i--) {
        auto block = blocks_[i];
        auto block_state = &blocks_liveness_state_[i];
        
        auto local_from = block->GetLowerId();
        auto local_to   = block->GetUpperId() + 2;
        
        BlockLivenssState::Walk(block_state->live_out(), [this, local_from, local_to](int virtual_register) {
            IntervalOf(virtual_register)->AddRange(local_from, local_to);
        });
        
        for (int j = static_cast<int>(block->instructions_size()) - 1; j >= 0; j--) {
            auto instr = block->instruction(j);
            
            if (auto moves = instr->mutable_parallel_move(Instruction::kStart)) {
                for (auto opds : moves->moves()) {
                    if (auto opd = opds->mutable_dest()->AsUnallocated()) {
                        auto interval = IntervalOf(opd);
                        interval->TouchEarliestRange(instr->id());

                        AddUsePosition(instr->id(), interval, opd);
                    }
                    if (auto opd = opds->mutable_src()->AsUnallocated()) {
                        auto interval = IntervalOf(opd);
                        interval->AddRange(local_from, instr->id());
                        AddUsePosition(instr->id(), interval, opd);
                    }
                }
            }

            for (int k = 0; k < instr->outputs_count(); k++) {
                if (auto opd = instr->OutputAt(k)->AsUnallocated()) {
                    auto interval = IntervalOf(opd);
                    interval->TouchEarliestRange(instr->id());

                    AddUsePosition(instr->id(), interval, opd);
                }
            }
            
            for (int k = 0; k < instr->temps_count(); k++) {
                if (auto opd = instr->TempAt(k)->AsUnallocated()) {
                    auto interval = IntervalOf(opd);
                    interval->AddRange(instr->id(), instr->id() + 1);
                    AddUsePosition(instr->id(), interval, opd);
                }
            }
            
            for (int k = 0; k < instr->inputs_count(); k++) {
                if (auto opd = instr->InputAt(k)->AsUnallocated()) {
                    auto interval = IntervalOf(opd);
                    auto upper_id = block->loop_end_nodes().empty()
                                  ? instr->id()
                                  : local_to;
                    interval->AddRange(local_from, upper_id);
                    AddUsePosition(instr->id(), interval, opd);
                }
            }
        }
    }
}

void RegisterAllocator::WalkIntervals() {
    LifetimeIntervalSet unhanded, active, inactive;
    for (int i = 0; i < fun_->frame()->virtual_registers_size(); i++) {
        if (!intervals_[i]) {
            continue;
        }
        unhanded.insert(intervals_[i]);
    }
    
    // process paramater first;
    DCHECK_NOTNULL(fun_->block(0));
    if (fun_->block(0)->instruction(0)->op() == ArchFrameEnter) {
        auto instr = fun_->block(0)->instruction(0);
        for (int i = 0; i < instr->outputs_count(); i++) {
            auto opd = DCHECK_NOTNULL(instr->OutputAt(i)->AsUnallocated());
            LifetimeInterval *interval = nullptr;
            if (opd->policy() == UnallocatedOperand::kFixedRegister) {
                interval = IntervalOf(opd);
                interval->AssignRegister(opd->fixed_register_id());
            } else if (opd->policy() == UnallocatedOperand::kFixedFPRegister) {
                interval = IntervalOf(opd);
                interval->AssignRegister(opd->fixed_fp_register_id());
            }
            if (interval) {
                Remove(&unhanded, interval);
                active.insert(interval);
            }
        }
    }
    
//    std::vector<LifetimeInterval *> splitted;
//    for (auto it : unhanded) {
//        SplitByUsePolicy(it, &splitted);
//    }
//    for (auto it : splitted) {
//        unhanded.insert(it);
//    }
    
    // note: new intervals may be sorted into the unhandled list during
    // allocation when intervals are split
    while (!unhanded.empty()) {
        auto current = *unhanded.begin();
        unhanded.erase(unhanded.begin());
        //const auto position = current->first_range().from;
        const auto position = current->earliest_range().from;
        
        // check for intervals in active that are expired or inactive
        std::vector<LifetimeInterval *> incoming_remove;
        for (auto it : active) {
            if (it->latest_range().to < position) {
                incoming_remove.push_back(it);
            } else if (it->IsNotCovers(position)) {
                incoming_remove.push_back(it);
                inactive.insert(it);
            }
        }
        RemoveAll(&active, std::move(incoming_remove));
        
        // check for intervals in inactive that are expired or active
        for (auto it : inactive) {
            if (it->latest_range().to < position) {
                incoming_remove.push_back(it);
            } else if (it->IsCovers(position)) {
                incoming_remove.push_back(it);
                active.insert(it);
            }
        }
        RemoveAll(&inactive, std::move(incoming_remove));
        
        
        // find a register for current
        if (!TryAllocateFreeRegister(current, &unhanded, active, inactive)) {
            // fail to allocation
            AllocateBlockedRegister(current, &unhanded, active, inactive);
        }

        if (current->has_assigned_any_register()) {
            active.insert(current);
        }
        
        if (!active.empty()) {
            printf("------\n");
            for (auto it : active) {
                printf("%d->%d, ", it->virtual_register(), it->assigned_operand());
            }
            printf("\n");
        }
    }
}

void RegisterAllocator::AssignRegisters() {
    for (auto block : blocks_) {
        for (auto instr : block->instructions()) {
            for (int i = 0; i < 2; i++) {
                if (auto moves = instr->mutable_parallel_move(static_cast<Instruction::GapPosition>(i))) {
                    for (auto opds : moves->moves()) {
                        if (auto opd = opds->mutable_src()->AsUnallocated()) {
                            AssignOperand(instr->id(), opds->mutable_src(), opd);
                        }
                        if (auto opd = opds->mutable_dest()->AsUnallocated()) {
                            AssignOperand(instr->id(), opds->mutable_dest(), opd);
                        }
                    }
                }
            }
            
            for (int i = 0; i < instr->operands_size(); i++) {
                if (auto opd = instr->OperandAt(i)->AsUnallocated()) {
                    AssignOperand(instr->id(), instr->OperandAt(i), opd);
                }
            }
        }
    }
}

static inline std::tuple<int, int> GetHighest(const std::vector<int> &free_reg_position,
                                              const std::vector<bool> &bitmap) {
    int highest = 0;
    int reg = 0;
    for (int i = 0; i < free_reg_position.size(); i++) {
        if (!bitmap[i]) {
            continue;
        }
        if (free_reg_position[i] > highest) {
            highest = free_reg_position[i];
            reg = i;
            //break;
        }
    }
    return std::make_tuple(reg, highest);
}

void RegisterAllocator::AddUsePosition(int pos, LifetimeInterval *interval, const UnallocatedOperand *opd) {
    int hint = 0;
    if (opd->has_fixed_reigster_policy()) {
        hint = opd->fixed_register_id();
    }
    
    if (opd->has_fixed_fp_reigster_policy()) {
        hint = opd->fixed_fp_register_id();
    }
    
    if (opd->has_fixed_slot_policy()) {
        hint = opd->fixed_slot_offset();
    }
    
    interval->AddUsePosition(pos, opd->policy()/*used_kind*/, hint);
}

void RegisterAllocator::SplitByUsePolicy(LifetimeInterval *current, std::vector<LifetimeInterval *> *splitted) {
    if (current->has_never_used()) {
        return;
    }
    
    auto latest_policy = static_cast<UnallocatedOperand::Policy>(current->earliest_use_position().use_kind);
    auto latest_hint = current->earliest_use_position().hint;
    for (int i = static_cast<int>(current->use_positions().size()) - 1; i >= 0; i--) {
        auto use = current->use_positions()[i];
        auto policy = static_cast<UnallocatedOperand::Policy>(use.use_kind);
        auto hint = use.hint;
        
        if (ShouldSplitByUsePolicy(latest_policy, latest_hint, policy, hint)) {
            //DCHECK(current->IsCovers(use.position));
            splitted->push_back(SplitInterval(current, use.position - 1));
        }
        latest_policy = policy;
        latest_hint   = hint;
    }
}

bool RegisterAllocator::ShouldSplitByUsePolicy(int policy0, int latest_hint, int policy1, int hint) {
    auto latest_policy = static_cast<UnallocatedOperand::Policy>(policy0);
    auto policy = static_cast<UnallocatedOperand::Policy>(policy1);
    
    switch (latest_policy) {
        case UnallocatedOperand::kRegisterOrSlot:
        case UnallocatedOperand::kRegisterOrSlotOrConstant:
            return policy != UnallocatedOperand::kRegisterOrSlot &&
                   policy != UnallocatedOperand::kRegisterOrSlotOrConstant;
        case UnallocatedOperand::kFixedSlot:
            return policy != UnallocatedOperand::kFixedSlot && policy != UnallocatedOperand::kMustHaveSlot;
        case UnallocatedOperand::kFixedRegister:
            return policy != UnallocatedOperand::kFixedRegister || hint != latest_hint;
        case UnallocatedOperand::kFixedFPRegister:
            return policy != UnallocatedOperand::kFixedFPRegister || hint != latest_hint;
        case UnallocatedOperand::kMustHaveSlot:
            return policy != UnallocatedOperand::kMustHaveSlot && policy != UnallocatedOperand::kFixedSlot;
        case UnallocatedOperand::kMustHaveRegister:
            return policy != UnallocatedOperand::kMustHaveRegister &&
                   policy != UnallocatedOperand::kRegisterOrSlot &&
                   policy != UnallocatedOperand::kRegisterOrSlotOrConstant;
        case UnallocatedOperand::kNone:
        default:
            UNREACHABLE();
            break;
    }
    return false;
}

bool RegisterAllocator::TryAllocateFreeRegister(LifetimeInterval *current,
                                                LifetimeIntervalSet *unhandled,
                                                const LifetimeIntervalSet &active,
                                                const LifetimeIntervalSet &inactive) {
    UnallocatedOperand::Policy policy = UnallocatedOperand::kNone;
    int hint = 0;
    if (current->has_any_used()) {
        auto use = current->earliest_use_position();
        policy = static_cast<UnallocatedOperand::Policy>(use.use_kind);
        hint = use.hint;

        switch (policy) {
            case UnallocatedOperand::kFixedSlot:
                current->AssignSlot(hint);
                return true;
            case UnallocatedOperand::kMustHaveSlot: {
                auto value = fun_->frame()->GetValue(current->GetOriginalVR());
                current->AssignSlot(fun_->frame()->AllocateSlot(value->type().ReferenceSizeInBytes(), 0));
            } return true;

            default:
                break;
        }
    }

    std::vector<int> free_gp_position(regconf_->max_gp_register(), std::numeric_limits<int>::max());
    std::vector<int> free_fp_position(regconf_->max_fp_register(), std::numeric_limits<int>::max());
    
    for (auto it : active) {
        DCHECK(it->has_assigned_any_register());
        if (it->has_assigned_gp_register()) {
            free_gp_position[it->assigned_operand()] = 0;
        } else {
            free_fp_position[it->assigned_operand()] = 0;
        }
    }

    for (auto it : inactive) {
        auto rs = it->GetIntersection(current);
        if (rs.from < 0 && rs.to < 0) {
            continue;
        }
        DCHECK(it->has_assigned_any_register());
        
        if (it->has_assigned_gp_register()) {
            free_gp_position[it->assigned_operand()] = rs.from;
        } else{
            free_fp_position[it->assigned_operand()] = rs.from;
        }
    }
    
    int reg = -1, pos = -1;
    switch (policy) {
        case UnallocatedOperand::kFixedRegister:
            reg = hint;
            pos = free_gp_position[reg];
            break;
        case UnallocatedOperand::kFixedFPRegister:
            reg = hint;
            pos = free_fp_position[reg];
            break;
        case UnallocatedOperand::kNone:
        case UnallocatedOperand::kMustHaveRegister:
        case UnallocatedOperand::kRegisterOrSlot:
        case UnallocatedOperand::kRegisterOrSlotOrConstant: {
                auto rs = GetHighest(current->should_gp_register()
                                             ? free_gp_position
                                             : free_fp_position,
                                             current->should_gp_register()
                                             ? regconf_->allocatable_gp_bitmap()
                                             : regconf_->allocatable_fp_bitmap());
                reg = std::get<0>(rs);
                pos = std::get<1>(rs);
            } break;
        default:
            UNREACHABLE();
            break;
    }

    if (pos == 0) {
        return false; // allocate fail
    }

    if (pos > current->latest_range().to) {
        DCHECK(current->IsNotCovers(pos));
        // register available for whole current
        current->AssignRegister(reg);
    } else {
        DCHECK(current->IsCovers(pos));
        // register available for first part of current
        current->AssignRegister(reg);
        // split
        unhandled->insert(SplitInterval(current, pos));
    }
    return true;
}

void RegisterAllocator::AllocateBlockedRegister(LifetimeInterval *current, LifetimeIntervalSet *unhandled,
                                                const LifetimeIntervalSet &active,
                                                const LifetimeIntervalSet &inactive) {
    std::vector<int> use_gp_position(regconf_->max_gp_register(), std::numeric_limits<int>::max());
    std::vector<int> use_fp_position(regconf_->max_fp_register(), std::numeric_limits<int>::max());

    for (auto it : active) {
        DCHECK(it->has_assigned_any_register());
        auto use = it->FindUsePositionAfter(current->earliest_range().from);
        if (it->has_assigned_gp_register()) {
            use_gp_position[it->assigned_operand()] = use.position;
        } else {
            use_fp_position[it->assigned_operand()] = use.position;
        }
    }
    
    for (auto it : inactive) {
        DCHECK(it->has_assigned_any_register());
        if (it->IsNotIntersects(current)) {
            continue;
        }
        auto use = it->FindUsePositionAfter(current->earliest_range().from);
        if (it->has_assigned_gp_register()) {
            use_gp_position[it->assigned_operand()] = use.position;
        } else {
            use_fp_position[it->assigned_operand()] = use.position;
        }
    }
    
    auto [reg, pos] = GetHighest(current->should_gp_register()
                                 ? use_gp_position
                                 : use_fp_position,
                                 current->should_gp_register()
                                 ? regconf_->allocatable_gp_bitmap()
                                 : regconf_->allocatable_fp_bitmap());
    if (pos < current->earliest_use_position().position) {
        // all active and inactive intervals are used before current, so it is best to spill current itself
        auto ty = fun_->frame()->GetType(current->GetOriginalVR());
        current->AssignSlot(fun_->frame()->AllocateSlot(ty.ReferenceSizeInBytes(), 0));
        unhandled->insert(SplitInterval(current, current->earliest_use_position().position + 1));
    } else {
        // spilling made a register free for first part of current
        current->AssignRegister(reg);
        UNREACHABLE();
        unhandled->insert(SplitInterval(current, pos));
    }
}

// split interval and returning child
LifetimeInterval *RegisterAllocator::SplitInterval(LifetimeInterval *interval, int pos) {
    //printd("%p, %d: %d", interval, interval->virtual_register(), pos);
    auto vr = static_cast<int>(intervals_.size());
    auto child = new LifetimeInterval(vr, interval->type());
    intervals_.push_back(child);
    interval->AddChild(child, pos);
    return child;
}

void RegisterAllocator::AssignOperand(int opid, InstructionOperand *receiver, UnallocatedOperand *unalloc) {
    auto interval = IntervalOf(unalloc->virtual_register())->ChildAt(opid);
    switch (unalloc->policy()) {
        case UnallocatedOperand::kRegisterOrSlot:
        case UnallocatedOperand::kRegisterOrSlotOrConstant: {
            DCHECK(interval->has_any_assinged());
            *receiver = AllocatedOperand(interval->has_assigned_any_register()
                                         ? AllocatedOperand::kRegister
                                         : AllocatedOperand::kSlot,
                                         interval->representation(),
                                         interval->assigned_operand());
        } break;
        case UnallocatedOperand::kMustHaveRegister: {
            DCHECK(interval->has_assigned_any_register());
            *receiver = AllocatedOperand(AllocatedOperand::kRegister, interval->representation(),
                                         interval->assigned_operand());
        } break;
        case UnallocatedOperand::kMustHaveSlot:
            break;
        case UnallocatedOperand::kFixedSlot:
            *receiver = AllocatedOperand(AllocatedOperand::kSlot, interval->representation(),
                                         unalloc->fixed_slot_offset());
            break;
        case UnallocatedOperand::kFixedRegister:
            *receiver = AllocatedOperand(AllocatedOperand::kRegister, interval->representation(),
                                         unalloc->fixed_register_id());
            break;
        case UnallocatedOperand::kFixedFPRegister:
            *receiver = AllocatedOperand(AllocatedOperand::kRegister, interval->representation(),
                                         unalloc->fixed_fp_register_id());
            break;
            
        default:
            UNREACHABLE();
            break;
    }
}

LifetimeInterval *RegisterAllocator::IntervalOf(UnallocatedOperand *opd) {
    return IntervalOf(opd->virtual_register());
}

LifetimeInterval *RegisterAllocator::IntervalOf(ir::Value *value) {
    auto vr = fun_->frame()->GetVirtualRegister(value);
    return IntervalOf(vr);
}

LifetimeInterval *RegisterAllocator::IntervalOf(int virtual_register) {
    DCHECK(virtual_register >= 0);
    DCHECK(virtual_register < intervals_.size());
    if (intervals_[virtual_register]) {
        return intervals_[virtual_register];
    }
    
    if (auto value = fun_->frame()->GetValue(virtual_register)) {
        intervals_[virtual_register] = new LifetimeInterval(virtual_register, value->type());
    } else {
        intervals_[virtual_register] = new LifetimeInterval(virtual_register, ir::Types::Word64); // TODO:
    }
    return intervals_[virtual_register];
}

int RegisterAllocator::offset_of_gp_register() const {
    return offset_of_virtual_register() + static_cast<int>(fun_->frame()->virtual_registers_size());
}

int RegisterAllocator::offset_of_fp_register() const {
    return offset_of_gp_register() + regconf_->max_gp_register();
}

} // namespace backend

} // namespace yalx

