#pragma once
#ifndef YALX_BACKEND_REGISTER_ALLOCATOR_H_
#define YALX_BACKEND_REGISTER_ALLOCATOR_H_

#include "backend/machine-type.h"
#include "ir/type.h"
#include "base/checking.h"
#include "base/base.h"
#include <numeric>
#include <vector>
#include <set>

namespace yalx {
namespace base {
class Arena;
} // namespace base
namespace ir {
class Function;
class Value;
} // namespace ir
namespace backend {


class UnallocatedOperand;
class InstructionOperand;
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


class LifetimeInterval final {
public:
    enum Status {
        kUnassigned,
        kRegisterAssigned,
        kSlotAssigned,
    };
    
    struct Range {
        int from;
        int to;
        
        //a [from to)
        //b     [from to)
        //
        //a     [from to)
        //b [from to)
        bool IsIntersects(Range rhs) const { return to > rhs.from && from < rhs.to; }
        
        Range Intersects(Range rhs) const {
            if (!IsIntersects(rhs)) {
                return {-1,-1};
            }
            if (from <= rhs.from) {
                return {rhs.from, to};
            }
            if (from >= rhs.from) {
                return {from, rhs.to};
            }
            UNREACHABLE();
        }
    };
    
    struct UsePosition {
        int position;
        int use_kind;
        int hint;
    };
    
    LifetimeInterval(int virtual_register, ir::Type type);
    ~LifetimeInterval();
    
    DEF_VAL_GETTER(int, virtual_register);
    
    MachineRepresentation representation() const { return rep_; }
    
    bool should_fp_register() const {
        return (rep_ == MachineRepresentation::kFloat32 || rep_ == MachineRepresentation::kFloat64);
    }
    bool should_gp_register() const { return !should_fp_register(); }

    bool has_assigned_fp_register() const { return has_assigned_any_register() && should_fp_register(); }
    bool has_assigned_gp_register() const { return has_assigned_any_register() && should_gp_register(); }
    
    bool has_assigned_any_register() const { return status() == kRegisterAssigned; }
    bool has_assigned_slot() const { return status() == kSlotAssigned; }
    bool has_any_assinged() const { return has_assigned_slot() || has_assigned_any_register(); }
    
    bool has_any_used() const { return !use_positions_.empty(); }
    bool has_never_used() const { return use_positions_.empty(); }
    
    DEF_VAL_GETTER(ir::Type, type);
    DEF_VAL_GETTER(Status, status);
    DEF_VAL_GETTER(int, assigned_operand);
    DEF_VAL_GETTER(std::vector<UsePosition>, use_positions);
    
    const LifetimeInterval *ChildAt(int opid) const;
    
    int GetOriginalVR() const {
        return split_parent_ ? split_parent_->virtual_register() : virtual_register();
    }
    
    void AssignRegister(int reg) {
        DCHECK(!has_any_assinged());
        assigned_operand_ = reg;
        status_ = kRegisterAssigned;
    }
    
    void AssignSlot(int offset) {
        DCHECK(!has_any_assinged());
        assigned_operand_ = offset;
        status_ = kSlotAssigned;
    }
    
    const UsePosition &earliest_use_position() const {
        DCHECK(!use_positions_.empty());
        return use_positions_.back();
    }
    
    const UsePosition &latest_use_position() const {
        DCHECK(!use_positions_.empty());
        return use_positions_.front();
    }
    
    const Range &earliest_range() const {
        DCHECK(!ranges_.empty());
        return ranges_.back();
    }
    const Range &latest_range() const {
        DCHECK(!ranges_.empty());
        return ranges_.front();
    }

    Range *TouchEarliestRange(int pos) {
        if (ranges_.empty()) {
            AddRange(pos, pos);
        }
        return &ranges_.back();
    }
    
    const Range &range(size_t i) const {
        DCHECK(i < ranges_.size());
        return ranges_[i];
    }
    
    bool IsNotCovers(int position) const { return !IsCovers(position); }
    bool IsCovers(int position) const {
        for (auto range : ranges_) {
            if (position == range.from) {
                return true;
            }
            if (position > range.from && position < range.to) {
                return true;
            }
        }
        for (auto child : split_children_) {
            if (child->IsCovers(position)) {
                return true;
            }
        }
        return false;
    }
    
    bool IsNotIntersects(const LifetimeInterval *it) const { return !IsIntersects(it); }
    
    bool IsIntersects(const LifetimeInterval *it) const {
        for (auto rhs : it->ranges_) {
            for (auto lhs : ranges_) {
                if (lhs.IsIntersects(rhs)) {
                    return true;
                }
            }
        }
        return false;
    }
    
    Range GetIntersection(const LifetimeInterval *it) const {
        for (auto rhs : it->ranges_) {
            for (auto lhs : ranges_) {
                if (auto rs = lhs.Intersects(rhs); rs.from >= 0 && rs.to >= 0) {
                    return rs;
                }
            }
        }
        return {-1,-1};
    }
    
    void AddRange(int from, int to) {
        for (auto range : ranges_) {
            if (from >= range.from && to <= range.to) {
                return;
            }
        }
        ranges_.push_back({from, to});
    }

    void AddUsePosition(int position, int used_kind, int hint = 0) {
        use_positions_.push_back({position, used_kind, hint});
    }
    
    UsePosition FindUsePositionAfter(int pos) const {
        for (int i = static_cast<int>(use_positions_.size()) - 1; i >= 0; i--) {
            auto use_position = use_positions_[i];
            if (use_position.position > pos) {
                return use_position;
            }
        }
        return use_positions_[0];
    }
    
    void AddChild(LifetimeInterval *child, int pos);
private:
    int const virtual_register_;
    ir::Type const type_;
    MachineRepresentation const rep_;
    Status status_ = kUnassigned;
    int assigned_operand_ = 0;
    LifetimeInterval *split_parent_ = nullptr;
    std::vector<LifetimeInterval *> split_children_;
    std::vector<Range> ranges_;
    std::vector<UsePosition> use_positions_;
}; // class LifetimeInterval

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
    // step 6
    void WalkIntervals();
    // step 7
    void AssignRegisters();

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
    
    
    LifetimeInterval *IntervalOf(UnallocatedOperand *opd);
    LifetimeInterval *IntervalOf(ir::Value *value);
    LifetimeInterval *IntervalOf(int virtual_register);
private:
    struct LifetimeIntervalComparator : public std::binary_function<LifetimeInterval *, LifetimeInterval *, bool> {
        bool operator() (LifetimeInterval *a, LifetimeInterval *b) const {
            if (a->earliest_range().from == b->earliest_range().from) {
                return a->latest_range().to >= b->latest_range().to;
            } else {
                return a->earliest_range().from < b->earliest_range().from;
            }
        }
    };

    using LifetimeIntervalSet = std::set<LifetimeInterval *, LifetimeIntervalComparator>;
    
    static void RemoveAll(LifetimeIntervalSet *set, std::vector<LifetimeInterval *> &&incoming) {
        for (auto it : incoming) { Remove(set, it); }
        incoming.clear();
    }
    
    static void Remove(LifetimeIntervalSet *set, LifetimeInterval *incoming) {
        for (auto iter = set->begin(); iter != set->end(); iter++) {
            if (*iter == incoming) {
                set->erase(iter);
                return;
            }
        }
    }
    
    int offset_of_virtual_register() const { return 0; }
    int offset_of_gp_register() const;
    int offset_of_fp_register() const;
    
    static void AddUsePosition(int pos, LifetimeInterval *interval, const UnallocatedOperand *opd);
    
    void ComputeBlocksLoop(std::vector<bool> *visited, InstructionBlock *block);
    
    void SplitByUsePolicy(LifetimeInterval *current, std::vector<LifetimeInterval *> *splitted);
    bool ShouldSplitByUsePolicy(int policy0, int hint0, int policy1, int hint1);
    bool TryAllocateFreeRegister(LifetimeInterval *current, LifetimeIntervalSet *unhandled,
                                 const LifetimeIntervalSet &active,
                                 const LifetimeIntervalSet &inactive);
    void AllocateBlockedRegister(LifetimeInterval *current, LifetimeIntervalSet *unhandled,
                                 const LifetimeIntervalSet &active,
                                 const LifetimeIntervalSet &inactive);
    
    LifetimeInterval *SplitInterval(LifetimeInterval *interval, int whit_opid);
    
    void AssignOperand(int opid, InstructionOperand *receiver, UnallocatedOperand *unalloc);
    
    base::Arena *const arena_;
    const RegistersConfiguration *const regconf_;
    InstructionFunction *const fun_;
    std::vector<InstructionBlock *> blocks_;
    std::vector<BlockLivenssState> blocks_liveness_state_;
    std::vector<LifetimeInterval *> intervals_;
}; // class RegisterAllocator

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_REGISTER_ALLOCATOR_H_
