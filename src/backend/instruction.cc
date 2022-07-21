#include "backend/instruction.h"
#include "backend/linkage-symbols.h"
#include "ir/metadata.h"
#include "ir/type.h"
#include "compiler/constants.h"
#include "runtime/object/type.h"
#include "base/checking.h"

namespace yalx {
namespace backend {


bool RegisterOperand::IsGeneralRegister() const {
    switch (rep()) {
        case MachineRepresentation::kWord8:
        case MachineRepresentation::kWord16:
        case MachineRepresentation::kWord32:
        case MachineRepresentation::kWord64:
            return true;
            
        default:
            return false;
    }
}

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
        case kLocation: {
            auto lhs = AsLocation();
            auto rhs = other->AsLocation();
            return lhs->mode() == rhs->mode() &&
                lhs->register0_id() == rhs->register0_id() &&
                lhs->register1_id() == rhs->register1_id() &&
                lhs->k() == rhs->k();
        } break;
        case kRegister: {
            auto lhs = AsRegister();
            auto rhs = other->AsRegister();
            return lhs->rep() == rhs->rep() && lhs->register_id() == rhs->register_id();
        } break;
        case kImmediate: {
            auto lhs = AsImmediate();
            auto rhs = other->AsImmediate();
            if (lhs->rep() != rhs->rep()) {
                return false;
            }
            switch (lhs->rep()) {
                case MachineRepresentation::kWord8:
                    return lhs->word8() == rhs->word8();
                case MachineRepresentation::kWord16:
                    return lhs->word16() == rhs->word16();
                case MachineRepresentation::kWord32:
                    return lhs->word32() == rhs->word32();
                case MachineRepresentation::kWord64:
                    return lhs->word64() == rhs->word64();
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
        case kReloaction: {
            auto lhs = AsReloaction();
            auto rhs = other->AsReloaction();
            return lhs->label() == rhs->label() && lhs->symbol_name() == rhs->symbol_name() &&
                lhs->fetch_address() == rhs->fetch_address();
        } break;
        default:
            UNREACHABLE();
            break;
    }
}

ReloactionOperand *ReloactionOperand::OffsetOf(base::Arena *arena, int offset) const {
    ReloactionOperand *copied = new (arena) ReloactionOperand(symbol_name(), label(), fetch_address());
    copied->offset_ = offset;
    return copied;
}

Instruction::Instruction(Code op, size_t inputs_count, size_t outputs_count, size_t temps_count, Operand *operands[])
: op_(op)
, inputs_count_(inputs_count)
, outputs_count_(outputs_count)
, temps_count_(temps_count) {
    auto bytes = operands_size() * sizeof(Operand *);
    ::memcpy(operands_, operands, bytes);
}

Instruction *Instruction::New(base::Arena *arena, Code op, Operand *operands[], size_t inputs_count,
                              size_t outputs_count, size_t temps_count) {
    auto chunk = AllocatePlacementMemory(arena, inputs_count, outputs_count, temps_count);
    return new (chunk) Instruction(op, inputs_count, outputs_count, temps_count, operands);
}

void *Instruction::AllocatePlacementMemory(base::Arena *arena, size_t inputs_count, size_t outputs_count,
                                           size_t temps_count) {
    size_t in_memory_bytes = sizeof(Instruction) + (inputs_count + outputs_count + temps_count) * sizeof(Operand *);
    return arena->Allocate(in_memory_bytes);
}

InstructionFunction::InstructionFunction(base::Arena *arena, const String *symbol)
: arena_(arena)
, symbol_(symbol)
, external_symbols_(arena)
, blocks_(arena) {
    
}

ReloactionOperand *InstructionFunction::AddArrayElementClassSymbol(const ir::ArrayModel *ar, bool fetch_address) {
    return AddClassSymbol(ar->element_type(), fetch_address);
}

ReloactionOperand *InstructionFunction::AddClassSymbol(const ir::Type &ty, bool fetch_address) {
    int rtty_id = 0;

    switch (ty.kind()) {
        case ir::Type::kString: {
            const std::string_view buf = "_yalx_Zplang_Zolang_ZdString$class";
            if (auto rel = FindExternalSymbolOrNull(buf)) {
                return rel;
            }
            auto symbol = String::New(arena_, buf);
            auto rel = new (arena_) ReloactionOperand(symbol, nullptr, fetch_address);
            return InsertExternalSymbol(symbol->ToSlice(), rel);
        } break;
        case ir::Type::kValue:
        case ir::Type::kReference: {
            auto clazz = ty.model();
            switch (clazz->declaration()) {
                case ir::Model::kClass:
                case ir::Model::kStruct:
                case ir::Model::kEnum: {
                    std::string buf;
                    LinkageSymbols::Build(&buf, clazz->full_name()->ToSlice());
                    buf.append("$class");
                    if (auto rel = FindExternalSymbolOrNull(buf)) {
                        return rel;
                    }
                    auto symbol = String::New(arena_, buf);
                    auto rel = new (arena_) ReloactionOperand(symbol, nullptr, fetch_address);
                    return InsertExternalSymbol(symbol->ToSlice(), rel);
                } break;
                    
                case ir::Model::kArray: {
                    auto array_ty = down_cast<ir::ArrayModel>(clazz);
                    auto offset = array_ty->dimension_count() > 1
                        ? Type_multi_dims_array * sizeof(yalx_class)
                        : Type_array * sizeof(yalx_class);
                    return new (arena_) ReloactionOperand(kRt_builtin_classes, static_cast<int>(offset), fetch_address);
                } break;
                
                case ir::Model::kChannel:
                case ir::Model::kFunction:
                case ir::Model::kInterface:
                    DCHECK(!"TODO");
                    break;
                    
                default:
                    UNREACHABLE();
                    break;
            }
        } break;
            
        case ir::Type::kInt8:
            rtty_id = Type_i8;
            break;
            
        case ir::Type::kUInt8:
        case ir::Type::kWord8:
            rtty_id = Type_u8;
            break;
        
        case ir::Type::kInt16:
            rtty_id = Type_i16;
            break;
            
        case ir::Type::kUInt16:
        case ir::Type::kWord16:
            rtty_id = Type_u16;
            break;
            
        case ir::Type::kInt32:
            rtty_id = Type_i32;
            break;
            
        case ir::Type::kUInt32:
        case ir::Type::kWord32:
            rtty_id = Type_u32;
            break;
            
        case ir::Type::kInt64:
            rtty_id = Type_i64;
            break;
            
        case ir::Type::kUInt64:
        case ir::Type::kWord64:
            rtty_id = Type_u64;
            break;
            
        case ir::Type::kFloat32:
            rtty_id = Type_f32;
            break;
            
        case ir::Type::kFloat64:
            rtty_id = Type_f64;
            break;

        default:
            UNREACHABLE();
            break;
    }
    if (auto rel = FindExternalSymbolOrNull(ty.ToString())) {
        return rel;
    }
    auto offset = rtty_id * sizeof(yalx_class);
    auto rel = new (arena_) ReloactionOperand(kRt_builtin_classes, static_cast<int>(offset), fetch_address);
    return InsertExternalSymbol(ty.ToString(), rel);
}

InstructionBlock::InstructionBlock(base::Arena *arena, InstructionFunction *owns, int label)
: arena_(arena)
, owns_(owns)
, successors_(arena)
, predecessors_(arena)
, instructions_(arena)
, label_(label) {
    
}

Instruction *InstructionBlock::New(Instruction::Code op) {
    auto instr = Instruction::New(arena_, op);
    instructions_.push_back(instr);
    return instr;
}

Instruction *InstructionBlock::NewI(Instruction::Code op, Instruction::Operand *input) {
    auto instr = Instruction::New(arena_, op, &input, 1/*inputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

Instruction *InstructionBlock::NewO(Instruction::Code op, Instruction::Operand *output) {
    auto instr = Instruction::New(arena_, op, &output, 0/*inputs_count*/, 1/*outputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

Instruction *InstructionBlock::NewIO(Instruction::Code op, Instruction::Operand *io, Instruction::Operand *input) {
    InstructionOperand *operands[2] = {input, io};
    auto instr = Instruction::New(arena_, op, operands, 1/*inputs_count*/, 1/*outputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

Instruction *InstructionBlock::NewIO(Instruction::Code op, Instruction::Operand *output, Instruction::Operand *in1,
                                     Instruction::Operand *in2) {
    InstructionOperand *operands[] = {in1, in2, output};
    auto instr = Instruction::New(arena_, op, operands, 2/*inputs_count*/, 1/*outputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

Instruction *InstructionBlock::NewIO2(Instruction::Code op, Instruction::Operand *out1, Instruction::Operand *out2,
                                      Instruction::Operand *input) {
    InstructionOperand *operands[] = {input, out1, out2};
    auto instr = Instruction::New(arena_, op, operands, 1/*inputs_count*/, 2/*outputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

Instruction *InstructionBlock::NewI2O(Instruction::Code op, Instruction::Operand *io, Instruction::Operand *in1,
                                      Instruction::Operand *in2) {
    InstructionOperand *operands[] = {in1, in2, io};
    auto instr = Instruction::New(arena_, op, operands, 2/*inputs_count*/, 1/*outputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

Instruction *InstructionBlock::NewII(Instruction::Code op, Instruction::Operand *in1, Instruction::Operand *in2) {
    InstructionOperand *operands[] = {in1, in2};
    auto instr = Instruction::New(arena_, op, operands, 2/*inputs_count*/, 0/*outputs_count*/);
    instructions_.push_back(instr);
    return instr;
}

} // namespace backend
} // namespace yalx
