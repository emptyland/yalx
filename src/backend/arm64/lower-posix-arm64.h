#pragma once
#ifndef YALX_BACKEND_LOWER_POSIX_ARM64_H
#define YALX_BACKEND_LOWER_POSIX_ARM64_H

#include "backend/instruction-selector.h"

namespace yalx::backend {

class Arm64PosixLower final : public InstructionSelector {
public:
    Arm64PosixLower(base::Arena *arena, const RegistersConfiguration *profile, Linkage *linkage,
                    ConstantsPool *const_pool, BarrierSet *barrier_set);

private:
    void VisitCondBr(ir::Value *instr) override;
    void VisitAddOrSub(ir::Value *ir) override;
    void VisitICmp(ir::Value *instr) override;
    void VisitLoadAddress(ir::Value *ir) override;
    InstructionOperand TryUseAsConstantOrImmediate(ir::Value *value) override;

    void VisitArithOperands(InstructionCode op, InstructionOperand output, ir::Value *input0, ir::Value *input1);
}; // Arm64PosixLower

} // namespace yalx::backend

#endif // YALX_BACKEND_LOWER_POSIX_ARM64_H
