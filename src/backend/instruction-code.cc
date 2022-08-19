#include "backend/instruction-code.h"

namespace yalx {

namespace backend {

const char *const kInstrCodeNames[] = {
#define DEFINE_NAME(name) #name,
    ARCH_OPCODE_LIST(DEFINE_NAME)
    X64_ARCH_OPCODE_LIST(DEFINE_NAME)
    ARM64_ARCH_OPCODE_LIST(DEFINE_NAME)
#undef DEFINE_ENUM
};


} // namespace backend

} // namespace yalx
