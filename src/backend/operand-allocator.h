#pragma once
#ifndef YALX_BACKEND_OPERAND_ALLOCATOR_H_
#define YALX_BACKEND_OPERAND_ALLOCATOR_H_

#include "backend/stackslot-allocator.h"
#include "backend/register-allocator.h"

namespace yalx {
namespace backend {

class OperandAllocator final {
public:
    enum Policy {
        kStackOnly,
        kRegisterFirst,
    };
    
    enum OperandMark {
        kVal,
        kF32,
        kF64,
        kPtr,
        kRef,
    };
    
    constexpr static const int kAnyRegister = RegisterAllocator::kAny;
    

    OperandAllocator(const StackConfiguration *sconf,
                     const RegisterConfiguration *rconf,
                     Policy policy,
                     base::Arena *arena);
    
    StackSlotAllocator *slots() { return &slots_; }
    RegisterAllocator *registers() { return &registers_; }
    
    void Prepare(ir::Function *fun);
    
    InstructionOperand *Allocated(ir::Value *value) {
        if (auto iter = allocated_.find(value); iter != allocated_.end()) {
            return iter->second;
        } else {
            return nullptr;
        }
    }
    
    InstructionOperand *Allocate(ir::Value *value);
    InstructionOperand *Allocate(ir::Type ty);
    InstructionOperand *Allocate(OperandMark mark, size_t size, ir::Model *model = nullptr);
    
    LocationOperand *AllocateStackSlot(ir::Value *value, size_t padding_size, StackSlotAllocator::Policy policy);
    LocationOperand *AllocateStackSlot(ir::Type ty, size_t padding_size, StackSlotAllocator::Policy policy);
    LocationOperand *AllocateStackSlot(OperandMark mark,
                                       size_t size,
                                       size_t padding_size,
                                       StackSlotAllocator::Policy policy,
                                       ir::Model *model = nullptr);
    
    RegisterOperand *AllocateReigster(ir::Value *value, int designate = kAnyRegister);
    RegisterOperand *AllocateReigster(ir::Type ty, int designate = kAnyRegister);
    RegisterOperand *AllocateReigster(OperandMark mark, size_t size, int designate = kAnyRegister);

    void Free(ir::Value *value) {
        if (auto iter = allocated_.find(value); iter != allocated_.end()) {
            Free(iter->second);
            allocated_.erase(iter);
        }
    }
    void Free(InstructionOperand *operand);
    
    bool WillBeLive(ir::Value *value, int position) const { return !WillBeDead(value, position); }
    bool WillBeDead(ir::Value *value, int position) const;
    
    void ReleaseDeads(int position);
    
    void Associate(ir::Value *value, InstructionOperand *operand);
    InstructionOperand *LinkTo(ir::Value *value, InstructionOperand *operand);
    
    struct BorrowedRecord {
        RegisterOperand *target;
        RegisterOperand *old;
        InstructionOperand *bak;
        ir::Value *original;
    };
    
    BorrowedRecord BorrowRegister(ir::Value *value, InstructionOperand *bak, int designate = kAnyRegister);
    BorrowedRecord BorrowRegister(ir::Type ty, InstructionOperand *bak, int designate = kAnyRegister);

    bool IsGeneralRegisterAlive(int id) const {
        return active_general_registers_.find(id) != active_general_registers_.end();
    }
    bool IsFloatRegisterAlive(int id) const {
        return active_float_registers_.find(id) != active_float_registers_.end();
    }
    
    struct LiveRange {
        int start_position = -1;
        int stop_position  = -1;
    };
    
    struct LiveRecord {
        size_t index;
        size_t size;
    };
    
    friend class RegisterSavingScope;
    friend class RegisterPreemptiveScope;
    DISALLOW_IMPLICIT_CONSTRUCTORS(OperandAllocator);
private:
    InstructionOperand *TryAllocateRegisterFirst(ir::Value *value);
    
    void Alive(ir::Value *value, int ir_position) {
        if (auto iter = live_ranges_.find(value); iter == live_ranges_.end()) {
            live_ranges_[value] = LiveRange{ir_position, -1};
        } else {
            live_ranges_[value].start_position = std::min(ir_position, live_ranges_[value].start_position);
        }
    }
    
    void Dead(ir::Value *value, int ir_position) {
        if (auto iter = live_ranges_.find(value); iter != live_ranges_.end()) {
            if (iter->second.stop_position == -1) {
                iter->second.stop_position = ir_position;
            }
        }
    }
    
    void DestinyDead(ir::Value *value, int ir_position) {
        if (auto iter = live_ranges_.find(value); iter != live_ranges_.end()) {
            if (iter->second.stop_position < ir_position) {
                iter->second.stop_position = ir_position;
            }
        }
    }
    
    struct RegisterRecord {
        RegisterOperand    *opd;
        InstructionOperand *bak;
        ir::Value *val;
    };

    const Policy policy_;
    base::Arena *const arena_;
    StackSlotAllocator slots_;
    RegisterAllocator registers_;
    std::map<int, RegisterRecord> active_general_registers_;
    std::map<int, RegisterRecord> active_float_registers_;
    std::map<ir::Value *, Instruction::Operand *> allocated_;
    std::map<ir::Value *, LiveRange> live_ranges_;
    std::vector<LiveRecord> dead_records_;
    std::vector<ir::Value *> deads_;
}; // class OperandAllocator

class RegisterSavingScope final {
public:
    //using MoveCallback = std::function<void(InstructionOperand *, InstructionOperand *, ir::Value *)>;
    
    class MovingDelegate {
    public:
        MovingDelegate() = default;
        virtual ~MovingDelegate() = default;
        virtual void MoveTo(InstructionOperand *, InstructionOperand *, ir::Type) = 0;
        virtual void Initialize() = 0;
        virtual void Finalize() = 0;
        DISALLOW_IMPLICIT_CONSTRUCTORS(MovingDelegate);
    }; // class MovingCallback
    
    RegisterSavingScope(OperandAllocator *allocator, int position, MovingDelegate *callback);
    ~RegisterSavingScope();
    
    void AddExclude(ir::Value *exclude, int designate, int position);

    bool Include(int designate, bool general) { return !Exclude(designate, general); }
    bool Exclude(int designate, bool general) {
        if (general) {
            return general_exclude_.find(designate) != general_exclude_.end();
        } else {
            return float_exclude_.find(designate) != float_exclude_.end();
        }
    }
    void SaveAll();
    
    bool HasSaved(int designate) {
        for (auto bak : backup_) {
            if (bak.old->register_id() == designate) {
                return true;
            }
        }
        return false;
    }
    
    bool IsNotStill(ir::Value *val, int designate) const { return !IsStill(val, designate); }
    
    bool IsStill(ir::Value *val, int designate) const {
        for (auto bak : backup_) {
            if (bak.val == val && bak.old->register_id() == designate) {
                return true;
            }
        }
        return false;
    }
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(RegisterSavingScope);
private:
    void Exit();
    
    struct Backup {
        ir::Value *val;
        LocationOperand *current;
        RegisterOperand *old;
    };
    
    OperandAllocator *const allocator_;
    const int position_;
    std::set<int> general_exclude_;
    std::set<int> float_exclude_;
    MovingDelegate *moving_delegate_;
    std::vector<Backup> backup_;
}; // class RegisterSavingScope

class RegisterPreemptiveScope final {
public:
    explicit RegisterPreemptiveScope(OperandAllocator *allocator);
    ~RegisterPreemptiveScope();
    
    RegisterOperand *Preempt(ir::Value *val, int designate);
    
private:
    struct BackupRecord {
        ir::Value *val;
        RegisterOperand *current;
        LocationOperand *backup;
        InstructionOperand *old;
    };
    
    void Enter();
    void Exit();
    
    OperandAllocator *const allocator_;
    std::vector<BackupRecord> backup_;
}; // class RegisterPreemptiveScope

} // namespace backend
} // namespace yalx

#endif // YALX_BACKEND_OPERAND_ALLOCATOR_H_
