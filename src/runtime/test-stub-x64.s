.section __TEXT,__text,regular,pure_instructions
.p2align 2

.text

.global _asm_stub1
_asm_stub1:
    pushq %rbp
    movl %edi, %eax
    addl %esi, %eax
    popq %rbp
    ret

.global _asm_stub2
_asm_stub2:
    pushq %rbp
    movl 8(%rdi), %eax
    popq %rbp
    ret

.global _asm_stub3,_scheduler
_asm_stub3:
    pushq %rbp
    leaq _scheduler(%rip), %rdi
    movq (%rdi), %rax
    popq %rbp
    ret

.global _asm_stub4,_puts
// test call external function
_asm_stub4:
    pushq %rbp
    leaq msg(%rip), %rdi
    callq _puts // call stdio.h puts
    popq %rbp
    ret

// test get thread-local variable
.global _asm_stub5,_thread_local_mach
_asm_stub5:
    pushq %rbp
    leaq _thread_local_mach(%rip), %rdi
    callq *(%rdi) // call _thread_local_mach
    movq (%rax), %rax
    popq %rbp
    ret

.data
msg:
    .ascii "Hello, world!"
    len = . - msg

