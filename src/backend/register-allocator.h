#pragma once
#ifndef YALX_BACKEND_REGISTER_ALLOCATOR_H_
#define YALX_BACKEND_REGISTER_ALLOCATOR_H_

#include "backend/instruction.h"
#include "base/arena-utils.h"
#include "base/base.h"

namespace yalx {
namespace ir {
class Function;
class Value;
} // namespace ir
namespace backend {

class RegisterConfiguration final {
public:
    RegisterConfiguration(int id_of_fp, int id_of_sp, MachineRepresentation rep_of_ptr,
                          int number_of_general_registers,
                          int number_of_float_registers,
                          int number_of_double_registers,
                          const int *allocatable_general_registers,
                          size_t number_of_allocatable_general_registers,
                          const int *allocatable_float_registers,
                          size_t number_of_allocatable_float_registers,
                          const int *allocatable_double_registers,
                          size_t number_of_allocatable_double_registers);
    
    DEF_VAL_GETTER(int, id_of_fp);
    DEF_VAL_GETTER(int, id_of_sp);
    DEF_VAL_GETTER(MachineRepresentation, rep_of_ptr);
    DEF_VAL_GETTER(int, number_of_general_registers);
    DEF_VAL_GETTER(int, number_of_float_registers);
    DEF_VAL_GETTER(int, number_of_double_registers);
    DEF_VAL_GETTER(int, number_of_allocatable_general_registers);
    DEF_VAL_GETTER(int, number_of_allocatable_float_registers);
    DEF_VAL_GETTER(int, number_of_allocatable_double_registers);
    
    int allocatable_general_register(int i) const {
        assert(i >= 0 && i < number_of_allocatable_general_registers());
        return allocatable_general_registers_[i];
    }
    
    int allocatable_float_register(int i) const {
        assert(i >= 0 && i < number_of_allocatable_float_registers());
        return allocatable_float_registers_[i];
    }
    
    int allocatable_double_register(int i) const {
        assert(i >= 0 && i < number_of_allocatable_float_registers());
        return allocatable_float_registers_[i];
    }
private:
    int id_of_fp_ = -1;
    int id_of_sp_ = -1;
    MachineRepresentation rep_of_ptr_ = MachineRepresentation::kNone;
    int number_of_general_registers_ = -1;
    int number_of_float_registers_ = -1;
    int number_of_double_registers_ = -1;
    int number_of_allocatable_general_registers_ = -1;
    int number_of_allocatable_float_registers_ = -1;
    int number_of_allocatable_double_registers_ = -1;
    const int *const allocatable_general_registers_ = nullptr;
    const int *const allocatable_float_registers_ = nullptr;
    const int *const allocatable_double_registers_ = nullptr;
}; // class RegisterConfiguration


// Initial set
// [0] r0 r1 r2 r3
// [1]    r1 r2 r3
// [2]    r1    r3

class RegisterAllocator final {
public:
    RegisterAllocator(const RegisterConfiguration *conf, base::Arena *arena);

    // SP/RSP
    DEF_PTR_GETTER(RegisterOperand, stack_pointer);
    // FP/RBP
    DEF_PTR_GETTER(RegisterOperand, frame_pointer);

private:
    base::Arena *const arena_;
    const RegisterConfiguration *const conf_;
    
    RegisterOperand *stack_pointer_ = nullptr;
    RegisterOperand *frame_pointer_ = nullptr;
}; // class RegisterAllocator

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_REGISTER_ALLOCATOR_H_


