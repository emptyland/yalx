#pragma once
#ifndef YALX_BACKEND_INSTRUCTION_SELECTOR_H_
#define YALX_BACKEND_INSTRUCTION_SELECTOR_H_

#include "backend/instruction-code.h"
#include "backend/instruction.h"
#include "base/arena-utils.h"
#include "base/checking.h"
#include "base/base.h"

namespace yalx {
namespace ir {
class Value;
class Type;
class Function;
class PrototypeModel;
class BasicBlock;
} // namespace ir
namespace backend {

class RegistersConfiguration;
class InstructionOperand;
class Instruction;
class Frame;
class Linkage;

class InstructionFunction;
class InstructionBlock;

class InstructionSelector {
public:
    InstructionSelector(base::Arena *arena, const RegistersConfiguration *regconf, Linkage *linkage);
    
    DEF_PTR_GETTER(const RegistersConfiguration, regconf);
    DEF_PTR_GETTER(Linkage, linkage);
    DEF_PTR_GETTER(Frame, frame);
    
    InstructionFunction *VisitFunction(ir::Function *fun);
    
    void VisitBasicBlock(ir::BasicBlock *block);
    void VisitParameters(ir::Function *fun, std::vector<InstructionOperand> *parameters);
    void VisitPhi(ir::Value *instr);
    void VisitCall(ir::Value *value);
    void VisitReturn(ir::Value *value);
    void VisitStackAlloc(ir::Value *value);
    void VisitHeapAlloc(ir::Value *value);

    //virtual
    virtual void VisitCondBr(ir::Value *instr) {UNREACHABLE();}
    virtual void VisitAddOrSub(ir::Value *instr) {UNREACHABLE();}
    virtual void VisitICmp(ir::Value *instr) {UNREACHABLE();}

    Instruction *Emit(InstructionCode opcode, InstructionOperand output,
                      int temps_count = 0, InstructionOperand *temps = nullptr);
    Instruction *Emit(InstructionCode opcode, InstructionOperand output,
                      InstructionOperand input0,
                      int temps_count = 0, InstructionOperand *temps = nullptr);
    Instruction *Emit(InstructionCode opcode, InstructionOperand output,
                      InstructionOperand input0, InstructionOperand input1,
                      int temps_count = 0, InstructionOperand *temps = nullptr);
    Instruction *Emit(InstructionCode opcode, InstructionOperand output,
                      InstructionOperand input0, InstructionOperand input1, InstructionOperand input2,
                      int temps_count = 0, InstructionOperand *temps = nullptr);
    Instruction *Emit(InstructionCode opcode, InstructionOperand output,
                      InstructionOperand input0, InstructionOperand input1, InstructionOperand input2,
                      InstructionOperand input3, int temps_count = 0, InstructionOperand *temps = nullptr);
    Instruction *Emit(InstructionCode opcode, InstructionOperand output,
                      InstructionOperand input0, InstructionOperand input1, InstructionOperand input2,
                      InstructionOperand input3, InstructionOperand input4,
                      int temps_count = 0, InstructionOperand *temps = nullptr);
    
    Instruction *Emit(InstructionCode opcode,
                      int outputs_count, InstructionOperand *outputs,
                      int inputs_count, InstructionOperand *inputs,
                      int temps_count, InstructionOperand *temps);
    
    Instruction *Emit(Instruction *instr) {
        DCHECK_NOTNULL(current_block_)->Add(instr);
        return instr;
    }
    
    UnallocatedOperand DefineAsFixedRegister(ir::Value *value, int index);
    UnallocatedOperand DefineAsFixedFPRegister(ir::Value *value, int index);
    UnallocatedOperand DefineAsFixedSlot(ir::Value *value, int index);
    UnallocatedOperand DefineAsRegisterOrSlot(ir::Value *value);
    UnallocatedOperand DefineAsRegister(ir::Value *value);
    
    UnallocatedOperand UseAsRegister(ir::Value *value);
    UnallocatedOperand UseAsFixedSlot(ir::Value *value, int index);
    
    ReloactionOperand UseAsExternalClassName(const String *name);
    ReloactionOperand UseAsExternalCFunction(const String *symbol);

    static InstructionOperand Invalid() { return InstructionOperand(); }
    static InstructionOperand NoOutput() { return InstructionOperand(); }
    
    UnallocatedOperand Define(ir::Value *value, UnallocatedOperand operand);
    UnallocatedOperand Use(ir::Value *value, UnallocatedOperand operand);
    
    bool IsLive(ir::Value *value) const { return !IsDefined(value) && IsUsed(value); }
    bool IsDefined(ir::Value *value) const;
    bool IsUsed(ir::Value *value) const;
    
    size_t ReturningValSizeInBytes(const ir::PrototypeModel *proto) const;
    size_t ParametersSizeInBytes(const ir::Function *fun) const;
    size_t OverflowParametersSizeInBytes(const ir::Function *fun) const;

    void UpdateRenames(Instruction *instr);
    void TryRename(InstructionOperand *opd);
    
    bool MatchCmpOnlyUsedByBr(ir::Value *instr) const;
    
    int GetLabel(ir::BasicBlock *key) const;
    InstructionBlock *GetBlock(ir::BasicBlock *key) const;
    
    //int NextBlockLabel() { return next_blocks_label_++; }
private:
    base::Arena *const arena_;
    const RegistersConfiguration *const regconf_;
    Linkage *const linkage_;
    InstructionBlock *current_block_ = nullptr;
    Frame *frame_ = nullptr;
    base::ArenaVector<bool> defined_;
    base::ArenaVector<bool> used_;
    std::map<ir::BasicBlock *, InstructionBlock *> block_mapping_;
}; // class InstructionSelector


InstructionFunction *Arm64SelectFunctionInstructions(base::Arena *arena, Linkage *linkage, ir::Function *fun);

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_INSTRUCTION_SELECTOR_H_
