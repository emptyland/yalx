#include "backend/instruction.h"
#include "backend/linkage-symbols.h"
#include "ir/metadata.h"
#include "ir/type.h"
#include "compiler/constants.h"
#include "runtime/object/type.h"
#include "base/checking.h"
#include "base/io.h"
#include <inttypes.h>

namespace yalx::backend {

bool InstructionOperand::Equals(const InstructionOperand *other) const {
    if (kind() != other->kind()) {
        return false;
    }
    switch (kind()) {
        case kInvalid:
            return other->IsInvalid();
        case kConstant: {
            auto lhs = AsConstant();
            auto rhs = other->AsConstant();
            return lhs->type() == rhs->type() && lhs->symbol_id() == rhs->symbol_id();
        } break;
        case kAllocated: {
            auto lhs = AsAllocated();
            auto rhs = other->AsAllocated();
            return lhs->location_kind() == rhs->location_kind() &&
                lhs->machine_representation() == rhs->machine_representation() && lhs->index() == rhs->index();
        } break;
        case kImmediate: {
            auto lhs = AsImmediate();
            auto rhs = other->AsImmediate();
            if (lhs->machine_representation() != rhs->machine_representation()) {
                return false;
            }
            switch (lhs->machine_representation()) {
                case MachineRepresentation::kWord8:
                    return lhs->word8_value() == rhs->word8_value();
                case MachineRepresentation::kWord16:
                    return lhs->word16_value() == rhs->word16_value();
                case MachineRepresentation::kWord32:
                    return lhs->word32_value() == rhs->word32_value();
                case MachineRepresentation::kWord64:
                    return lhs->word64_value() == rhs->word64_value();
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
        case kReloaction: {
            auto lhs = AsReloaction();
            auto rhs = other->AsReloaction();
            if (lhs->is_label() != rhs->is_label()) {
                return false;
            }
            if (lhs->is_label()) {
                return lhs->is_label() == rhs->is_label() &&
                    lhs->label() == rhs->label() &&
                    lhs->offset() == rhs->offset();
            } else {
                return lhs->is_symbol() == rhs->is_symbol() &&
                    lhs->symbol_name() == rhs->symbol_name() &&
                    lhs->offset() == rhs->offset();
            }
        } break;
        default:
            UNREACHABLE();
            break;
    }
}

void InstructionOperand::PrintTo(base::PrintingWriter *printer) const {
    switch (kind()) {
        case kInvalid:
            printer->Write("!INVALID!");
            break;
            
        case kUnallocated: {
            auto opd = AsUnallocated();
            printer->Print("%%%d [", opd->virtual_register());
            switch (opd->policy()) {
                case UnallocatedOperand::kNone:
                    break;
                case UnallocatedOperand::kFixedSlot:
                    if (opd->fixed_slot_offset() >= 0) {
                        printer->Print("fp+%d", opd->fixed_slot_offset());
                    } else {
                        printer->Print("fp%d", opd->fixed_slot_offset());
                    }
                    break;
                case UnallocatedOperand::kFixedRegister:
                    printer->Print("gp=%d", opd->fixed_register_id());
                    break;
                case UnallocatedOperand::kFixedFPRegister:
                    printer->Print("fp=%d", opd->fixed_fp_register_id());
                    break;
                case UnallocatedOperand::kMustHaveSlot:
                    printer->Write("fp+x");
                    break;
                case UnallocatedOperand::kMustHaveRegister:
                    printer->Write("r:x");
                    break;
                case UnallocatedOperand::kRegisterOrSlot:
                    printer->Write("m/r");
                    break;
                case UnallocatedOperand::kRegisterOrSlotOrConstant:
                    printer->Write("m/r/k");
                    break;
                case UnallocatedOperand::kSameAsInput:
                    printer->Write("=in");
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            printer->Write("]");
        } break;
            
        case kAllocated: {
            auto opd = AsAllocated();
            printer->Print("{%s ", GetMachineRepresentationAlias(opd->machine_representation()));
            switch (opd->location_kind()) {
                case AllocatedOperand::kRegister:
                    printer->Print("$%d", opd->index());
                    break;
                case AllocatedOperand::kSlot:
                    if (opd->index() >= 0) {
                        printer->Print("fp+%d", opd->index());
                    } else {
                        printer->Print("fp%d", opd->index());
                    }
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            printer->Write("}");
        } break;
            
        case kConstant: {
            auto opd = AsConstant();
            printer->Write("<");
            switch (opd->type()) {
                case ConstantOperand::kString:
                    printer->Print("literals:%d", opd->symbol_id());
                    break;
                case ConstantOperand::kNumber:
                    printer->Print("const:%d", opd->symbol_id());
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            printer->Write(">");
        } break;
            
        case kImmediate: {
            auto opd = AsImmediate();
            switch (opd->machine_representation()) {
                case MachineRepresentation::kWord8:
                    printer->Print("#%" PRIi8, opd->word8_value());
                    break;
                case MachineRepresentation::kWord16:
                    printer->Print("#%" PRIi16, opd->word8_value());
                    break;
                case MachineRepresentation::kWord32:
                    printer->Print("#%" PRIi32, opd->word8_value());
                    break;
                case MachineRepresentation::kWord64:
                    printer->Print("#%" PRIi64, opd->word8_value());
                    break;
                case MachineRepresentation::kFloat32:
                    printer->Print("#%f", opd->float32_value());
                    break;
                case MachineRepresentation::kFloat64:
                    printer->Print("#%f", opd->float64_value());
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case kReloaction: {
            auto opd = AsReloaction();
            printer->Write("<");
            if (opd->is_label()) {
                printer->Print("L%d:", opd->label()->label());
            }
            if (opd->is_symbol()) {
                printer->Print("%s", opd->symbol_name()->data());
            }
            printer->Write(">");
        } break;
            
        default:
            UNREACHABLE();
            break;
    }
}

Instruction::Instruction(Code op,
                         size_t inputs_count,
                         Operand inputs[],
                         size_t outputs_count,
                         Operand outputs[],
                         size_t temps_count,
                         Operand temps[])
: op_(op)
, inputs_count_(inputs_count)
, outputs_count_(outputs_count)
, temps_count_(temps_count) {
    parallel_moves_[0] = nullptr;
    parallel_moves_[1] = nullptr;
    ::memcpy(operands_ + input_offset(), inputs, inputs_count * sizeof(Operand));
    ::memcpy(operands_ + output_offset(), outputs, outputs_count * sizeof(Operand));
    ::memcpy(operands_ + temp_offset(), temps, temps_count * sizeof(Operand));
}

PhiInstruction::PhiInstruction(base::Arena *arena, int virtual_register, int input_count)
: virtual_register_(virtual_register)
, output_(UnallocatedOperand(UnallocatedOperand::kRegisterOrSlot, virtual_register))
, operands_(arena)
, inputs_(arena) {
    operands_.resize(input_count, InstructionOperand::kInvliadVirtualRegister);
    inputs_.resize(input_count, nullptr);
}

Instruction *Instruction::New(base::Arena *arena, Code op,
                              size_t inputs_count,
                              Operand inputs[],
                              size_t outputs_count,
                              Operand outputs[],
                              size_t temps_count,
                              Operand temps[]) {
    auto chunk = AllocatePlacementMemory(arena, inputs_count, outputs_count, temps_count);
    return new (chunk) Instruction(op, inputs_count, inputs, outputs_count, outputs, temps_count, temps);
}

void *Instruction::AllocatePlacementMemory(base::Arena *arena, size_t inputs_count, size_t outputs_count,
                                           size_t temps_count) {
    size_t in_memory_bytes = sizeof(Instruction) + (inputs_count + outputs_count + temps_count) * sizeof(Operand);
    return arena->Allocate(in_memory_bytes);
}

void Instruction::PrintTo(int ident, base::PrintingWriter *printer) const {
    if (auto moves = parallel_move(kStart)) {
        for (auto opds : moves->moves()) {
            if (id() >= 0) {
                printer->Write("[ ^ ]")->Indent(ident)->Write("Move ");
            } else {
                printer->Indent(ident)->Write("Move ");
            }
            opds->dest().PrintTo(printer);
            printer->Write(" <- ");
            opds->src().PrintTo(printer);
            printer->Writeln("");
        }
    }
    
    if (id() >= 0) {
        printer->Print("[%03d]", id());
    }
    printer->Indent(ident);
    
    for (int i = 0; i < outputs_count(); i++) {
        if (i > 0) {
            printer->Write(", ");
        }
        operands_[output_offset() + i].PrintTo(printer);
    }
    
    //printd("%08x %08x\n", InstructionCodeField::kMask, InstructionCodeField::kBits);
    auto code = InstructionCodeField::Decode(op());
    if (outputs_count() > 0) {
        printer->Print(" = %s", kInstrCodeNames[code]);
    } else {
        printer->Print("%s", kInstrCodeNames[code]);
    }
    if (inputs_count() + temps_count() > 0) {
        printer->Write(" ");
    }
    
    for (int i = 0; i < inputs_count(); i++) {
        if (i > 0) {
            printer->Write(", ");
        }
        operands_[input_offset() + i].PrintTo(printer);
    }
    
    if (temps_count() > 0) {
        printer->Write("(");
        for (int i = 0; i < temps_count(); i++) {
            if (i > 0) {
                printer->Write(", ");
            }
            operands_[temp_offset() + i].PrintTo(printer);
        }
        printer->Write(")");
    }
    
    printer->Write("\n");
    
    if (auto moves = parallel_move(kEnd)) {
        for (auto opds : moves->moves()) {
            if (id() >= 0) {
                printer->Write("[ v ]")->Indent(ident)->Write("Move ");
            } else {
                printer->Indent(ident)->Write("Move ");
            }
            opds->dest().PrintTo(printer);
            printer->Write(" <- ");
            opds->src().PrintTo(printer);
            printer->Writeln("");
        }
    }
}

void PhiInstruction::PrintTo(int ident, base::PrintingWriter *printer) const {
    
    if (id() >= 0) {
        printer->Print("[%03d]", id());
    }
    printer->Indent(ident);
    output_.PrintTo(printer);
    printer->Print(" = phi <%%%d>", virtual_register());
    for (auto input : operands_) {
        printer->Print(", %%%d", input);
    }
    printer->Write("\n");
}

InstructionFunction::InstructionFunction(base::Arena *arena, const String *symbol, Frame *frame)
: arena_(arena)
, frame_(frame)
, symbol_(symbol)
, blocks_(arena) {
    
}

void InstructionFunction::PrintTo(base::PrintingWriter *printer) const {
    printer->Println("%s:", symbol()->data());
    for (auto block : blocks()) {
        block->PrintTo(printer);
    }
}

//ReloactionOperand *InstructionFunction::AddArrayElementClassSymbol(const ir::ArrayModel *ar, bool fetch_address) {
//    return AddClassSymbol(ar->element_type(), fetch_address);
//}
//
//ReloactionOperand *InstructionFunction::AddClassSymbol(const ir::Type &ty, bool fetch_address) {
//    int rtty_id = 0;
//
//    switch (ty.kind()) {
//        case ir::Type::kString: {
//            const std::string_view buf = "_yalx_Zplang_Zolang_ZdString$class";
//            if (auto rel = FindExternalSymbolOrNull(buf)) {
//                return rel;
//            }
//            auto symbol = String::New(arena_, buf);
//            auto rel = new (arena_) ReloactionOperand(symbol, nullptr, fetch_address);
//            return InsertExternalSymbol(symbol->ToSlice(), rel);
//        } break;
//        case ir::Type::kValue:
//        case ir::Type::kReference: {
//            auto clazz = ty.model();
//            switch (clazz->declaration()) {
//                case ir::Model::kClass:
//                case ir::Model::kStruct:
//                case ir::Model::kEnum: {
//                    std::string buf;
//                    LinkageSymbols::Build(&buf, clazz->full_name()->ToSlice());
//                    buf.append("$class");
//                    if (auto rel = FindExternalSymbolOrNull(buf)) {
//                        return rel;
//                    }
//                    auto symbol = String::New(arena_, buf);
//                    auto rel = new (arena_) ReloactionOperand(symbol, nullptr, fetch_address);
//                    return InsertExternalSymbol(symbol->ToSlice(), rel);
//                } break;
//                    
//                case ir::Model::kArray: {
//                    auto array_ty = down_cast<ir::ArrayModel>(clazz);
//                    auto offset = array_ty->dimension_count() > 1
//                        ? Type_multi_dims_array * sizeof(yalx_class)
//                        : Type_array * sizeof(yalx_class);
//                    return new (arena_) ReloactionOperand(kRt_builtin_classes, static_cast<int>(offset), fetch_address);
//                } break;
//                
//                case ir::Model::kChannel:
//                case ir::Model::kFunction:
//                case ir::Model::kInterface:
//                    DCHECK(!"TODO");
//                    break;
//                    
//                default:
//                    UNREACHABLE();
//                    break;
//            }
//        } break;
//            
//        case ir::Type::kInt8:
//            rtty_id = Type_i8;
//            break;
//            
//        case ir::Type::kUInt8:
//        case ir::Type::kWord8:
//            rtty_id = Type_u8;
//            break;
//        
//        case ir::Type::kInt16:
//            rtty_id = Type_i16;
//            break;
//            
//        case ir::Type::kUInt16:
//        case ir::Type::kWord16:
//            rtty_id = Type_u16;
//            break;
//            
//        case ir::Type::kInt32:
//            rtty_id = Type_i32;
//            break;
//            
//        case ir::Type::kUInt32:
//        case ir::Type::kWord32:
//            rtty_id = Type_u32;
//            break;
//            
//        case ir::Type::kInt64:
//            rtty_id = Type_i64;
//            break;
//            
//        case ir::Type::kUInt64:
//        case ir::Type::kWord64:
//            rtty_id = Type_u64;
//            break;
//            
//        case ir::Type::kFloat32:
//            rtty_id = Type_f32;
//            break;
//            
//        case ir::Type::kFloat64:
//            rtty_id = Type_f64;
//            break;
//
//        default:
//            UNREACHABLE();
//            break;
//    }
//    if (auto rel = FindExternalSymbolOrNull(ty.ToString())) {
//        return rel;
//    }
//    auto offset = rtty_id * sizeof(yalx_class);
//    auto rel = new (arena_) ReloactionOperand(kRt_builtin_classes, static_cast<int>(offset), fetch_address);
//    return InsertExternalSymbol(ty.ToString(), rel);
//}

InstructionBlock::InstructionBlock(base::Arena *arena, InstructionFunction *owns, int id, int label)
: arena_(arena)
, owns_(owns)
, successors_(arena)
, predecessors_(arena)
, instructions_(arena)
, loop_end_nodes_(arena)
, id_(id)
, label_(label) {
    
}

int InstructionBlock::GetLowerId() const { return instructions().front()->id(); }

int InstructionBlock::GetUpperId() const {
    int upper_id = instructions().back()->id();
    for (auto end : loop_end_nodes_) {
        upper_id = std::max(end->instructions().back()->id(), upper_id);
    }
    return upper_id;
}

void InstructionBlock::PrintTo(base::PrintingWriter *printer) const {
    printer->Println("L%d:", label());
    for (auto instr : instructions()) {
        instr->PrintTo(1, printer);
    }
}

} // namespace yalx
