.p2align 2

.text

.global handle_c_polling_page_entry,current_mach,handle_polling_page_exception

// rax, rdx, rsi, rdi, r8~11
// 408
handle_c_polling_page_entry:
    pushq %rbx
    movq %rsp, %rbp
    pushq %rbp

    pushq %rax
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11

    callq current_mach
    movq %rax, %rdi
    callq handle_polling_page_exception
    addq %rax, 2
    movq %rax, 8(%rbp)

    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rax

    popq %rbp
    retq


.global fast_poll_page,mm_polling_page

fast_poll_page:
    pushq %rbp
    movq mm_polling_page(%rip), %rax
    test %eax,(%rax) // 2 bytes
    // nop
    // nop // 2 bytes padding
    popq %rbp
    ret
