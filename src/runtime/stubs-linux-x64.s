.p2align 2

.text

.global handle_c_polling_page_entry,current_mach,handle_polling_page_exception

// rax, rdx, rsi, rdi, r8~11
handle_c_polling_page_entry:
    pushq rax
    pushq rdx
    pushq rsi
    pushq rdi
    pushq r8
    pushq r9
    pushq r10
    pushq r11

    callq current_mach
    movq %rax, %rdi
    callq handle_polling_page_exception

    popq r11
    popq r10
    popq r9
    popq r8
    popq rdi
    popq rsi
    popq rdx
    popq rax
