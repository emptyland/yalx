#pragma once
#ifndef YALX_VM_INTERPRETER_DISPATCHER_H_
#define YALX_VM_INTERPRETER_DISPATCHER_H_

#include "vm/bytecode/bytecode.h"
#include "base/checking.h"
#include "base/arena-utils.h"
#include "base/base.h"

namespace yalx {

namespace vm {

class MacroAssembler;

class Dispatcher final {
public:
    using BytecodeRoutine = void (Dispatcher::*)(MacroAssembler *, Bitwise);
    
    void Setup();

private:
    void Declare(Bytecode::Opcode op, uint32_t bitwise_bits, int operands_count, BytecodeRoutine routine);
    void EmitExtractOperands1(int wide);
    void EmitExtractOperands2(int wide);
    void EmitExtractOperands3(int wide);
    void EmitExtractOperands4(int wide);
    void EmitDispatch();
}; // class Dispatcher

} // namespace vm

} // namespace yalx

#endif // YALX_VM_INTERPRETER_DISPATCHER_H_
