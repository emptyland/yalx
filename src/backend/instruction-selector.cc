#include "backend/instruction-selector.h"
#include "backend/registers-configuration.h"
#include "backend/instruction.h"
#include "backend/frame.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/type.h"

namespace yalx {

namespace backend {

InstructionSelector::InstructionSelector(const RegistersConfiguration *regconf, base::Arena *arena)
: arena_(arena)
, regconf_(regconf)
, instructions_(arena)
, defined_(arena)
, used_(arena) {
    
}

void InstructionSelector::VisitParameters(ir::Function *fun) {
    //fun->paramaters()
    int gp_index = 0;
    int fp_index = 0;
    for (auto param : fun->paramaters()) {
        if (param->type().IsFloating()) {
            if (fp_index < regconf_->number_of_argument_fp_registers()) {
                DefineFixedFPRegister(param, regconf_->argument_fp_register(fp_index));
            } else {
                // TODO:
                UNREACHABLE();
            }
            fp_index++;
        } else {
            
            switch (param->type().kind()) {
                case ir::Type::kValue:
                    if (param->type().IsCompactEnum()) {
                        goto pass_gp_register;
                    }
                    // TODO:
                    UNREACHABLE();
                    break;
                    
                
                case ir::Type::kString:
                case ir::Type::kReference:
                default:
                pass_gp_register:
                    if (gp_index < regconf_->number_of_argument_gp_registers()) {
                        DefineFixedRegister(param, regconf_->argument_gp_register(gp_index));
                    } else {
                        // TODO:
                        UNREACHABLE();
                    }
                    break;
            }
            
            gp_index++;
        }
    }
}

void InstructionSelector::VisitCall(ir::Value *value) {
//    DCHECK(value->Is(ir::Operator::kCallHandle) ||
//           value->Is(ir::Operator::kCallVirtual) ||
//           value->Is(ir::Operator::kCallAbstract) ||
//           value->Is(ir::Operator::kCallDirectly) ||
//           value->Is(ir::Operator::kCallIndirectly));
    
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, InstructionOperand output,
                                       int temps_count, InstructionOperand *temps) {
    int outputs_count = output.IsInvalid() ? 0 : 1;
    return Emit(opcode, outputs_count, &output, 0, nullptr, temps_count, temps);
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, InstructionOperand output,
                                       InstructionOperand input0, int temps_count, InstructionOperand *temps) {
    int outputs_count = output.IsInvalid() ? 0 : 1;
    InstructionOperand inputs[] = {input0};
    int inputs_count = arraysize(inputs);
    return Emit(opcode, outputs_count, &output, inputs_count, inputs, temps_count, temps);
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, InstructionOperand output, InstructionOperand input0,
                                       InstructionOperand input1, int temps_count, InstructionOperand *temps) {
    int outputs_count = output.IsInvalid() ? 0 : 1;
    InstructionOperand inputs[] = {input0, input1};
    int inputs_count = arraysize(inputs);
    return Emit(opcode, outputs_count, &output, inputs_count, inputs, temps_count, temps);
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, InstructionOperand output, InstructionOperand input0,
                                       InstructionOperand input1, InstructionOperand input2, int temps_count,
                                       InstructionOperand *temps) {
    int outputs_count = output.IsInvalid() ? 0 : 1;
    InstructionOperand inputs[] = {input0, input1, input2};
    int inputs_count = arraysize(inputs);
    return Emit(opcode, outputs_count, &output, inputs_count, inputs, temps_count, temps);
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, InstructionOperand output, InstructionOperand input0,
                                       InstructionOperand input1, InstructionOperand input2, InstructionOperand input3,
                                       int temps_count, InstructionOperand *temps) {
    int outputs_count = output.IsInvalid() ? 0 : 1;
    InstructionOperand inputs[] = {input0, input1, input2, input3};
    int inputs_count = arraysize(inputs);
    return Emit(opcode, outputs_count, &output, inputs_count, inputs, temps_count, temps);
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, InstructionOperand output, InstructionOperand input0,
                                       InstructionOperand input1, InstructionOperand input2, InstructionOperand input3,
                                       InstructionOperand input4, int temps_count, InstructionOperand *temps) {
    int outputs_count = output.IsInvalid() ? 0 : 1;
    InstructionOperand inputs[] = {input0, input1, input2, input3, input4};
    int inputs_count = arraysize(inputs);
    return Emit(opcode, outputs_count, &output, inputs_count, inputs, temps_count, temps);
}

Instruction *InstructionSelector::Emit(InstructionCode opcode, int outputs_count, InstructionOperand *outputs,
                                       int inputs_count, InstructionOperand *inputs, int temps_count,
                                       InstructionOperand *temps) {
    auto instr = Instruction::New(arena_, opcode, inputs_count, inputs, outputs_count, outputs, temps_count, temps);
    return Emit(instr);
}

UnallocatedOperand InstructionSelector::DefineFixedRegister(ir::Value *value, int index) {
    return Define(value, UnallocatedOperand{
        UnallocatedOperand::kFixedRegister,
        UnallocatedOperand::kUsedAtStart,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::DefineFixedFPRegister(ir::Value *value, int index) {
    return Define(value, UnallocatedOperand{
        UnallocatedOperand::kFixedFPRegister,
        UnallocatedOperand::kUsedAtStart,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::DefineFixedSlot(ir::Value *value, int index) {
    return Define(value, UnallocatedOperand{
        UnallocatedOperand::kFixedSlot,
        UnallocatedOperand::kUsedAtStart,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::Define(ir::Value *value, UnallocatedOperand operand) {
    DCHECK(frame_->GetVirtualRegister(value) == operand.virtual_register());
    auto vid = operand.virtual_register();
    if (vid >= defined_.size()) {
        defined_.resize(vid + 1, false);
    }
    defined_[vid] = true;
    return operand;
}

UnallocatedOperand InstructionSelector::Use(ir::Value *value, UnallocatedOperand operand) {
    DCHECK(frame_->GetVirtualRegister(value) == operand.virtual_register());
    auto vid = operand.virtual_register();
    if (vid >= defined_.size()) {
        used_.resize(vid + 1, false);
    }
    used_[vid] = true;
    return operand;
}

bool InstructionSelector::IsDefined(ir::Value *value) const {
    auto vid = frame_->GetVirtualRegister(value);
    DCHECK(vid >= 0 && vid < defined_.size());
    return defined_[vid];
}

bool InstructionSelector::IsUsed(ir::Value *value) const {
    auto vid = frame_->GetVirtualRegister(value);
    DCHECK(vid >= 0 && vid < used_.size());
    return used_[vid];
}

} // namespace backend

} // namespace yalx
