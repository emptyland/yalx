.p2align 2

.text

.global handle_c_polling_page_entry,current_mach,handle_polling_page_exception

// rax, rdx, rsi, rdi, r8~11
// [return addr]  8(%rbp)
// [prev rbp   ]  0(%rbp)
// [ saved rax ] -8(%rbp)
handle_c_polling_page_entry:
    subq $8, %rsp
    pushq %rbp
    movq %rsp, %rbp

    subq $128, %rsp

    movq %rax, -8(%rbp)
    movq %rcx, -16(%rbp)
    movq %rdx, -24(%rbp)
    movq %rbx, -32(%rbp)
    movq %rsi, -40(%rbp)
    movq %rdi, -48(%rbp)
    movq %r8,  -56(%rbp)
    movq %r9,  -64(%rbp)
    movq %r10, -72(%rbp)
    movq %r11, -80(%rbp)
    movq %r12, -88(%rbp)
    movq %r13, -96(%rbp)
    movq %r14, -104(%rbp)
    movq %r15, -112(%rbp)

    callq current_mach
    movq %rax, %rdi
    callq handle_polling_page_exception
    addq $4, %rax
    movq %rax, 8(%rbp)

    movq -8(%rbp), %rax
    movq -16(%rbp), %rcx
    movq -24(%rbp), %rdx
    movq -32(%rbp), %rbx
    movq -40(%rbp), %rsi
    movq -48(%rbp), %rdi
    movq -56(%rbp), %r8
    movq -64(%rbp), %r9
    movq -72(%rbp), %r10
    movq -80(%rbp), %r11
    movq -88(%rbp), %r12
    movq -96(%rbp), %r13
    movq -104(%rbp), %r14
    movq -112(%rbp), %r15

    addq $128, %rsp

    popq %rbp
    retq


.global fast_poll_page,mm_polling_page

fast_poll_page:
    pushq %rbp
    movq mm_polling_page(%rip), %rax
    test %eax,(%rax) // 2 bytes
    nop
    nop // 2 bytes padding
    popq %rbp
    ret

