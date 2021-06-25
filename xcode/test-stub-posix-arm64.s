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

co_dummy_entry1:
    ret

//----------------------------------------------------------------------------------------------------------------------
// yalx.lang.debugOutput(a: any)
//----------------------------------------------------------------------------------------------------------------------
.global _y2zlang_debugOutput,_dbg_output
_y2zlang_debugOutput:
    // TODO:
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
