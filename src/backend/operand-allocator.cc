#include "backend/operand-allocator.h"
#include "ir/metadata.h"
#include "ir/operator.h"
#include "ir/type.h"
#include "ir/node.h"

namespace yalx {
namespace backend {


OperandAllocator::OperandAllocator(const StackConfiguration *sconf,
                                   const RegisterConfiguration *rconf,
                                   Policy policy,
                                   base::Arena *arena)
: policy_(policy)
, arena_(arena)
, slots_(sconf, arena)
, registers_(rconf, arena) {
    
}

void OperandAllocator::Prepare(ir::Function *fun) {
    int position = 0;
    for (auto blk : fun->blocks()) {
        for (auto instr : blk->instructions()) {
            if (instr->type().kind() != ir::Type::kVoid) {
                Alive(instr, position);
            }
            for (int i = 0; i < instr->op()->value_in(); i++) {
                Alive(instr->InputValue(i), position);
            }
            position++;
        }
    }
    const auto total = position;
    for (int64_t i = fun->blocks_size() - 1; i >= 0; i--) {
        auto blk = fun->block(i);
        for (int64_t j = blk->instructions_size() - 1; j >= 0; j--) {
            auto instr = blk->instruction(j);
            if (instr->type().kind() != ir::Type::kVoid) {
                Dead(instr, position);
            }
            for (int k = 0; k < instr->op()->value_in(); k++) {
                Dead(instr->InputValue(k), position);
            }
            position--;
        }
    }
    assert(position == 0);
    std::unique_ptr<std::vector<ir::Value *>[]> deads(new std::vector<ir::Value *>[total + 1]);
    for (auto [val, range] : live_ranges_) {
        assert(range.stop_position >= 0);
        assert(range.stop_position < total + 1);
        
        deads[range.stop_position].push_back(val);
    }
    
    LiveRecord rd;
    rd.index = 0;
    rd.size  = 0;
    for (int i = 0; i < total + 1; i++) {
        rd.size = deads[i].size();
        dead_records_.push_back(rd);
        for (auto val : deads[i]) { deads_.push_back(val); }
        rd.index += rd.size;
    }
}

InstructionOperand *OperandAllocator::Allocate(ir::Value *value) {
    switch (policy_) {
        case kStackOnly:
            return AllocateStackSlot(value, StackSlotAllocator::kFit);
            
        case kRegisterFirst:
            return TryAllocateRegisterFirst(value);

        default:
            UNREACHABLE();
            break;
    }
    return nullptr;
}

InstructionOperand *OperandAllocator::Allocate(OperandMark mark, size_t size, ir::Model *model) {
    if (policy_ == kStackOnly) {
        return AllocateStackSlot(mark, size, StackSlotAllocator::kFit, model);
    }
    switch (mark) {
        case kVal: {
            if (size > 8 || model) {
                return AllocateStackSlot(mark, size, StackSlotAllocator::kFit, model);
            }
            if (auto opd = AllocateReigster(mark, size)) {
                return opd;
            }
            return AllocateStackSlot(mark, size, StackSlotAllocator::kFit, model);
        } break;
        case kPtr:
        case kF32:
        case kF64:
        case kRef: {
            if (auto opd = AllocateReigster(mark, size)) {
                return opd;
            }
            return AllocateStackSlot(mark, size, StackSlotAllocator::kFit, model);
        } break;
        default:
            UNREACHABLE();
            break;
    }
}

InstructionOperand *OperandAllocator::TryAllocateRegisterFirst(ir::Value *value) {
    if (value->type().kind() == ir::Type::kValue && !value->type().IsPointer()) {
        return AllocateStackSlot(value, StackSlotAllocator::kFit);
    }
    if (auto opd = AllocateReigster(value)) {
        return opd;
    }
    return AllocateStackSlot(value, StackSlotAllocator::kFit);
}

LocationOperand *OperandAllocator::AllocateStackSlot(ir::Value *value, StackSlotAllocator::Policy policy) {
    LocationOperand *slot = nullptr;
    switch (value->type().kind()) {
        case ir::Type::kValue:
            if (value->type().IsPointer()) {
                slot = AllocateStackSlot(kPtr, kPointerSize, policy);
            } else {
                slot = AllocateStackSlot(kVal, value->type().model()->PlacementSizeInBytes(), policy,
                                         value->type().model());
            }
            break;
        case ir::Type::kString:
        case ir::Type::kReference:
            slot = AllocateStackSlot(kRef, kPointerSize, policy);
            break;
        default:
            slot = AllocateStackSlot(kVal, value->type().bytes(), policy);
            break;
    }
    allocated_[value] = slot;
    return slot;
}

LocationOperand *OperandAllocator::AllocateStackSlot(OperandMark mark, size_t size, StackSlotAllocator::Policy policy,
                                                     ir::Model *model) {
    switch (mark) {
        case kPtr:
            return slots_.AllocateValSlot(kPointerSize, policy, model);
        case kVal:
        case kF32:
        case kF64:
            return slots_.AllocateValSlot(size, policy, model);
        case kRef:
            return slots_.AllocateRefSlot(policy);
        default:
            UNREACHABLE();
            break;
    }
}

RegisterOperand *OperandAllocator::AllocateReigster(ir::Value *value, int designate) {
    RegisterOperand *reg = nullptr;
    if (value->type().IsPointer()) {
        reg = AllocateReigster(kPtr, kPointerSize, designate);
    } else if (value->type().IsReference()) {
        reg = AllocateReigster(kRef, kPointerSize, designate);
    } else if (value->type().IsFloating()) {
        reg = AllocateReigster(value->type().bits() == 32 ? kF32 : kF64, value->type().bytes(), designate);
    } else if (value->type().kind() == ir::Type::kValue || value->type().kind() == ir::Type::kVoid) {
        return nullptr;
    } else {
        reg = AllocateReigster(kVal, value->type().bytes(), designate);
    }
    if (reg) {
        allocated_[value] = reg;
    }
    return reg;
}

static MachineRepresentation ToMachineRepresentation(size_t size) {
    switch (size) {
        case 1:
            return MachineRepresentation::kWord8;
        case 2:
            return MachineRepresentation::kWord16;
        case 4:
            return MachineRepresentation::kWord32;
        case 8:
            return MachineRepresentation::kWord64;
        default:
            UNREACHABLE();
            break;
    }
    return MachineRepresentation::kNone;
}

RegisterOperand *OperandAllocator::AllocateReigster(OperandMark mark, size_t size, int designate) {
    switch (mark) {
        case kPtr:
        case kRef:
            return registers_.AllocateRegister(registers_.conf()->rep_of_ptr(), designate);
        case kVal:
            return registers_.AllocateRegister(ToMachineRepresentation(size), designate);
        case kF32:
            return registers_.AllocateRegister(MachineRepresentation::kFloat32, designate);
        case kF64:
            return registers_.AllocateRegister(MachineRepresentation::kFloat64, designate);
        default:
            UNREACHABLE();
            break;
    }
    return nullptr;
}

void OperandAllocator::Free(InstructionOperand *operand) {
    switch (operand->kind()) {
        case InstructionOperand::kRegister:
            registers()->FreeRegister(static_cast<RegisterOperand *>(operand));
            break;
        case InstructionOperand::kLocation:
            slots()->FreeSlot(static_cast<LocationOperand *>(operand));
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void OperandAllocator::ReleaseDeads(int position) {
    assert(position >= 0);
    assert(position < dead_records_.size());
    
    auto rd = dead_records_[position];
    for (size_t i = rd.index; i < rd.index + rd.size; i++) {
        auto key = deads_[i];
        if (auto iter = allocated_.find(key); iter != allocated_.end()) {
            auto operand = iter->second;
            if (operand->IsLocation()) {
                slots_.FreeSlot(static_cast<LocationOperand *>(operand));
            } else if (operand->IsRegister()) {
                // TODO:
                UNREACHABLE();
            } else {
                UNREACHABLE();
            }
        }
    }
}

} // namespace backend
} // namespace yalx
