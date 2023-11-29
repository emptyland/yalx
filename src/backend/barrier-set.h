#pragma once
#ifndef YALX_BACKEND_BARRIER_SET_H
#define YALX_BACKEND_BARRIER_SET_H

#include "backend/instruction.h"
#include "base/base.h"

namespace yalx::ir {
class Value;
}
namespace yalx::backend {

class InstructionSelector;
class InstructionBlock;
class InstructionFunction;

class BarrierSet {
public:
    BarrierSet() = default;
    virtual ~BarrierSet() = default;
    virtual void BeforeFun(InstructionSelector *selector, InstructionFunction *) {}
    virtual void AfterFun(InstructionSelector *selector, InstructionFunction *) {}
    virtual void BeforeBlock(InstructionSelector *selector, InstructionBlock *) {}
    virtual void AfterBlock(InstructionSelector *selector, InstructionBlock *) {}

    virtual void PostLoad(InstructionSelector *selector, ir::Value *dest) = 0;
    virtual void PreStore(InstructionSelector *selector) = 0;
    virtual void PostStore(InstructionSelector *selector) = 0;

    static BarrierSet *OfNoGC();
    static BarrierSet *OfYGCPosixX64();
    static BarrierSet *OfYGCPosixArm64();
    DISALLOW_IMPLICIT_CONSTRUCTORS(BarrierSet)
};

} //namespace yalx::backend

#endif //YALX_BACKEND_BARRIER_SET_H
