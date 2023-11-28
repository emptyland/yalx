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
class ConstantsPool;
class InstructionOperand;
class Instruction;
class Frame;
class Linkage;

class InstructionFunction;
class InstructionBlock;

class InstructionSelector {
public:
    InstructionSelector(base::Arena *arena,
                        const RegistersConfiguration *config,
                        Linkage *linkage,
                        ConstantsPool *const_pool);
    
    DEF_PTR_GETTER(const RegistersConfiguration, config);
    DEF_PTR_GETTER(Linkage, linkage);
    DEF_PTR_GETTER(Frame, frame);
    DEF_PTR_GETTER(base::Arena, arena);
    DEF_PTR_GETTER(ConstantsPool, const_pool);
    
    InstructionFunction *VisitFunction(ir::Function *fun);
    
    void VisitBasicBlock(ir::BasicBlock *block);
    void Select(ir::Value *instr);
    void VisitParameters(ir::Function *fun, std::vector<InstructionOperand> *parameters);
    void VisitPhi(ir::Value *instr);
    void VisitCallDirectly(ir::Value *ir);
    void VisitReturn(ir::Value *value);
    void VisitStackAlloc(ir::Value *value);
    void VisitHeapAlloc(ir::Value *value);

    //virtual
    virtual void VisitCondBr(ir::Value *instr) {UNREACHABLE();}
    virtual void VisitAddOrSub(ir::Value *instr) {UNREACHABLE();}
    virtual void VisitICmp(ir::Value *instr) {UNREACHABLE();}
    virtual void VisitLoadAddress(ir::Value *instr) {UNREACHABLE();}
    virtual Instruction *EmitLoadAddress(InstructionOperand output, InstructionOperand input) {return nullptr;}
    virtual InstructionOperand TryUseAsConstantOrImmediate(ir::Value *value) {UNREACHABLE();}

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
    UnallocatedOperand DefineAsSlot(ir::Value *value);
    UnallocatedOperand DefineAsRegister(ir::Value *value);
    
    UnallocatedOperand UseAsRegister(ir::Value *value);
    UnallocatedOperand UseAsSlot(ir::Value *value);
    UnallocatedOperand UseAsRegisterOrSlot(ir::Value *value);
    UnallocatedOperand UseAsFixedSlot(ir::Value *value, int index);
    UnallocatedOperand UseAsFixedRegister(ir::Value *value, int index);
    UnallocatedOperand UseAsFixedFPRegister(ir::Value *value, int index);
    ImmediateOperand UseAsImmediate(ir::Value *value) const;
    
    ReloactionOperand UseAsExternalClassName(const String *name) const;
    static ReloactionOperand UseAsExternalCFunction(const String *symbol);

    static InstructionOperand TryUseAsIntegralImmediate(ir::Value *value, int bits = 63);
    
    static bool IsIntegralImmediate(ir::Value *value, int bits = 64);
    static bool IsAnyConstant(ir::Value *value);

    static InstructionOperand Invalid() { return {}; }
    static InstructionOperand NoOutput() { return {}; }
    
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
    const RegistersConfiguration *const config_;
    Linkage *const linkage_;
    ConstantsPool *const const_pool_;
    InstructionBlock *current_block_ = nullptr;
    Frame *frame_ = nullptr;
    base::ArenaVector<char> defined_;
    base::ArenaVector<char> used_;
    std::map<ir::BasicBlock *, InstructionBlock *> block_mapping_;
}; // class InstructionSelector


InstructionFunction *Arm64SelectFunctionInstructions(base::Arena *arena, Linkage *linkage, ConstantsPool *const_pool,
                                                     ir::Function *fun);

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_INSTRUCTION_SELECTOR_H_
