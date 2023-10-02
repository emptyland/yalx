.p2align 2

.text

.global asm_stub1
asm_stub1:
    pushq %rbp
    movl %edi, %eax
    addl %esi, %eax
    popq %rbp
    ret

.global asm_stub2
asm_stub2:
    pushq %rbp
    movl 8(%rdi), %eax
    popq %rbp
    ret

.global asm_stub3,scheduler
asm_stub3:
    pushq %rbp
    leaq scheduler(%rip), %rdi
    movq (%rdi), %rax
    popq %rbp
    ret

.global asm_stub4,_puts
// test call external function
asm_stub4:
    pushq %rbp
    leaq msg(%rip), %rdi
    callq puts // call stdio.h puts
    popq %rbp
    ret

// test get thread-local variable
.global asm_stub5 //,_thread_local_mach
asm_stub5:
    // pushq %rbp
    // leaq _thread_local_mach(%rip), %rdi
    // callq *(%rdi) // call _thread_local_mach
    // movq (%rax), %rax
    // popq %rbp
    retq

.global asm_stub6
asm_stub6:
    pushq %rbp
    movq %rsp, %rbp

    movl $199, 28(%rbp)
    movl $911, 24(%rbp)
    movl $222, 20(%rbp)
    movl $220, 16(%rbp)

    popq %rbp
    retq

.global y2zmain_main,yield,yalx_new_string,heap,y2zlang_debugOutput
y2zmain_main:
    pushq %rbp
    movq %rsp, %rbp

    leaq msg(%rip), %rdi
    callq puts // call stdio.h puts

    //callq _yield

    leaq co_dummy_entry1(%rip), %rdi // _spawn_co
    xorq %rsi, %rsi //
    callq spawn_co

    movq heap(%rip), %rdi
    leaq msg(%rip), %rsi
    movq $13, %rdx
    callq yalx_new_string

    xorq %rdi, %rdi
    pushq %rdi
    pushq %rax
    callq y2zlang_debugOutput
    addq $16, %rsp

    popq %rbp
    ret


co_dummy_entry1:
    pushq %rbp
    movq %rsp, %rbp

    leaq dummy1(%rip), %rdi
    callq puts

    popq %rbp
    ret

// _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
// _yalx_Zplang_Zolang_ZdAny_Zdfinalize
// _yalx_Zplang_Zolang_ZdAny_ZdhashCode
// _yalx_Zplang_Zolang_ZdAny_Zdid
// _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
// _yalx_Zplang_Zolang_Zd_Z4init

.global yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
// .weak_definition _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor:
    retq

.global yalx_Zplang_Zolang_ZdAny_Zdfinalize
// .weak_definition _yalx_Zplang_Zolang_ZdAny_Zdfinalize
yalx_Zplang_Zolang_ZdAny_Zdfinalize:
    retq

.global yalx_Zplang_Zolang_ZdAny_ZdhashCode
// .weak_definition _yalx_Zplang_Zolang_ZdAny_ZdhashCode
yalx_Zplang_Zolang_ZdAny_ZdhashCode:
    pushq %rbp
    movq %rsp, %rbp
    movl $0, 28(%rbp)
    popq %rbp
    ret

.global yalx_Zplang_Zolang_ZdAny_Zdid
// .weak_definition _yalx_Zplang_Zolang_ZdAny_Zdid
yalx_Zplang_Zolang_ZdAny_Zdid:
    pushq %rbp
    movq %rsp, %rbp
    movl $0, 28(%rbp)
    popq %rbp
    ret

.global yalx_Zplang_Zolang_ZdAny_ZdisEmpty
// .weak_definition _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
yalx_Zplang_Zolang_ZdAny_ZdisEmpty:
    pushq %rbp
    movq %rsp, %rbp
    movl $0, 28(%rbp)
    popq %rbp
    ret

.global yalx_Zplang_Zolang_Zd_Z4init
// .weak_definition _yalx_Zplang_Zolang_Zd_Z4init
yalx_Zplang_Zolang_Zd_Z4init:
    retq

//----------------------------------------------------------------------------------------------------------------------
// yalx.lang.debugOutput(a: any)
//----------------------------------------------------------------------------------------------------------------------
.global y2zlang_debugOutput,dbg_output
y2zlang_debugOutput:
    pushq %rbp
    movq %rsp, %rbp

    movq 16(%rbp), %rdi
    callq dbg_output
    xorq %rax, %rax

    popq %rbp
    ret

.global yalx_magic_number1,yalx_magic_number2,yalx_magic_number3
.data
msg:
    .ascii "Hello, world!\0"
    // len = . - msg
dummy1:
    .ascii "Hello, dummy 1\0"
yalx_magic_number1:
    .ascii "FKJP"
yalx_magic_number2:
    .ascii "KLTW"
yalx_magic_number3:
    .ascii "HKDG"
