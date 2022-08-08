#include "backend/instruction-selector.h"
#include "backend/arm64/instruction-codes-arm64.h"
#include "backend/registers-configuration.h"

namespace yalx {

namespace backend {

class Arm64InstructionSelector final : public InstructionSelector {
public:
    Arm64InstructionSelector(base::Arena *arena, Linkage *linkage)
    : InstructionSelector(arena, RegistersConfiguration::of_arm64(), linkage) {}
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Arm64InstructionSelector);
private:
    
}; // class Arm64InstructionSelector

InstructionFunction *Arm64SelectFunctionInstructions(base::Arena *arena, Linkage *linkage,
                                                     ir::Function *fun) {
    Arm64InstructionSelector selector(arena, linkage);
    return selector.VisitFunction(fun);
}

} // namespace backend

} // namespace yalx

