.section __TEXT,__text,regular,pure_instructions
.p2align 2

.global _entry
_entry:
    sub sp, sp, 0x10
    stp fp, lr, [sp, 0]
    mov x0, 0
    ldp fp, lr, [sp, 0]
    add sp, sp, 0x10
    ret

.global _asm_stub1
_asm_stub1:
    add w0, w0, w1
    ret

.global _asm_stub2
_asm_stub2:
    ldr w0, [x0, 8]
    ret

.global _yalx_rt0
_yalx_rt0:
    // TODO:
    ret
