#include "backend/register-allocator.h"
#include "ir/metadata.h"
#include "ir/operator.h"
#include "ir/type.h"
#include "ir/node.h"


namespace yalx {

namespace backend {

RegisterConfiguration::RegisterConfiguration(
    int id_of_fp, int id_of_sp, MachineRepresentation rep_of_ptr,
    int number_of_general_registers,
    int number_of_float_registers,
    int number_of_double_registers,
    const int *allocatable_general_registers,
    size_t number_of_allocatable_general_registers,
    const int *allocatable_float_registers,
    size_t number_of_allocatable_float_registers,
    const int *allocatable_double_registers,
    size_t number_of_allocatable_double_registers)
: id_of_fp_(id_of_fp)
, id_of_sp_(id_of_sp)
, rep_of_ptr_(rep_of_ptr)
, number_of_general_registers_(number_of_general_registers)
, number_of_float_registers_(number_of_float_registers)
, number_of_double_registers_(number_of_double_registers)
, number_of_allocatable_general_registers_(static_cast<int>(number_of_allocatable_general_registers))
, number_of_allocatable_float_registers_(static_cast<int>(number_of_allocatable_float_registers))
, number_of_allocatable_double_registers_(static_cast<int>(number_of_allocatable_double_registers))
, allocatable_general_registers_(allocatable_general_registers)
, allocatable_float_registers_(allocatable_float_registers)
, allocatable_double_registers_(allocatable_double_registers) {
    
}

RegisterAllocator::RegisterAllocator(const RegisterConfiguration *conf, base::Arena *arena)
: arena_(arena)
, conf_(conf) {
    stack_pointer_ = new (arena) RegisterOperand(conf->id_of_sp(), conf->rep_of_ptr());
    frame_pointer_ = new (arena) RegisterOperand(conf->id_of_fp(), conf->rep_of_ptr());
    for (int i = 0; i < conf->number_of_allocatable_general_registers(); i++) {
        general_pool_.insert(conf->allocatable_general_register(i));
    }
    for (int i = 0; i < conf->number_of_allocatable_float_registers(); i++) {
        float_pool_.insert(conf->allocatable_float_register(i));
    }
    for (int i = 0; i < conf->number_of_allocatable_double_registers(); i++) {
        double_pool_.insert(conf->allocatable_double_register(i));
    }
}

RegisterOperand *RegisterAllocator::AllocateRegister(MachineRepresentation rep, int designate) {
    switch (rep) {
        case MachineRepresentation::kWord8:
        case MachineRepresentation::kWord16:
        case MachineRepresentation::kWord32:
        case MachineRepresentation::kWord64:
            return Allocate(&general_pool_, rep, designate);
        case MachineRepresentation::kFloat32:
            return Allocate(&float_pool_, rep, designate);
        case MachineRepresentation::kFloat64:
            return Allocate(&double_pool_, rep, designate);
        case MachineRepresentation::kNone:
        case MachineRepresentation::kBit:
        default:
            UNREACHABLE();
            break;
    }
}

void RegisterAllocator::FreeRegister(RegisterOperand *reg) {
    switch (reg->rep()) {
        case MachineRepresentation::kWord8:
        case MachineRepresentation::kWord16:
        case MachineRepresentation::kWord32:
        case MachineRepresentation::kWord64:
            general_pool_.insert(reg->register_id());
            break;
        case MachineRepresentation::kFloat32:
            float_pool_.insert(reg->register_id());
            break;
        case MachineRepresentation::kFloat64:
            double_pool_.insert(reg->register_id());
            break;
        case MachineRepresentation::kNone:
        case MachineRepresentation::kBit:
        default:
            UNREACHABLE();
            break;
    }
}

RegisterOperand *RegisterAllocator::Allocate(std::set<int> *pool, MachineRepresentation rep, int designate) {
    if (designate != kAny) {
        if (auto iter = pool->find(designate); iter != pool->end()) {
            pool->erase(iter);
            return new (arena_) RegisterOperand(designate, rep);
        } else {
            return nullptr;
        }
    } else {
        if (!pool->empty()) {
            auto iter = pool->begin();
            pool->erase(iter);
            return new (arena_) RegisterOperand(*iter, rep);
        } else {
            return nullptr;
        }
    }
}

} // namespace backend

} // namespace yalx
