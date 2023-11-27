#pragma once
#ifndef YALX_BACKEND_ZERO_SLOT_ALLOCATOR_H
#define YALX_BACKEND_ZERO_SLOT_ALLOCATOR_H

#include "backend/machine-type.h"
#include "base/base.h"
#include <vector>
#include <unordered_map>

namespace yalx::base {
class Arena;
}

namespace yalx::backend {

class RegistersConfiguration;
class InstructionFunction;
class InstructionOperand;
class InstructionBlock;
class Instruction;
class UnallocatedOperand;
class AllocatedOperand;
class ParallelMove;

class ZeroSlotAllocator {
public:
    ZeroSlotAllocator(base::Arena *arena, const RegistersConfiguration *profile, InstructionFunction *fun);

    void Run();

    void ProcessBlock(InstructionBlock *block);
    void ProcessFrame();
    void VisitParameters(InstructionBlock *entry);
    void VisitBlock(InstructionBlock *block);
    void VisitInstruction(Instruction *instr);

    DISALLOW_IMPLICIT_CONSTRUCTORS(ZeroSlotAllocator);
private:
    enum Location {
        kSlot,
        kReg,
        kFPReg,
    };
    struct Allocated {
        Location location;
        int value;
    };

    class InstrScope;

    void AddReturningForFrameExit(Instruction *instr);
    void AllocateSlot(Instruction *instr, UnallocatedOperand *unallocated);
    bool AllocateSlotIfNotExists(InstructionOperand *opd, int vr, const int *hint = nullptr);
    int AllocateSlot(InstructionOperand *opd, int vr, const int *hint = nullptr);
    int ReallocatedSlot(Instruction *instr, UnallocatedOperand *unallocated, Allocated allocated);
    void AddParallelMove(const InstructionOperand &dest, const InstructionOperand &src, const ir::Type &ty, ParallelMove *moving);

    base::Arena *const arena_;
    const RegistersConfiguration *const profile_;
    InstructionFunction *const fun_;
    Instruction *frame_enter_ = nullptr;
    std::vector<Instruction *> frame_exit_;

    InstrScope *current_ = nullptr;

    int stack_top_;
    std::unordered_map<int, Allocated> virtual_allocated_;
}; // class ZeroSlotAllocator


} // namespace yalx::backend

#endif //YALX_BACKEND_ZERO_SLOT_ALLOCATOR_H
