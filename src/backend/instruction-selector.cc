#include "backend/instruction-selector.h"
#include "backend/registers-configuration.h"
#include "backend/linkage-symbols.h"
#include "backend/instruction.h"
#include "backend/frame.h"
#include "ir/metadata.h"
#include "ir/utils.h"
#include "ir/node.h"
#include "ir/type.h"
#include "base/io.h"

namespace yalx {

namespace backend {

InstructionSelector::InstructionSelector(base::Arena *arena, const RegistersConfiguration *regconf,
                                         Linkage *linkage)
: arena_(arena)
, regconf_(regconf)
, linkage_(linkage)
, defined_(arena)
, used_(arena) {
    
}

InstructionFunction *InstructionSelector::VisitFunction(ir::Function *fun) {
    frame_ = new (arena_) Frame(arena_, fun);
    auto instr_fun = new (arena_) InstructionFunction(arena_, linkage()->Mangle(fun->full_name()), frame_);
    
    std::vector<InstructionOperand> parameters;
    VisitParameters(fun, &parameters);
    
    for (auto basic_block : fun->blocks()) {
        block_mapping_[basic_block] = instr_fun->NewBlock(linkage()->NextBlockLabel());
    }
    
    instr_fun->set_entry(block_mapping_[fun->entry()]);
    
    for (auto basic_block : fun->blocks()) {
        auto instr_block = block_mapping_[basic_block];

        for (auto predecessor : basic_block->inputs()) {
            instr_block->AddPredecessors(block_mapping_[predecessor]);
        }
        for (auto successor : basic_block->outputs()) {
            instr_block->AddSuccessor(block_mapping_[successor]);
        }
        
        current_block_ = instr_block;
        
        if (basic_block == fun->entry()) {
            InstructionOperand temps[1];
            temps[0] = ImmediateOperand{-1};
            //Emit(ArchFrameEnter, NoOutput(), arraysize(temps), temps);
            Emit(ArchFrameEnter,
                 static_cast<int>(parameters.size()),
                 &parameters.front(),
                 0,
                 nullptr,
                 arraysize(temps), temps);
        }
        
        VisitBasicBlock(basic_block);
        current_block_ = nullptr;
    }
    
    block_mapping_.clear();
    return instr_fun;
}

void InstructionSelector::VisitBasicBlock(ir::BasicBlock *block) {
    
    for (auto instr : block->instructions()) {
        
        switch (instr->op()->value()) {
            case ir::Operator::kHeapAlloc:
                VisitHeapAlloc(instr);
                break;
            
            case ir::Operator::kStackAlloc:
                VisitStackAlloc(instr);
                break;
                
            case ir::Operator::kCallHandle:
            case ir::Operator::kCallVirtual:
            case ir::Operator::kCallDirectly:
            case ir::Operator::kCallAbstract:
            case ir::Operator::kCallIndirectly:
                VisitCall(instr);
                break;
                
            case ir::Operator::kAdd:
            case ir::Operator::kSub:
                VisitAddOrSub(instr);
                break;
                
            case ir::Operator::kICmp:
                VisitICmp(instr);
                break;
                
            case ir::Operator::kBr:
                if (instr->op()->value_in() > 0) {
                    VisitCondBr(instr);
                } else {
                    Emit(ArchJmp, NoOutput(), ReloactionOperand(GetBlock(instr->OutputControl(0))));
                    Emit(ArchNop, NoOutput());
                }
                break;

            case ir::Operator::kRet:
                VisitReturn(instr);
                break;
                
            case ir::Operator::kPhi:
                VisitPhi(instr);
                break; // Ignore phi nodes

            debugging_info:
            default: {
            #ifndef NDEBUG
                ir::PrintingContext ctx(0);
                std::string buf;
                auto file = base::NewMemoryWritableFile(&buf);
                base::PrintingWriter printer(file, true);
                instr->PrintTo(&ctx, &printer);
                printd("%s", buf.c_str());
            #endif
                UNREACHABLE();
            } break;
                break;
        }
    }
}

void InstructionSelector::VisitParameters(ir::Function *fun, std::vector<InstructionOperand> *parameters) {
    //fun->paramaters()
    int gp_index = 0;
    int fp_index = 0;
    for (auto param : fun->paramaters()) {
        if (param->type().IsFloating()) {
            if (fp_index < regconf_->number_of_argument_fp_registers()) {
                parameters->push_back(DefineAsFixedFPRegister(param, regconf_->argument_fp_register(fp_index)));
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
                        parameters->push_back(DefineAsFixedRegister(param, regconf_->argument_gp_register(gp_index)));
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

void InstructionSelector::VisitPhi(ir::Value *instr) {
    auto input_count = instr->op()->value_in();
    DCHECK(input_count == current_block_->predecessors_size());
    auto phi = new (arena_) PhiInstruction(arena_, frame()->GetVirtualRegister(instr), input_count);
    current_block_->AddPhi(phi);
    for (int i = 0; i < input_count; i++) {
        auto vr = frame()->GetVirtualRegister(instr->InputValue(i));
        phi->SetInput(i, vr);
        if (vr >= used_.size()) {
            used_.resize(vr + 1, false);
        }
        used_[vr] = true;
    }
}

void InstructionSelector::VisitCall(ir::Value *value) {
//    DCHECK(value->Is(ir::Operator::kCallHandle) ||
//           value->Is(ir::Operator::kCallVirtual) ||
//           value->Is(ir::Operator::kCallAbstract) ||
//           value->Is(ir::Operator::kCallDirectly) ||
//           value->Is(ir::Operator::kCallIndirectly));
    
    UNREACHABLE();
}

void InstructionSelector::VisitReturn(ir::Value *value) {
    auto fun = frame_->fun();
    auto overflow_args_size = OverflowParametersSizeInBytes(fun);
    auto returning_val_size = ReturningValSizeInBytes(fun->prototype());
    auto caller_saving_size = RoundUp(overflow_args_size + returning_val_size, Frame::kStackAlignmentSize);
    auto caller_padding_size = caller_saving_size - returning_val_size - overflow_args_size;
    auto returning_val_offset = Frame::kCalleeReservedSize + caller_padding_size + overflow_args_size;

    std::vector<InstructionOperand> inputs;
    for (int i = value->op()->value_in() - 1; i >= 0; i--) {
        auto ty = fun->prototype()->return_type(i);
        if (ty.kind() == ir::Type::kVoid) {
            continue;
        }
        inputs.push_back(UseAsFixedSlot(value->InputValue(i), static_cast<int>(returning_val_offset)));
        returning_val_offset += RoundUp(ty.ReferenceSizeInBytes(), Frame::kSlotAlignmentSize);
    }

    ImmediateOperand tmp{-1};
    Emit(ArchFrameExit, 0, nullptr, static_cast<int>(inputs.size()), &inputs.front(), 1, &tmp);
    
    frame_->set_returning_val_size(static_cast<int>(returning_val_size));
}

void InstructionSelector::VisitStackAlloc(ir::Value *value) {
    auto model = ir::OperatorWith<const ir::StructureModel *>::Data(value->op());
    ImmediateOperand input{static_cast<int>(model->PlacementSizeInBytes())};
    Emit(ArchStackAlloc, DefineAsRegisterOrSlot(value), input);
}

void InstructionSelector::VisitHeapAlloc(ir::Value *value) {
    auto model = ir::OperatorWith<const ir::StructureModel *>::Data(value->op());
    
    Emit(ArchSaveCallerRegisters, NoOutput());
    
    ReloactionOperand klass = UseAsExternalClassName(model->full_name());
    UnallocatedOperand arg0(UnallocatedOperand::kFixedRegister,
                            regconf()->argument_gp_register(0),
                            frame()->NextVirtualRegister());
    Emit(AndBits(ArchLoadEffectAddress, CallDescriptorField::Encode(kCallNative)), arg0, klass);
    
    ReloactionOperand heap_alloc = UseAsExternalCFunction(kRt_heap_alloc);
    Emit(ArchCallNative, DefineAsRegisterOrSlot(value), heap_alloc, arg0);
    
    Emit(AndBits(ArchRestoreCallerRegisters, CallDescriptorField::Encode(kCallNative)), NoOutput());
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

UnallocatedOperand InstructionSelector::DefineAsFixedRegister(ir::Value *value, int index) {
    return Define(value, UnallocatedOperand{
        UnallocatedOperand::kFixedRegister,
        UnallocatedOperand::kUsedAtStart,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::DefineAsFixedFPRegister(ir::Value *value, int index) {
    return Define(value, UnallocatedOperand{
        UnallocatedOperand::kFixedFPRegister,
        UnallocatedOperand::kUsedAtStart,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::DefineAsFixedSlot(ir::Value *value, int index) {
    return Define(value, UnallocatedOperand{
        UnallocatedOperand::kFixedSlot,
        UnallocatedOperand::kUsedAtStart,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::DefineAsRegisterOrSlot(ir::Value *value) {
    return Define(value, UnallocatedOperand{
        UnallocatedOperand::kRegisterOrSlot,
        UnallocatedOperand::kUsedAtStart,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::DefineAsRegister(ir::Value *value) {
    return Define(value, UnallocatedOperand{
        UnallocatedOperand::kMustHaveRegister,
        UnallocatedOperand::kUsedAtStart,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::UseAsRegister(ir::Value *value) {
    return Use(value, UnallocatedOperand{
        UnallocatedOperand::kMustHaveRegister,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::UseAsFixedSlot(ir::Value *value, int index) {
    return Use(value, UnallocatedOperand{ UnallocatedOperand::FixedSlotTag(), index, frame_->GetVirtualRegister(value)});
}

ReloactionOperand InstructionSelector::UseAsExternalClassName(const String *name) {
    return ReloactionOperand {linkage()->MangleClassName(name)};
}

ReloactionOperand InstructionSelector::UseAsExternalCFunction(const String *symbol) {
    return ReloactionOperand {symbol};
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
    if (vid >= used_.size()) {
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

size_t InstructionSelector::OverflowParametersSizeInBytes(const ir::Function *fun) const {
    size_t size_in_bytes = 0;
    int float_count = regconf()->number_of_argument_fp_registers();
    int general_count = regconf()->number_of_argument_gp_registers();

    for (auto param : fun->paramaters()) {
        if (param->type().IsFloating()) {
            if (--float_count < 0) {
                auto size = RoundUp(param->type().ReferenceSizeInBytes(), Frame::kSlotAlignmentSize);
                size_in_bytes += size;
            }
        } else {
            if (--general_count < 0) {
                auto size = RoundUp(param->type().ReferenceSizeInBytes(), Frame::kSlotAlignmentSize);
                size_in_bytes += size;
            }
        }
    }
    return size_in_bytes;
}

size_t InstructionSelector::ReturningValSizeInBytes(const ir::PrototypeModel *proto) const {
    size_t size_in_bytes = 0;
    for (auto ty : proto->return_types()) {
        if (ty.kind() == ir::Type::kVoid) {
            continue;
        }
        size_in_bytes += RoundUp(ty.ReferenceSizeInBytes(), Frame::kSlotAlignmentSize);
    }
    return size_in_bytes;
}

size_t InstructionSelector::ParametersSizeInBytes(const ir::Function *fun) const {
    size_t size_in_bytes = 0;
    for (auto param : fun->paramaters()) {
        auto size = RoundUp(param->type().ReferenceSizeInBytes(), Frame::kSlotAlignmentSize);
        size_in_bytes += size;
    }
    return size_in_bytes;
}

void InstructionSelector::UpdateRenames(Instruction *instr) {
    for (int i = 0; i < instr->inputs_count(); i++) {
        TryRename(instr->InputAt(i));
    }
}

void InstructionSelector::TryRename(InstructionOperand *opd) {
    if (!opd->IsUnallocated()) {
        return;
    }
    
    auto unallocated = opd->AsUnallocated();
    auto rename = frame()->GetRename(unallocated->virtual_register());
    if (rename != unallocated->virtual_register()) {
        *opd = UnallocatedOperand(*unallocated, rename);
    }
}

// instr->users().size() == 1 && instr->users().begin()->user->Is(ir::Operator::kBr)
bool InstructionSelector::MatchCmpOnlyUsedByBr(ir::Value *instr) const {
    return instr->users().size() == 1 && instr->users().begin()->user->Is(ir::Operator::kBr);
}

int InstructionSelector::GetLabel(ir::BasicBlock *key) const {
    return GetBlock(key)->label();
}

InstructionBlock *InstructionSelector::GetBlock(ir::BasicBlock *key) const {
    auto iter = block_mapping_.find(key);
    DCHECK(iter != block_mapping_.end());
    return iter->second;
}

} // namespace backend

} // namespace yalx
