#pragma
#ifndef YALX_BACKEND_X64_LOWER_POSIX_X64_H
#define YALX_BACKEND_X64_LOWER_POSIX_X64_H

#include "backend/instruction-selector.h"

namespace yalx::backend {

class X64PosixLower final : public InstructionSelector {
public:
    X64PosixLower(base::Arena *arena, const RegistersConfiguration *profile, Linkage *linkage, ConstantsPool *const_pool);

private:
    void VisitCondBr(ir::Value *instr) override;
    void VisitAddOrSub(ir::Value *instr) override;
    void VisitICmp(ir::Value *instr) override;
    InstructionOperand TryUseAsConstantOrImmediate(ir::Value *value) override;
}; // X64PosixLower

} // namespace yalx::backend

#endif //YALX_BACKEND_X64_LOWER_POSIX_X64_H
