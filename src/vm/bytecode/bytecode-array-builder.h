#pragma once
#ifndef YALX_VM_BYTECODE_BYTECODE_ARRAY_BUILDER_H_
#define YALX_VM_BYTECODE_BYTECODE_ARRAY_BUILDER_H_

#include "vm/bytecode/bytecode.h"
#include "base/arena-utils.h"
#include "base/checking.h"
#include "base/base.h"

namespace yalx {

namespace vm {

class BytecodeArrayBuilder final {
public:
    BytecodeArrayBuilder(base::Arena *arena);
    
    template<class T = uint8_t, Bitwise Bits = Bitwise::kNone>
    inline void Emit(Bytecode::Opcode opcode, T a = 0, T b = 0, T c = 0, T d = 0) {
        DCHECK(!Bytecode::IsWidePrefix(opcode));
        bytecodes_.push_back(new (arena_) BytecodeNode<T>(opcode, Bits, a, b, c, d));
    }
    
    template<Bitwise Bits = Bitwise::kNone>
    inline void EmitNoPrefix(Bytecode::Opcode opcode, uint8_t a = 0, uint8_t b = 0, uint8_t c = 0, uint8_t d = 0) {
        return Emit<decltype(a), Bits>(opcode, a, b, c, d);
    }
    
    template<Bitwise Bits = Bitwise::kNone>
    inline void EmitWide16(Bytecode::Opcode opcode, uint16_t a = 0, uint16_t b = 0, uint16_t c = 0, uint16_t d = 0) {
        return Emit<decltype(a), Bits>(opcode, a, b, c, d);
    }
private:
    base::Arena *arena_;
    base::ArenaVector<Bytecode *> bytecodes_;
}; // class BytecodeArrayBuilder

} // namespace vm

} // namespace yalx


#endif // YALX_VM_BYTECODE_BYTECODE_ARRAY_BUILDER_H_
