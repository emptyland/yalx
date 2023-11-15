#include "backend/x64/lower-posix-x64.h"
#include "ir/metadata.h"
#include "ir/utils.h"
#include "ir/node.h"
#include "ir/type.h"
#include "base/utils.h"
#include "base/io.h"

namespace yalx::backend {

X64PosixLower::X64PosixLower(base::Arena *arena, const RegistersConfiguration *config, Linkage *linkage,
                             ConstantsPool *const_pool)
                             : InstructionSelector(arena, config, linkage, const_pool) {
}


void X64PosixLower::VisitCondBr(ir::Value *instr) {
    UNREACHABLE();
}

void X64PosixLower::VisitAddOrSub(ir::Value *instr) {
    UNREACHABLE();
}

void X64PosixLower::VisitICmp(ir::Value *instr) {
    UNREACHABLE();
}

} // namespace yalx::backend