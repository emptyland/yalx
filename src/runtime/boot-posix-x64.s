.section __TEXT,__text,regular,pure_instructions
.p2align 2


//----------------------------------------------------------------------------------------------------------------------
// void trampoline()
//----------------------------------------------------------------------------------------------------------------------
.global _trampoline,_c0
_trampoline:
    pushq %rbp
    movq %rsp, %rbp;

    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %r15
    
    leaq _c0(%rip), %rdi
    movq %rsp, 48(%rdi) // RSP -> co->c_sp
    movq %rbp, 56(%rdi) // RBP -> co->c_fp

    movq 32(%rdi), %rsi // co->stack -> RSI
    movq 16(%rsi), %rsp // co->stack->top -> RSP
    movq 16(%rsi), %rbp // co->stack->top -> RBP

    movq %rdi, %r13 // R13 is current coroutine
    callq *40(%rdi) // call c0.entry

    leaq _c0(%rip), %rdi
    movq 48(%rdi), %rsp // restore RSP
    movq 56(%rdi), %rbp // restore RBP

    popq %r15
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    ret


//----------------------------------------------------------------------------------------------------------------------
// void call_returning_vals(void *returnning_vals, size_t size_in_bytes, void *yalx_fun)
//----------------------------------------------------------------------------------------------------------------------
// constexpr Register Argv_0 = rdi; // {kRDI},
// constexpr Register Argv_1 = rsi; // {kRSI},
// constexpr Register Argv_2 = rdx; // {kRDX},
// constexpr Register Argv_3 = rcx; // {kRCX},
// constexpr Register Argv_4 = r8;  // {kR8},
// constexpr Register Argv_5 = r9;  // {kR9},
// constexpr Register Argv_6 = r10; // {kR10},
// constexpr Register Argv_7 = r11; // {kR11},
// memcpy(dest, src, size)
.global _call_returning_vals, _memcpy
_call_returning_vals:
    pushq %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %r15

    movq %rsp, %rbp
    addq $32, %rsi
    subq %rsi, %rsp
    
    movq %rdi, -8(%rbp) // returnning_vals
    movq %rsi, -16(%rbp) // size_in_bytes
    movq %rdx, -24(%rbp) // yalx_fun
    
    callq *%rdx

    movq -8(%rbp), %rdi // dest
    movq %rsp, %rsi // src
    //addq $8, %rsi
    movq -16(%rbp), %rdx // size
    subq $32, %rdx
    callq _memcpy

    movq -16(%rbp), %rsi
    addq %rsi, %rsp

    popq %r15
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    retq
//----------------------------------------------------------------------------------------------------------------------
// void yield()
//----------------------------------------------------------------------------------------------------------------------
.global _yield,_yalx_schedule,_thread_local_mach
_yield:
    pushq %rbp
    movq %rsp, %rbp;

    callq _current_co // get current-coroutine(thread local)
    movq %rsp, 72(%rax) // RSP -> co->n_sp
    movq %rbp, 80(%rax) // RBP -> co->n_fp
    leaq (%rip), %rdi // RIP -> RDI
    addq $27, %rdi
    movq %rdi, 64(%rax) // RIP + 16 -> co->n_pc
    callq _yalx_schedule
    cmpl $0, %eax
    je yield_exit // yalx_schedule() == 0
    jmp yield_sched // yalx_schedule() != 0
    nop
    nop
    nop
    nop

yield_exit:
    popq %rbp
    ret
yield_sched:
    callq _current_co
    //int3
    movq 72(%rax), %rsp
    movq 80(%rax), %rbp
    movq 64(%rax), %rdi
    jmp *%rdi
    int3

//----------------------------------------------------------------------------------------------------------------------
// struct coroutine *current_co()
//----------------------------------------------------------------------------------------------------------------------
.global _current_co
_current_co:
    pushq %rbp
    movq %rsp, %rbp;

    leaq _thread_local_mach(%rip), %rdi
    callq *(%rdi) // get _thread_local_mach
    movq (%rax), %rax
    movq 40(%rax), %rax // _thread_local_mach->running

    popq %rbp
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
    pushq %rbp
    movq %rsp, %rbp;
    
    movq %rbp, %rdx
    addq $16, %rdx
    //addq %rsi, %rdx
    callq _yalx_install_coroutine
    // TODO: check and throw exception
    
    callq _yield // run new coroutine first

    popq %rbp
    ret


//----------------------------------------------------------------------------------------------------------------------
// void coroutine_finalize_stub()
//----------------------------------------------------------------------------------------------------------------------
.global _coroutine_finalize_stub
_coroutine_finalize_stub:
    callq _current_co
    movl $5, 24(%rax) // co->state = CO_DEAD
    callq _yalx_schedule
    callq _current_co
    //int3
    movq 72(%rax), %rsp
    movq 80(%rax), %rbp
    movq 64(%rax), %rdi
    jmp *%rdi
    int3

