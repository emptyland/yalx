#pragma once
#ifndef YALX_BACKEND_REGISTER_CONFIGURATION_H_
#define YALX_BACKEND_REGISTER_CONFIGURATION_H_

#include "base/checking.h"
#include "base/base.h"

namespace yalx {

namespace backend {

class RegistersConfiguration final {
public:
    RegistersConfiguration(int number_of_argument_gp_registers,
                           const int *argument_gp_registers,
                           int number_of_argument_fp_registers,
                           const int *argument_fp_registers,
                           int number_of_allocatable_gp_registers,
                           const int *allocatable_gp_registers,
                           int number_of_allocatable_fp_registers,
                           const int *allocatable_fp_registers,
                           int number_of_callee_save_gp_registers,
                           const int *callee_save_gp_registers,
                           int number_of_callee_save_fp_registers,
                           const int *callee_save_fp_registers,
                           int number_of_gp_registers,
                           const int *gp_registers,
                           int number_of_fp_registers,
                           const int *fp_registers,
                           int scratch0,
                           int scratch1,
                           int returning0_register,
                           int fp,
                           int sp,
                           int root);
    
#define DEFINE_REGISTERS_SET(name) \
    int number_of_##name##s() const { return number_of_##name##s_; } \
    int name(int i) const { \
        DCHECK(i >= 0); \
        DCHECK(i < number_of_##name##s()); \
        return name##s_[i]; \
    }
    
    DEFINE_REGISTERS_SET(argument_gp_register);
    DEFINE_REGISTERS_SET(argument_fp_register);
    DEFINE_REGISTERS_SET(allocatable_gp_register);
    DEFINE_REGISTERS_SET(allocatable_fp_register);
    DEFINE_REGISTERS_SET(callee_save_gp_register);
    DEFINE_REGISTERS_SET(callee_save_fp_register);
    DEFINE_REGISTERS_SET(gp_register);
    DEFINE_REGISTERS_SET(fp_register);
    
#undef DEFINE_REGISTERS_SET
    
    DEF_VAL_GETTER(int, scratch0);
    DEF_VAL_GETTER(int, scratch1);
    DEF_VAL_GETTER(int, returning0_register);
    DEF_VAL_GETTER(int, fp);
    DEF_VAL_GETTER(int, sp);
    DEF_VAL_GETTER(int, root);
    
    static const RegistersConfiguration *of_arm64();
    static const RegistersConfiguration *of_x64();
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(RegistersConfiguration);
private:
    const int        number_of_argument_gp_registers_;
    const int *const argument_gp_registers_;
    
    const int        number_of_argument_fp_registers_;
    const int *const argument_fp_registers_;
    
    const int        number_of_allocatable_gp_registers_;
    const int *const allocatable_gp_registers_;
    
    const int        number_of_allocatable_fp_registers_;
    const int *const allocatable_fp_registers_;
    
    const int        number_of_callee_save_gp_registers_;
    const int *const callee_save_gp_registers_;
    
    const int        number_of_callee_save_fp_registers_;
    const int *const callee_save_fp_registers_;
    
    const int        number_of_gp_registers_;
    const int *const gp_registers_;
    
    const int        number_of_fp_registers_;
    const int *const fp_registers_;
    
    const int scratch0_;
    const int scratch1_;
    
    const int returning0_register_;
    const int fp_;
    const int sp_;
    const int root_;
}; // class RegistersConfiguration

} // namespace backend

} // namespace yalx

#endif // YALX_BACKEND_REGISTER_CONFIGURATION_H_
