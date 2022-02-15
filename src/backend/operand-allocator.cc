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
        for (auto user : blk->phi_node_users()) {
            Alive(user.phi, position);
        }
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

InstructionOperand *OperandAllocator::Allocate(ir::Type ty) {
    switch (ty.kind()) {
        case ir::Type::kValue:
            if (ty.IsPointer()) {
                return Allocate(kPtr, kPointerSize, ty.model());
            } else {
                return Allocate(kVal, ty.ReferenceSizeInBytes(), ty.model());
            }
            break;
            
        case ir::Type::kString:
        case ir::Type::kReference:
            return Allocate(kRef, kPointerSize, ty.model());
            
        case ir::Type::kVoid:
            UNREACHABLE();
            break;
            
        default:
            return Allocate(kVal, ty.ReferenceSizeInBytes(), ty.model());
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

LocationOperand *OperandAllocator::AllocateStackSlot(ir::Type ty, StackSlotAllocator::Policy policy) {
    LocationOperand *slot = nullptr;
    switch (ty.kind()) {
        case ir::Type::kValue:
            if (ty.IsPointer()) {
                slot = AllocateStackSlot(kPtr, kPointerSize, policy);
            } else {
                slot = AllocateStackSlot(kVal, ty.model()->PlacementSizeInBytes(), policy, ty.model());
            }
            break;
        case ir::Type::kString:
        case ir::Type::kReference:
            slot = AllocateStackSlot(kRef, kPointerSize, policy);
            break;
        default:
            slot = AllocateStackSlot(kVal, ty.bytes(), policy);
            break;
    }
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
        if (reg->IsGeneralRegister()) {
            active_general_registers_[reg->register_id()].val = value;
        } else if (reg->IsFloatRegister() || reg->IsDoubleRegister()) {
            active_float_registers_[reg->register_id()].val = value;
        }
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
    RegisterOperand *opd = nullptr;
    switch (mark) {
        case kPtr:
        case kRef:
            opd = registers_.AllocateRegister(registers_.conf()->rep_of_ptr(), designate);
            break;
        case kVal:
            opd = registers_.AllocateRegister(ToMachineRepresentation(size), designate);
            break;
        case kF32:
            opd = registers_.AllocateRegister(MachineRepresentation::kFloat32, designate);
            break;
        case kF64:
            opd = registers_.AllocateRegister(MachineRepresentation::kFloat64, designate);
            break;
        default:
            UNREACHABLE();
            return nullptr;
    }
    if (opd) {
        if (opd->IsGeneralRegister()) {
            active_general_registers_[opd->register_id()] = {
                .val = nullptr,
                .opd = opd,
                .bak = nullptr,
            };
        } else if (opd->IsFloatRegister() || opd->IsDoubleRegister()) {
            active_float_registers_[opd->register_id()] = {
                .val = nullptr,
                .opd = opd,
                .bak = nullptr,
            };
        }
    }
    return opd;
}

void OperandAllocator::Free(InstructionOperand *operand) {
    switch (operand->kind()) {
        case InstructionOperand::kRegister: {
            auto reg = operand->AsRegister();
            registers()->FreeRegister(reg);
            if (reg->IsGeneralRegister()) {
                active_general_registers_.erase(reg->register_id());
            } else if (reg->IsFloatRegister() || reg->IsDoubleRegister()) {
                active_float_registers_.erase(reg->register_id());
            }
        } break;
        case InstructionOperand::kLocation:
            slots()->FreeSlot(operand->AsLocation());
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
                slots_.FreeSlot(operand->AsLocation());
            } else if (operand->IsRegister()) {
                registers_.FreeRegister(operand->AsRegister());
            } else {
                UNREACHABLE();
            }
        }
    }
}

RegisterSavingScope::RegisterSavingScope(OperandAllocator *allocator, MoveCallback &&callback)
: allocator_(allocator)
, move_callback_(std::move(callback)) {
}

RegisterSavingScope::~RegisterSavingScope() {
    Exit();
}

void RegisterSavingScope::AddExclude(ir::Value *exclude, int designate, int position) {
    assert(position >= 0);
    assert(position + 1 < allocator_->live_ranges_.size());
    auto dead = allocator_->dead_records_[position + 1];
    for (int i = dead.index; i < dead.index + dead.size; i++) {
        if (exclude == allocator_->deads_[i]) {
            if (!exclude->type().IsFloating()) {
                general_exclude_.insert(designate);
            } else {
                float_exclude_.insert(designate);
            }
            return;
        }
    }
    
    auto opd = allocator_->Allocated(exclude);
    if (opd) {
        if (auto reg = opd->AsRegister()) {
            if (reg->register_id() == designate) {
                if (!exclude->type().IsFloating()) {
                    general_exclude_.insert(designate);
                } else {
                    float_exclude_.insert(designate);
                }
                return;
            }
        }
    }
}

void RegisterSavingScope::SaveAll() {
    //std::map<ir::Value *, InstructionOperand *> incoming_move;
    for (auto [rid, rd] : allocator_->active_general_registers_) {
        if (general_exclude_.find(rid) != general_exclude_.end()) {
            continue;
        }
        auto bak = allocator_->AllocateStackSlot(rd.val->type(), StackSlotAllocator::kFit);
        move_callback_(bak, rd.opd, rd.val);
        backup_.push_back({rd.val, bak, rd.opd});
    }
    for (auto [rid, rd] : allocator_->active_float_registers_) {
        if (float_exclude_.find(rid) != float_exclude_.end()) {
            continue;
        }
        auto bak = allocator_->AllocateStackSlot(rd.val->type(), StackSlotAllocator::kFit);
        move_callback_(bak, rd.opd, rd.val);
        backup_.push_back({rd.val, bak, rd.opd});
    }
    for (auto bak : backup_) {
        allocator_->Move(bak.val, bak.current);
    }
}

void RegisterSavingScope::Exit() {
    for (auto bak : backup_) {
        auto opd = DCHECK_NOTNULL(allocator_->AllocateReigster(bak.val, bak.old->register_id()));
        move_callback_(opd, bak.current, bak.val);
        allocator_->Move(bak.val, opd);
    }
}

RegisterPreemptiveScope::RegisterPreemptiveScope(OperandAllocator *allocator)
: allocator_(allocator) {
    Enter();
}

RegisterPreemptiveScope::~RegisterPreemptiveScope() {
    Exit();
}

RegisterOperand *RegisterPreemptiveScope::Preempt(ir::Value *val, int designate) {
    UNREACHABLE();
    // TODO:
    return nullptr;
}

void RegisterPreemptiveScope::Enter() {
    
}

void RegisterPreemptiveScope::Exit() {
    
}

} // namespace backend
} // namespace yalx
