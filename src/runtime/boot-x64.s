.section __TEXT,__text,regular,pure_instructions
.p2align 2

.global _trampoline,_c0
// rdi = coroutine
_trampoline:
    movq %rsp, %rbp;
    // Save fucking callee shit registers
    pushq %rbp
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
    movq %rbp, %rsp;
    ret


.global _schedule,yalx_schedule,_thread_local_mach
_schedule:
    movq %rsp, %rbp;
    pushq %rbp

    callq _current_co // get current-coroutine(thread local)
    movq %rsp, 72(%rax) // RSP -> co->n_sp
    movq %rbp, 80(%rax) // RBP -> co->n_fp
    leaq (%rip), %rdi // RIP -> RDI
    addq 16, %rdi
    movq %rdi, 64(%rax) // RIP + 16 -> co->n_pc

    nop
    nop
    nop
    nop

    nop
    nop
    nop
    nop

    nop
    nop
    nop
    nop

    nop
    nop
    nop
    nop

    nop
    nop
    nop
    nop

    movq %rbp, %rsp;
    ret

//------------------------------------------------------------------------------------
// struct coroutine *current_co()
//------------------------------------------------------------------------------------
.global _current_co
_current_co:
    movq %rsp, %rbp;
    pushq %rbp
    
    movq _thread_local_mach(%rip), %rdi
    callq *(%rdi) // get _thread_local_mach
    movq 272(%rax), %rax // _thread_local_mach->running

    movq %rbp, %rsp;
    ret
