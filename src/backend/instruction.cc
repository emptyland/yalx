#include "backend/instruction.h"
#include "backend/linkage-symbols.h"
#include "ir/metadata.h"
#include "ir/type.h"
#include "compiler/constants.h"
#include "runtime/object/type.h"
#include "base/checking.h"

namespace yalx {
namespace backend {


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
, operands_(arena) {
    operands_.resize(input_count, InstructionOperand::kInvliadVirtualRegister);
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

InstructionFunction::InstructionFunction(base::Arena *arena, const String *symbol, Frame *frame)
: arena_(arena)
, frame_(frame)
, symbol_(symbol)
, blocks_(arena) {
    
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

InstructionBlock::InstructionBlock(base::Arena *arena, InstructionFunction *owns, int label)
: arena_(arena)
, owns_(owns)
, successors_(arena)
, predecessors_(arena)
, instructions_(arena)
, phis_(arena)
, label_(label) {
    
}

} // namespace backend
} // namespace yalx
