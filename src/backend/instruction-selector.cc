#include "backend/instruction-selector.h"
#include "backend/registers-configuration.h"
#include "backend/linkage-symbols.h"
#include "backend/instruction.h"
#include "backend/frame.h"
#include "ir/metadata.h"
#include "ir/utils.h"
#include "ir/node.h"
#include "ir/type.h"
#include "base/utils.h"
#include "base/io.h"


namespace yalx::backend {

InstructionSelector::InstructionSelector(base::Arena *arena,
                                         const RegistersConfiguration *config,
                                         Linkage *linkage,
                                         ConstantsPool *const_pool)
: arena_(arena)
, config_(config)
, linkage_(linkage)
, const_pool_(const_pool)
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
        Select(instr);
    }
    
    for (auto user : block->phi_node_users()) {
        auto term = current_block_->instructions().back();
        auto moves = term->GetOrNewParallelMove(Instruction::kStart, arena_);
        if (user.dest->op()->IsConstant()) {
            moves->AddMove(DefineAsRegisterOrSlot(user.phi), UseAsImmediate(user.dest), arena_);
        } else {
            moves->AddMove(DefineAsRegisterOrSlot(user.phi), UseAsRegisterOrSlot(user.dest), arena_);
        }
        
    }
}

void InstructionSelector::Select(ir::Value *instr) {
    switch (instr->op()->value()) {
        case ir::Operator::kHeapAlloc:
            VisitHeapAlloc(instr);
            break;

        case ir::Operator::kStackAlloc:
            VisitStackAlloc(instr);
            break;

        case ir::Operator::kCallHandle:
//        case ir::Operator::kCallVirtual:
        case ir::Operator::kCallDirectly:
//        case ir::Operator::kCallAbstract:
//        case ir::Operator::kCallIndirectly:
            VisitCallDirectly(instr);
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
            }
            break;

        case ir::Operator::kRet:
            VisitReturn(instr);
            break;

        case ir::Operator::kPhi:
            VisitPhi(instr);
            break; // Ignore phi nodes

        case ir::Operator::kReturningVal:
            // Ignore returning-val nodes.
            break;

        case ir::Operator::kLoadAddress:
            VisitLoadAddress(instr);
            break;

//        case ir::Operator::kWord8Constant:
//        case ir::Operator::kWord16Constant:
//        case ir::Operator::kWord32Constant:
//        case ir::Operator::kWord64Constant:
//        case ir::Operator::kU8Constant:
//        case ir::Operator::kU16Constant:
//        case ir::Operator::kU32Constant:
//        case ir::Operator::kU64Constant:
//        case ir::Operator::kI8Constant:
//        case ir::Operator::kI16Constant:
//        case ir::Operator::kI32Constant:
//        case ir::Operator::kI64Constant:
//        case ir::Operator::kF32Constant:
//        case ir::Operator::kF64Constant:
//        case ir::Operator::kStringConstant:
//        case ir::Operator::kNilConstant:
//            return VisitConst(instr, hint);

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
    }
}

void InstructionSelector::VisitParameters(ir::Function *fun, std::vector<InstructionOperand> *parameters) {
    //fun->paramaters()
    int gp_index = 0;
    int fp_index = 0;
    for (auto param : fun->paramaters()) {
        if (param->type().IsFloating()) {
            if (fp_index < config_->number_of_argument_fp_registers()) {
                parameters->push_back(DefineAsFixedFPRegister(param, config_->argument_fp_register(fp_index++)));
            } else {
                // TODO:
                UNREACHABLE();
            }
        } else {
            
            switch (param->type().kind()) {
                case ir::Type::kValue:
                    if (param->type().IsCompactEnum()) {
                        goto pass_gp_register;
                    }
                    goto pass_gp_register;
                    
                
                case ir::Type::kString:
                case ir::Type::kReference:
                default:
                pass_gp_register:
                    if (gp_index < config_->number_of_argument_gp_registers()) {
                        parameters->push_back(DefineAsFixedRegister(param, config_->argument_gp_register(gp_index++)));
                    } else {
                        // TODO:
                        UNREACHABLE();
                    }
                    break;
            }
        }
    }
}

void InstructionSelector::VisitPhi(ir::Value *instr) {
    // Ignore
}

void InstructionSelector::VisitCallDirectly(ir::Value *ir) {
    DCHECK(ir->Is(ir::Operator::kCallHandle) || ir->Is(ir::Operator::kCallDirectly));

    ir::Function *callee = nullptr;
    if (ir->Is(ir::Operator::kCallDirectly)) {
        callee = ir::OperatorWith<ir::Function *>::Data(ir->op());
    } else {
        DCHECK(ir->Is(ir::Operator::kCallHandle));
        auto handle = ir::OperatorWith<const ir::Handle *>::Data(ir->op());
        auto method = std::get<const ir::Model::Method *>(handle->owns()->GetMember(handle));
        callee = method->fun;
    }
    std::vector<ir::Value *> returning_vals;
    returning_vals.push_back(ir);
    if (ir->users().size() > 0) {
        for (auto edge : ir->users()) {
            if (edge.user->Is(ir::Operator::kReturningVal)) {
                returning_vals.push_back(edge.user);
            }
        }
    }
    if (returning_vals.size() > 1) {
        std::sort(returning_vals.begin() + 1, returning_vals.end(), [](ir::Value *v1, ir::Value *v2) {
            return ir::OperatorWith<int>::Data(v1) < ir::OperatorWith<int>::Data(v2);
        });
    }

    std::vector<std::pair<UnallocatedOperand, InstructionOperand>> moving;

    std::vector<UnallocatedOperand> inputs;
    std::vector<ir::Value *> overflow_args;
    size_t overflow_args_size = 0;
    int gp_index = 0, fp_index = 0;
    for (int i = 0; i < ir->op()->value_in(); i++) {
        auto arg = ir->InputValue(i);
        const auto size = RoundUp(arg->type().ReferenceSizeInBytes(),
                                               static_cast<intptr_t>(arg->type().AlignmentSizeInBytes()));
        if (arg->type().IsFloating()) {
            if (fp_index < config()->number_of_argument_fp_registers()) {
                auto opd = UseAsFixedFPRegister(arg, config()->argument_fp_register(fp_index++));
                if (auto imm = TryUseAsConstantOrImmediate(arg); !imm.IsInvalid()) {
                    moving.emplace_back(opd, imm);
                }
                inputs.push_back(opd);

            } else {
                // overflow
                overflow_args_size += size;
                overflow_args.push_back(arg);
                inputs.push_back(UseAsSlot(arg));
            }
        } else {
            if (gp_index < config()->number_of_argument_gp_registers()) {
                auto opd = UseAsFixedRegister(arg, config()->argument_gp_register(gp_index++));
                if (auto imm = TryUseAsConstantOrImmediate(arg); !imm.IsInvalid()) {
                    moving.emplace_back(opd, imm);
                }
                inputs.push_back(opd);
            } else {
                // overflow
                overflow_args_size += size;
                overflow_args.push_back(arg);
                inputs.push_back(UseAsSlot(arg));
            }
        }
    }

    size_t returning_vals_size = 0;
    std::vector<UnallocatedOperand> outputs;
    for (auto rv : returning_vals) {
        if (rv->type().kind() == ir::Type::kVoid) {
            continue;
        }
        returning_vals_size += RoundUp(rv->type().ReferenceSizeInBytes(), rv->type().AlignmentSizeInBytes());
        outputs.push_back(DefineAsSlot(rv));
    }

    ImmediateOperand hints[3] = {
        ImmediateOperand{static_cast<int32_t>(ReturningValSizeInBytes(callee->prototype()))},
        ImmediateOperand{static_cast<int32_t>(overflow_args_size)},
        ImmediateOperand{-1},
    };
    Emit(ArchBeforeCall, NoOutput(), arraysize(hints), hints);

    InstructionOperand symbol[2] = {
        ReloactionOperand{linkage()->Mangle(callee->full_name())},
        ImmediateOperand{static_cast<int32_t>(overflow_args_size + returning_vals_size)}
    };
    auto instr = Emit(AndBits(ArchCall, CallDescriptorField::Encode(kCallDirectly)),
         static_cast<int>(outputs.size()), &outputs[0],
         static_cast<int>(inputs.size()), &inputs[0],
         2, symbol);

    for (auto [dest, src] : moving) {
        instr->GetOrNewParallelMove(Instruction::kStart, arena())->AddMove(dest, src, arena());
    }

    Emit(ArchAfterCall, NoOutput(), arraysize(hints), hints);
}

void InstructionSelector::VisitReturn(ir::Value *value) {
    auto fun = frame_->fun();
    auto overflow_args_size = OverflowParametersSizeInBytes(fun);
    auto returning_val_size = ReturningValSizeInBytes(fun->prototype());
    auto caller_saving_size = RoundUp(overflow_args_size + returning_val_size, Frame::kStackAlignmentSize);
    auto caller_padding_size = caller_saving_size - returning_val_size - overflow_args_size;
    auto returning_val_offset = Frame::kCalleeReservedSize + caller_padding_size + overflow_args_size;
    frame_->set_returning_val_offset(returning_val_offset);

    std::vector<InstructionOperand> inputs;
    for (int i = value->op()->value_in() - 1; i >= 0; i--) {
        auto ty = fun->prototype()->return_type(i);
        if (ty.kind() == ir::Type::kVoid) {
            continue;
        }
        if (auto input = TryUseAsConstantOrImmediate(value->InputValue(i)); input.IsInvalid()) {
            inputs.insert(inputs.begin(), DefineAsRegisterOrSlot(value->InputValue(i)));
        } else {
            inputs.insert(inputs.begin(), input);
        }
        returning_val_offset += RoundUp(ty.ReferenceSizeInBytes(), ty.AlignmentSizeInBytes());
    }

    ImmediateOperand tmp{-1};
    Emit(ArchFrameExit, 0, nullptr,static_cast<int>(inputs.size()), &inputs.front(), 1, &tmp);

//    returning_val_offset = Frame::kCalleeReservedSize + caller_padding_size + overflow_args_size;
//
//    std::vector<InstructionOperand> outputs;
//    for (int i = value->op()->value_in() - 1; i >= 0; i--) {
//        auto ty = fun->prototype()->return_type(i);
//        if (ty.kind() == ir::Type::kVoid) {
//            continue;
//        }
//        auto mr = ToMachineRepresentation(ty);
//        auto dest = AllocatedOperand::Slot(mr, config()->fp(),
//                                                    static_cast<int>(returning_val_offset));
//        if (!dest.Equals(&inputs[i])) {
//            instr->GetOrNewParallelMove(Instruction::kStart, arena())
//                 ->AddMove(dest, inputs[i], arena());
//        }
//        outputs.insert(outputs.begin(), dest);
//        returning_val_offset += RoundUp(ty.ReferenceSizeInBytes(), Frame::kSlotAlignmentSize);
//    }

    DCHECK_NOTNULL(frame_)->set_returning_val_size(static_cast<int>(returning_val_size));
}

void InstructionSelector::VisitStackAlloc(ir::Value *value) {
    auto model = ir::OperatorWith<const ir::StructureModel *>::Data(value->op());
    ImmediateOperand input{static_cast<int>(model->PlacementSizeInBytes())};
    Emit(ArchStackAlloc, DefineAsRegisterOrSlot(value), input);
}

void InstructionSelector::VisitHeapAlloc(ir::Value *value) {
    auto model = ir::OperatorWith<const ir::StructureModel *>::Data(value->op());
    
    Emit(ArchBeforeCall, NoOutput());
    
    ReloactionOperand klass = UseAsExternalClassName(model->full_name());
    UnallocatedOperand arg0(UnallocatedOperand::kFixedRegister,
                            config()->argument_gp_register(0),
                            frame()->NextVirtualRegister());
    Emit(AndBits(ArchLoadEffectAddress, CallDescriptorField::Encode(kCallNative)), arg0, klass);
    
    ReloactionOperand heap_alloc = UseAsExternalCFunction(kRt_heap_alloc);
    Emit(ArchCallNative, DefineAsFixedRegister(value, config()->returning0_register()), heap_alloc, arg0);
    
    Emit(AndBits(ArchAfterCall, CallDescriptorField::Encode(kCallNative)), NoOutput());
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
        index,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::DefineAsFixedFPRegister(ir::Value *value, int index) {
    return Define(value, UnallocatedOperand{
        UnallocatedOperand::kFixedFPRegister,
        index,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::DefineAsFixedSlot(ir::Value *value, int index) {
    return Define(value, UnallocatedOperand{
        UnallocatedOperand::FixedSlotTag{},
        index,
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

UnallocatedOperand InstructionSelector::DefineAsSlot(ir::Value *value) {
    return Define(value, UnallocatedOperand{
            UnallocatedOperand::kMustHaveSlot,
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

UnallocatedOperand InstructionSelector::UseAsSlot(ir::Value *value) {
    return Use(value, UnallocatedOperand{
            UnallocatedOperand::kMustHaveSlot,
            frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::UseAsRegisterOrSlot(ir::Value *value) {
    return Use(value, UnallocatedOperand{
        UnallocatedOperand::kRegisterOrSlot,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::UseAsFixedSlot(ir::Value *value, int index) {
    return Use(value, UnallocatedOperand{ UnallocatedOperand::FixedSlotTag(), index,
                                          frame_->GetVirtualRegister(value)});
}

UnallocatedOperand InstructionSelector::UseAsFixedRegister(ir::Value *value, int index) {
    return Use(value, UnallocatedOperand{
        UnallocatedOperand::kFixedRegister,
        index,
        frame_->GetVirtualRegister(value)
    });
}

UnallocatedOperand InstructionSelector::UseAsFixedFPRegister(ir::Value *value, int index) {
    return Use(value, UnallocatedOperand{
        UnallocatedOperand::kFixedFPRegister,
        index,
        frame_->GetVirtualRegister(value)
    });
}

ImmediateOperand InstructionSelector::UseAsImmediate(ir::Value *value) const {
    frame()->GetVirtualRegister(value);
    switch (value->op()->value()) {
        case ir::Operator::kI8Constant:
        case ir::Operator::kU8Constant:
            return ImmediateOperand{ir::OperatorWith<int8_t>::Data(value->op())};
            
        case ir::Operator::kU16Constant:
        case ir::Operator::kI16Constant:
            return ImmediateOperand{ir::OperatorWith<int16_t>::Data(value->op())};
            
        case ir::Operator::kU32Constant:
        case ir::Operator::kI32Constant:
            return ImmediateOperand{ir::OperatorWith<int32_t>::Data(value->op())};
            
        case ir::Operator::kI64Constant:
        case ir::Operator::kU64Constant:
            return ImmediateOperand{ir::OperatorWith<int64_t>::Data(value->op())};
            
        case ir::Operator::kF32Constant:
        case ir::Operator::kF64Constant:
            UNREACHABLE();
            break;
            
        default:
            break;
    }
    UNREACHABLE();
    return ImmediateOperand{0};
}

ReloactionOperand InstructionSelector::UseAsExternalClassName(const String *name) const {
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

InstructionOperand InstructionSelector::TryUseAsIntegralImmediate(ir::Value *value, int bits) {
    InstructionOperand invalid;
    switch (value->op()->value()) {
        case ir::Operator::kI8Constant:
        case ir::Operator::kU8Constant:
        case ir::Operator::kWord8Constant: {
            auto imm_val = ir::OperatorWith<int8_t>::Data(value);
            return base::is_intn(imm_val, bits) ? ImmediateOperand{imm_val} : invalid;
        } break;
            
        case ir::Operator::kI16Constant:
        case ir::Operator::kU16Constant:
        case ir::Operator::kWord16Constant: {
            auto imm_val = ir::OperatorWith<int16_t>::Data(value);
            return base::is_intn(imm_val, bits) ? ImmediateOperand{imm_val} : invalid;
        } break;
            
        case ir::Operator::kI32Constant:
        case ir::Operator::kU32Constant:
        case ir::Operator::kWord32Constant: {
            auto imm_val = ir::OperatorWith<int32_t>::Data(value);
            return base::is_intn(imm_val, bits) ? ImmediateOperand{imm_val} : invalid;
        } break;
        
        case ir::Operator::kI64Constant:
        case ir::Operator::kU64Constant:
        case ir::Operator::kWord64Constant: {
            auto imm_val = ir::OperatorWith<int64_t>::Data(value);
            return base::is_intn(imm_val, bits) ? ImmediateOperand{imm_val} : invalid;
        } break;
            
        default:
            return invalid;
    }
}

bool InstructionSelector::IsIntegralImmediate(ir::Value *value, int bits) {
    switch (value->op()->value()) {
        case ir::Operator::kI8Constant:
        case ir::Operator::kU8Constant:
        case ir::Operator::kWord8Constant:
            return base::is_intn(ir::OperatorWith<int8_t>::Data(value), bits);
            
        case ir::Operator::kI16Constant:
        case ir::Operator::kU16Constant:
        case ir::Operator::kWord16Constant:
            return base::is_intn(ir::OperatorWith<int16_t>::Data(value), bits);
            
        case ir::Operator::kI32Constant:
        case ir::Operator::kU32Constant:
        case ir::Operator::kWord32Constant:
            return base::is_intn(ir::OperatorWith<int32_t>::Data(value), bits);
        
        case ir::Operator::kI64Constant:
        case ir::Operator::kU64Constant:
        case ir::Operator::kWord64Constant:
            return base::is_intn(ir::OperatorWith<int64_t>::Data(value), bits);
            
        default:
            return false;
    }
}

bool InstructionSelector::IsAnyConstant(ir::Value *value) {
    return value->op()->IsConstant();
}

size_t InstructionSelector::OverflowParametersSizeInBytes(const ir::Function *fun) const {
    size_t size_in_bytes = 0;
    int float_count = config()->number_of_argument_fp_registers();
    int general_count = config()->number_of_argument_gp_registers();

    for (auto param : fun->paramaters()) {
        if (param->type().IsFloating()) {
            if (--float_count < 0) {
                auto size = RoundUp(param->type().ReferenceSizeInBytes(),
                                    static_cast<intptr_t>(param->type().AlignmentSizeInBytes()));
                size_in_bytes += size;
            }
        } else {
            if (--general_count < 0) {
                auto size = RoundUp(param->type().ReferenceSizeInBytes(),
                                    static_cast<intptr_t>(param->type().AlignmentSizeInBytes()));
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
        size_in_bytes += RoundUp(ty.ReferenceSizeInBytes(),static_cast<intptr_t>(ty.AlignmentSizeInBytes()));
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

} // namespace yalx
