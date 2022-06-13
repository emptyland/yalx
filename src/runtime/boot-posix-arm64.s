.section __TEXT,__text,regular,pure_instructions
.p2align 2


//----------------------------------------------------------------------------------------------------------------------
// void trampoline()
//----------------------------------------------------------------------------------------------------------------------
.global _trampoline,_c0
_trampoline:
    ret


//----------------------------------------------------------------------------------------------------------------------
// void yield()
//----------------------------------------------------------------------------------------------------------------------
.global _yield,_yalx_schedule,_thread_local_mach
_yield:
    ret


//----------------------------------------------------------------------------------------------------------------------
// struct coroutine *current_co()
//----------------------------------------------------------------------------------------------------------------------
.global _current_co
_current_co:
    ret


//----------------------------------------------------------------------------------------------------------------------
// void spawn_co(address_t entry, u32_t params_bytes)
// rdi = entry
// rsi = params_bytes
//
// | params begin |
// |     ....     |
// | params end   |
// +--------------+
// | return addr  |
// +--------------+
// | saved RBP    |
//----------------------------------------------------------------------------------------------------------------------
.global _spawn_co,_yalx_install_coroutine
_spawn_co:
    ret


//----------------------------------------------------------------------------------------------------------------------
// void coroutine_finalize_stub()
//----------------------------------------------------------------------------------------------------------------------
.global _coroutine_finalize_stub
_coroutine_finalize_stub:
    ret



//----------------------------------------------------------------------------------------------------------------------
// void call0_returning_vals(void *returnning_vals, size_t size_in_bytes, void *yalx_fun)
//----------------------------------------------------------------------------------------------------------------------
// x0 = returnning_vals
// x1 = size_in_bytes
// x2 = yalx_fun
// r19…r28 Callee-saved registers
.global _call0_returning_vals, _memcpy
_call0_returning_vals:
    sub sp, sp, 96
    stp fp, lr, [sp, 80]
    stp x19, x20, [sp, 64]
    stp x21, x22, [sp, 48]
    stp x23, x24, [sp, 32]
    stp x25, x26, [sp, 16]
    stp x27, x28, [sp, 0]
    
    mov x19, x1
    add x19, x19, 32
    mov fp, sp
    sub sp, sp, x19

    str x0, [fp, -8]  // store returnning_vals
    str x1, [fp, -16] // store size_in_bytes
    str x2, [fp, -24] // stroe yalx_fun

    blr x2

    ldr x0, [fp, -8]
    mov x1, sp
    ldr x2, [fp, -16]
    bl _memcpy

    ldr x19, [fp, -16]
    add x19, x19, 32
    add sp, sp, x19

    ldp x27, x28, [sp, 0]
    ldp x25, x26, [sp, 16]
    ldp x23, x24, [sp, 32]
    ldp x21, x22, [sp, 48]
    ldp x19, x20, [sp, 64]
    ldp fp, lr, [sp, 80]
    add sp, sp, 96
    mov x0, 0
    ret


//----------------------------------------------------------------------------------------------------------------------
// void throw_to(struct coroutine *root, address_t pc, address_t fp, address_t sp)
// x0 = root
// x1 = pc
// x2 = fp
// x3 = sp
//----------------------------------------------------------------------------------------------------------------------
.global _throw_to
_throw_to:
    mov x26, x0 // arg0 -> root
    mov fp, x2
    mov sp, x3
    blr x1
    brk #0x3c
