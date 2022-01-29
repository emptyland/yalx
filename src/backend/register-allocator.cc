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

RegisterAllocator::RegisterAllocator(base::Arena *arena, RegisterConfiguration *conf)
: arena_(arena)
, conf_(conf) {
    stack_pointer_ = new (arena) RegisterOperand(conf->id_of_sp(), conf->rep_of_ptr());
    frame_pointer_ = new (arena) RegisterOperand(conf->id_of_fp(), conf->rep_of_ptr());
}

void RegisterAllocator::Prepare(ir::Function *fun) {
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

    for (int64_t i = fun->blocks_size() - 1; i >= 0; i--) {
        auto blk = fun->block(i);
        for (int64_t j = blk->instructions_size() - 1; j >= 0; j--) {
            auto instr = blk->instruction(j);
            if (instr->type().kind() != ir::Type::kVoid) {
                Dead(instr, position);
            }
            for (int k = 0; k < instr->op()->value_in(); k++) {
                Alive(instr->InputValue(k), position);
            }
            position--;
        }
    }
    assert(position == 0);
}

} // namespace backend

} // namespace yalx
