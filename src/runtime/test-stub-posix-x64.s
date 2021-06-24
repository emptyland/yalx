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
    //movq %rsp, %rbp
    pushq %rbp
    leaq _thread_local_mach(%rip), %rdi
    callq *(%rdi) // call _thread_local_mach
    movq (%rax), %rax
    popq %rbp
    //movq %rbp, %rsp
    ret

.global _y2zmain_main,_yield,_yalx_new_string,_heap,_y2zlang_debugOutput
_y2zmain_main:
    pushq %rbp
    movq %rsp, %rbp

    leaq msg(%rip), %rdi
    callq _puts // call stdio.h puts

    //callq _yield

    leaq co_dummy_entry1(%rip), %rdi // _spawn_co
    xorq %rsi, %rsi //
    callq _spawn_co

    leaq _heap(%rip), %rdi
    leaq msg(%rip), %rsi
    movq $13, %rdx
    callq _yalx_new_string

    xorq %rdi, %rdi
    pushq %rdi
    pushq %rax
    callq _y2zlang_debugOutput
    addq $16, %rsp

    popq %rbp
    ret


co_dummy_entry1:
    pushq %rbp
    movq %rsp, %rbp

    leaq dummy1(%rip), %rdi
    callq _puts

    popq %rbp
    ret

//----------------------------------------------------------------------------------------------------------------------
// yalx.lang.debugOutput(a: any)
//----------------------------------------------------------------------------------------------------------------------
.global _y2zlang_debugOutput,_dbg_output
_y2zlang_debugOutput:
    pushq %rbp
    movq %rsp, %rbp

    movq 16(%rbp), %rdi
    callq _dbg_output
    xorq %rax, %rax

    popq %rbp
    ret

.global _yalx_magic_number1,_yalx_magic_number2
.data
msg:
    .ascii "Hello, world!\0"
    // len = . - msg
dummy1:
    .ascii "Hello, dummy 1\0"
_yalx_magic_number1:
    .ascii "FKJP"
_yalx_magic_number2:
    .ascii "KLTW"
