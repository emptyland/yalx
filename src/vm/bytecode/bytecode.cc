#include "vm/bytecode/bytecode.h"

namespace yalx {

namespace vm {

int Bytecode::OperandsCount() const {
    switch (opcode()) {
#define DEFINE_OPERANDS_COUNT(name, acc, count, ...) case k##name: return count;
        DECL_BYTECODES(DEFINE_OPERANDS_COUNT)
#undef DEFINE_OPERANDS_COUNT
        default:
            UNREACHABLE();
            break;
    }
    return -1;
}

size_t Bytecode::Encode(uint8_t **buf) const {
    switch (prefix()) {
        case kWide16:
            return Cast<uint16_t>(this)->Encode(buf);
        case kWide32:
            return Cast<uint32_t>(this)->Encode(buf);
        case kWide64:
            return Cast<uint64_t>(this)->Encode(buf);
        default:
            return Cast<uint8_t>(this)->Encode(buf);
    }
}

} // namespace vm

} // namespace yalx
