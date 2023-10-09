.section __TEXT,__text,regular,pure_instructions
.p2align 2

.text

.global _asm_stub1
_asm_stub1:
    add w0, w0, w1
    ret


.global _asm_stub2
_asm_stub2:
    sub sp, sp, 0x10
    stp fp, lr, [sp, 0]

    ldr w0, [x0, 8]

    ldp fp, lr, [sp, 0]
    add sp, sp, 0x10
    ret


.global _asm_stub3,_scheduler
_asm_stub3:
    sub sp, sp, 0x10
    stp fp, lr, [sp, 0]

    adrp x0, _scheduler@PAGE
    add x0, x0, _scheduler@PAGEOFF
    ldr x0, [x0, 0]
    
    ldp fp, lr, [sp, 0]
    add sp, sp, 0x10
    ret

.global _asm_stub4,_puts
// test call external function
_asm_stub4:
    sub sp, sp, 0x10
    stp fp, lr, [sp, 0]
    
    adrp x0, msg@PAGE
    add x0, x0, msg@PAGEOFF
    bl _puts

    ldp fp, lr, [sp, 0]
    add sp, sp, 0x10
    ret


// test get thread-local variable
.global _asm_stub5,_thread_local_mach
_asm_stub5:
    sub sp, sp, 0x10
    stp fp, lr, [sp, 0]

    // Load the thread-local variable
    adrp x8, _thread_local_mach@PAGE
    add x8, x8, _thread_local_mach@PAGEOFF
    ldr x9, [x8]
    mov x0, x8
    blr x9
    ldr x0, [x0, 0]
    
    ldp fp, lr, [sp, 0]
    add sp, sp, 0x10
    ret

.global _y2zmain_main,_yield,_yalx_new_string,_heap,_y2zlang_debugOutput
_y2zmain_main:
    // TODO:
    ret

.global _asm_stub6
_asm_stub6:
    sub sp, sp, 0x10
    stp fp, lr, [sp, 0]
    add fp, sp, 0
    
    mov w0, 199
    str w0, [fp, 28]
    mov w0, 911
    str w0, [fp, 24]
    mov w0, 222
    str w0, [fp, 20]
    mov w0, 220
    str w0, [fp, 16]

    ldp fp, lr, [sp, 0]
    add sp, sp, 0x10
    ret

co_dummy_entry1:
    ret

// _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
// _yalx_Zplang_Zolang_ZdAny_Zdfinalize
// _yalx_Zplang_Zolang_ZdAny_ZdhashCode
// _yalx_Zplang_Zolang_ZdAny_Zdid
// _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
// _yalx_Zplang_Zolang_Zd_Z4init

.global _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
.weak_definition _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
_yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor:
    ret

.global _yalx_Zplang_Zolang_ZdAny_Zdfinalize
.weak_definition _yalx_Zplang_Zolang_ZdAny_Zdfinalize
_yalx_Zplang_Zolang_ZdAny_Zdfinalize:
    ret

.global _yalx_Zplang_Zolang_ZdAny_ZdhashCode
.weak_definition _yalx_Zplang_Zolang_ZdAny_ZdhashCode
_yalx_Zplang_Zolang_ZdAny_ZdhashCode:
    ret

.global _yalx_Zplang_Zolang_ZdAny_Zdid
.weak_definition _yalx_Zplang_Zolang_ZdAny_Zdid
_yalx_Zplang_Zolang_ZdAny_Zdid:
    ret

.global _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
.weak_definition _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
_yalx_Zplang_Zolang_ZdAny_ZdisEmpty:
    ret

.global _yalx_Zplang_Zolang_Zd_Z4init
.weak_definition _yalx_Zplang_Zolang_Zd_Z4init
_yalx_Zplang_Zolang_Zd_Z4init:
    ret

//----------------------------------------------------------------------------------------------------------------------
// yalx.lang.debugOutput(a: any)
//----------------------------------------------------------------------------------------------------------------------
.global _y2zlang_debugOutput,_dbg_output
_y2zlang_debugOutput:
    // TODO:
    ret

.global _yalx_magic_number1,_yalx_magic_number2,_yalx_magic_number3
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
_yalx_magic_number3:
    .ascii "HKDG"
.global _yalx_Zplang_Zolang_ZdAny$class
.weak_definition _yalx_Zplang_Zolang_ZdAny$class
_yalx_Zplang_Zolang_ZdAny$class:
    .quad 0
.global _yalx_Zplang_Zolang_ZdException$class
.weak_definition _yalx_Zplang_Zolang_ZdException$class
_yalx_Zplang_Zolang_ZdException$class:
    .quad 0
.global _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
.weak_definition _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
_yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor:
    .quad 0
