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
    std::map<ir::Value *, ir::Value *> ptr_associated_vals;
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
            // special process pointer operator
            if (instr->Is(ir::Operator::kStackAlloc)) {
                if (instr->users().size() > 0) {
                    for (auto used : instr->users()) {
                        if (used.user->Is(ir::Operator::kLoadAddress)) {
                            ptr_associated_vals[used.user] = instr;
                        }
                    }
                }
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
            
        #define PROCESS_PTR_ASSOCIATED_VAL(val) \
            if ((val)->Is(ir::Operator::kLoadAddress)) { \
                if (auto iter = ptr_associated_vals.find((val)); iter != ptr_associated_vals.end()) { \
                    DestinyDead(iter->second, position); \
                } \
            }
            for (int k = 0; k < instr->op()->value_in(); k++) {
                Dead(instr->InputValue(k), position);
                // special process pointer operator
                PROCESS_PTR_ASSOCIATED_VAL(instr->InputValue(k));
            }
            // special process pointer operator
            PROCESS_PTR_ASSOCIATED_VAL(instr);
            
        #undef PROCESS_PTR_ASSOCIATED_VAL

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
            return AllocateStackSlot(value, 0/*padding_size*/,  StackSlotAllocator::kFit);
            
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
        return AllocateStackSlot(mark, size, 0/*padding_size*/, StackSlotAllocator::kFit, model);
    }
    switch (mark) {
        case kVal: {
            if (size > 8 || model) {
                return AllocateStackSlot(mark, size, 0/*padding_size*/, StackSlotAllocator::kFit, model);
            }
            if (auto opd = AllocateReigster(mark, size)) {
                return opd;
            }
            return AllocateStackSlot(mark, size, 0/*padding_size*/, StackSlotAllocator::kFit, model);
        } break;
        case kPtr:
        case kF32:
        case kF64:
        case kRef: {
            if (auto opd = AllocateReigster(mark, size)) {
                return opd;
            }
            return AllocateStackSlot(mark, size, 0/*padding_size*/, StackSlotAllocator::kFit, model);
        } break;
        default:
            UNREACHABLE();
            break;
    }
}

InstructionOperand *OperandAllocator::TryAllocateRegisterFirst(ir::Value *value) {
    if (value->type().kind() == ir::Type::kValue && !value->type().IsPointer()) {
        return AllocateStackSlot(value, 0/*padding_size*/, StackSlotAllocator::kFit);
    }
    if (auto opd = AllocateReigster(value)) {
        return opd;
    }
    return AllocateStackSlot(value, 0/*padding_size*/, StackSlotAllocator::kFit);
}

LocationOperand *OperandAllocator::AllocateStackSlot(ir::Value *value, size_t padding_size,
                                                     StackSlotAllocator::Policy policy) {
    LocationOperand *slot = nullptr;
    switch (value->type().kind()) {
        case ir::Type::kValue:
            if (value->type().IsPointer()) {
                slot = AllocateStackSlot(kPtr, kPointerSize, padding_size, policy);
            } else {
                slot = AllocateStackSlot(kVal, value->type().model()->PlacementSizeInBytes(),
                                         padding_size,
                                         policy,
                                         value->type().model());
            }
            break;
        case ir::Type::kString:
        case ir::Type::kReference:
            slot = AllocateStackSlot(kRef, kPointerSize, padding_size, policy);
            break;
        default:
            slot = AllocateStackSlot(kVal, value->type().bytes(), padding_size, policy);
            break;
    }
    allocated_[value] = slot;
    return slot;
}

LocationOperand *OperandAllocator::AllocateStackSlot(ir::Type ty, size_t padding_size,
                                                     StackSlotAllocator::Policy policy) {
    LocationOperand *slot = nullptr;
    switch (ty.kind()) {
        case ir::Type::kValue:
            if (ty.IsPointer()) {
                slot = AllocateStackSlot(kPtr, kPointerSize, padding_size, policy);
            } else {
                slot = AllocateStackSlot(kVal, ty.model()->PlacementSizeInBytes(), padding_size, policy, ty.model());
            }
            break;
        case ir::Type::kString:
        case ir::Type::kReference:
            slot = AllocateStackSlot(kRef, kPointerSize, padding_size, policy);
            break;
        default:
            slot = AllocateStackSlot(kVal, ty.bytes(), padding_size, policy);
            break;
    }
    return slot;
}

LocationOperand *OperandAllocator::AllocateStackSlot(OperandMark mark,
                                                     size_t size,
                                                     size_t padding_size,
                                                     StackSlotAllocator::Policy policy,
                                                     ir::Model *model) {
    switch (mark) {
        case kPtr:
            return slots_.AllocateValSlot(kPointerSize, padding_size, policy, model);
        case kVal:
        case kF32:
        case kF64:
            return slots_.AllocateValSlot(size, padding_size, policy, model);
        case kRef:
            return slots_.AllocateRefSlot(padding_size, policy);
        default:
            UNREACHABLE();
            break;
    }
}

RegisterOperand *OperandAllocator::AllocateReigster(ir::Value *value, int designate) {
    RegisterOperand *reg = AllocateReigster(value->type(), designate);
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

RegisterOperand *OperandAllocator::AllocateReigster(ir::Type ty, int designate) {
    if (ty.IsPointer()) {
        return AllocateReigster(kPtr, kPointerSize, designate);
    } else if (ty.IsReference()) {
        return AllocateReigster(kRef, kPointerSize, designate);
    } else if (ty.IsFloating()) {
        return AllocateReigster(ty.bits() == 32 ? kF32 : kF64, ty.bytes(), designate);
    } else if (ty.kind() == ir::Type::kValue || ty.kind() == ir::Type::kVoid) {
        return nullptr;
    } else {
        return AllocateReigster(kVal, ty.bytes(), designate);
    }
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

void OperandAllocator::Associate(ir::Value *value, InstructionOperand *operand) {
    if (auto iter = allocated_.find(value); iter != allocated_.end()) {
        Free(iter->second);
    }
    allocated_[value] = operand;
    if (auto reg = operand->AsRegister()) {
        if (reg->IsGeneralRegister()) {
            active_general_registers_[reg->register_id()] = {
                .val = value,
                .opd = reg,
                .bak = nullptr,
            };
        } else {
            active_float_registers_[reg->register_id()] = {
                .val = value,
                .opd = reg,
                .bak = nullptr,
            };
        }
    }
}

InstructionOperand *OperandAllocator::LinkTo(ir::Value *value, InstructionOperand *operand) {
    InstructionOperand *old = nullptr;
    if (auto iter = allocated_.find(value); iter != allocated_.end()) {
        old = iter->second;
    }
    if (old && old->Equals(operand)) {
        return nullptr;
    }
    allocated_[value] = operand;
    return old;
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

bool OperandAllocator::WillBeDead(ir::Value *value, int position) const {
    assert(position >= 0);
    assert(position < dead_records_.size());
    auto rd = dead_records_[position];
    for (size_t i = rd.index; i < rd.index + rd.size; i++) {
        if (value == deads_[i]) {
            return true;
        }
    }
    return false;
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

OperandAllocator::BorrowedRecord OperandAllocator::BorrowRegister(ir::Value *value, InstructionOperand *bak,
                                                                  int designate/* = kAnyRegister*/) {
    auto brd = BorrowRegister(value->type(), bak, designate);
    Associate(value, brd.target);
    return brd;
}

OperandAllocator::BorrowedRecord OperandAllocator::BorrowRegister(ir::Type ty, InstructionOperand *bak,
                                                                  int designate/* = kAnyRegister*/) {
    constexpr static const BorrowedRecord kNoBorrowed = {nullptr, nullptr, nullptr, nullptr};
    RegisterRecord active_rd;
    const auto rep = ToMachineRepresentation(ty);
    if (designate != kAnyRegister) {
        if (ty.IsFloating()) {
            auto iter = active_float_registers_.find(designate);
            if (iter == active_float_registers_.end()) {
                return kNoBorrowed;
            }
            active_rd = iter->second;
        } else {
            auto iter = active_general_registers_.find(designate);
            if (iter == active_general_registers_.end()) {
                return kNoBorrowed;
            }
            active_rd = iter->second;
        }
    } else {
        if (ty.IsFloating()) {
            if (active_float_registers_.empty()) {
                return kNoBorrowed;
            }
            auto iter = active_float_registers_.begin();
            active_rd = iter->second;
        } else {
            if (active_general_registers_.empty()) {
                return kNoBorrowed;
            }
            auto iter = active_general_registers_.begin();
            active_rd = iter->second;
        }
    }

    if (active_rd.val) {
        Associate(active_rd.val, bak);
    } else {
        Free(active_rd.opd);
    }
    return {AllocateReigster(ty), active_rd.opd, bak, active_rd.val};
}

RegisterSavingScope::RegisterSavingScope(OperandAllocator *allocator, int position, MovingDelegate *callback)
: allocator_(allocator)
, position_(position)
, moving_delegate_(callback) {
    assert(position >= 0);
    assert(position + 1 < allocator_->dead_records_.size());
}

RegisterSavingScope::~RegisterSavingScope() {
    Exit();
}

void RegisterSavingScope::AddExclude(ir::Value *exclude, int designate, int position) {
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
    moving_delegate_->Initialize();
    for (auto [rid, rd] : allocator_->active_general_registers_) {
        if (general_exclude_.find(rid) != general_exclude_.end()) {
            continue;
        }
        auto bak = allocator_->AllocateStackSlot(DCHECK_NOTNULL(rd.val)->type(), 0/*padding_size*/,
                                                 StackSlotAllocator::kFit);
        moving_delegate_->MoveTo(bak, rd.opd, rd.val->type());
        backup_.push_back({rd.val, bak, rd.opd});
    }
    for (auto [rid, rd] : allocator_->active_float_registers_) {
        if (float_exclude_.find(rid) != float_exclude_.end()) {
            continue;
        }
        
        //if (allocator_->WillBeLive(rd.val, position_ + 1)) {
        auto bak = allocator_->AllocateStackSlot(rd.val->type(), 0/*padding_size*/, StackSlotAllocator::kFit);
        moving_delegate_->MoveTo(bak, rd.opd, rd.val->type());
        backup_.push_back({rd.val, bak, rd.opd});
        //}
    }
    for (auto bak : backup_) {
        if (bak.val) {
            allocator_->Associate(bak.val, bak.current);
        } else {
            allocator_->Free(bak.old);
        }
    }
    moving_delegate_->Finalize();
}

void RegisterSavingScope::Exit() {
    moving_delegate_->Initialize();
    for (auto bak : backup_) {
        DCHECK_NOTNULL(bak.val);
        if (allocator_->WillBeLive(bak.val, position_ + 1)) {
            auto opd = allocator_->Allocate(bak.val->type());
            moving_delegate_->MoveTo(opd, bak.current, bak.val->type());
            allocator_->Associate(bak.val, opd);
        }
    }
    moving_delegate_->Finalize();
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
