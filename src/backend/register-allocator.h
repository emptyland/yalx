#pragma once
#ifndef YALX_BACKEND_REGISTER_ALLOCATOR_H_
#define YALX_BACKEND_REGISTER_ALLOCATOR_H_

#include "backend/machine-type.h"
#include "ir/type.h"
#include "base/checking.h"
#include "base/base.h"
#include <set>
#include <vector>

namespace yalx {
namespace base {
class Arena;
} // namespace base
namespace ir {
class Function;
} // namespace ir
namespace backend {


class UnallocatedOperand;
class InstructionFunction;
class InstructionBlock;
class RegistersConfiguration;
class Frame;

class BlockLivenssState {
public:
#define DECL_LIVENESS_KINDS(V) \
    V(Kill, kill) \
    V(Gen,  gen)  \
    V(In,   in)   \
    V(Out,  out)
    
    BlockLivenssState() {}
    
    void Reset(size_t capacity);

#define DEFINE_LIVENESS_OPS(kind, name) \
    const std::vector<bool> &live_##name() const { return live_##name##_; } \
    std::vector<bool> *mutable_live_##name() { return &live_##name##_; } \
    void set_live_##name(const std::vector<bool> &other) { live_##name##_ = other; } \
    bool DoesNotInLive##kind(int virtual_register) const { return !DoesIn(live_##name##_, virtual_register); } \
    bool DoesInLive##kind(int virtual_register) const { return DoesIn(live_##name##_, virtual_register); } \
    void AddLive##kind(int virtual_register) { Add(&live_##name##_, virtual_register); } \
    void RemoveLive##kind(int virtual_register) { Remove(&live_##name##_, virtual_register); } \
    void ClearLive##kind() { live_##name##_.clear(); }
    
    DECL_LIVENESS_KINDS(DEFINE_LIVENESS_OPS)

#undef DEFINE_LIVENESS_OPS
    
    void Union(std::vector<bool> *lhs, const std::vector<bool> &rhs) {
        for (int i = 0; i < rhs.size(); i++) {
            if (rhs[i]) {
                Add(lhs, i);
            }
        }
    }
    
    void Subtract(std::vector<bool> *receiver, const std::vector<bool> &lhs, const std::vector<bool> &rhs) {
        for (int i = 0; i < lhs.size(); i++) {
            if (lhs[i]) {
                Add(receiver, i);
            }
        }
        for (int i = 0; i < rhs.size(); i++) {
            if (rhs[i]) {
                Remove(receiver, i);
            }
        }
    }
    
    static void Walk(const std::vector<bool> &set, std::function<void (int)> &&callback) {
        for (int i = 0; i < set.size(); i++) {
            if (set[i]) {
                callback(i);
            }
        }
    }
    
private:
    bool DoesIn(const std::vector<bool> &set, int virtual_register) const {
        DCHECK(virtual_register >= 0);
        DCHECK(virtual_register < capacity_);
        if (virtual_register < set.size()) {
            return set[virtual_register];
        } else {
            return false;
        }
    }
    
    void Add(std::vector<bool> *set, int virtual_register) {
        DCHECK(virtual_register >= 0);
        DCHECK(virtual_register < capacity_);
        if (virtual_register >= set->size()) {
            set->resize(virtual_register + 1, false);
        }
        (*set)[virtual_register] = true;
    }
    
    void Remove(std::vector<bool> *set, int virtual_register) {
        DCHECK(virtual_register >= 0);
        DCHECK(virtual_register < capacity_);
        if (virtual_register < set->size()) {
            (*set)[virtual_register] = false;
        }
    }
    
    std::vector<bool> live_kill_;
    std::vector<bool> live_gen_;
    std::vector<bool> live_in_;
    std::vector<bool> live_out_;
    size_t capacity_ = 0;
};



class LivenessInterval final {
public:
    struct Range {
        int from;
        int to;
    };
    
    struct UsePosition {
        int position;
        int use_kind;
    };
    
    explicit LivenessInterval(ir::Type type);
    ~LivenessInterval();
    
    MachineRepresentation representation() const { return rep_; }
    
    Range *first_range() {
        DCHECK(!ranges_.empty());
        return &ranges_.front();
    }
    
    void AddRange(int from, int to) { ranges_.push_back({from, to}); }
    void AddUsePosition(int position, int used_kind) { user_positions_.push_back({position, used_kind}); }
private:
    ir::Type const type_;
    MachineRepresentation const rep_;
    LivenessInterval *split_parent_ = nullptr;
    std::vector<LivenessInterval *> split_children_;
    std::vector<Range> ranges_;
    std::vector<UsePosition> user_positions_;
}; // class LivenessInterval

class RegisterAllocator final {
public:
    RegisterAllocator(base::Arena *arena, const RegistersConfiguration *regconf, InstructionFunction *fun);
    ~RegisterAllocator();
    
    // step 0
    void Prepare();
    // step 1
    void ComputeBlocksOrder();
    // step 2
    void NumberizeAllInstructions();
    // step 3
    void ComputeLocalLiveSets();
    // step 4
    void ComputeGlobalLiveSets();
    // step 5
    void BuildIntervals();

    InstructionBlock *OrderedBlockAt(int i) const {
        DCHECK(i >= 0);
        DCHECK(i < blocks_.size());
        return blocks_[i];
    }
    
    size_t ordered_blocks_size() const { return blocks_.size(); }
    
    BlockLivenssState *BlockLivenssStateOf(InstructionBlock *block) {
        auto iter = std::find(blocks_.begin(), blocks_.end(), block);
        DCHECK(iter != blocks_.end());
        auto index = static_cast<int>(iter - blocks_.begin());
        DCHECK(index >= 0 && index < blocks_liveness_state_.size());
        return &blocks_liveness_state_[index];
    }
    
    
    LivenessInterval *IntervalOf(UnallocatedOperand *opd);
    LivenessInterval *IntervalOfVR(int virtual_register);
    LivenessInterval *IntervalOfGP(int gp);
    LivenessInterval *IntervalOfFP(int fp);
    LivenessInterval *IntervalOfIndex(int index);
private:
    base::Arena *const arena_;
    const RegistersConfiguration *const regconf_;
    InstructionFunction *const fun_;
    std::vector<InstructionBlock *> blocks_;
    std::vector<BlockLivenssState> blocks_liveness_state_;
    std::vector<LivenessInterval *> intervals_;
}; // class RegisterAllocator

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_REGISTER_ALLOCATOR_H_
