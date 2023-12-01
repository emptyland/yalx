.section __TEXT,__text,regular,pure_instructions
.p2align 2

.text

.global _handle_c_polling_page_entry,_current_mach,_handle_polling_page_exception

// ALIAS_REGISTER(Register, cp, x27);
// ALIAS_REGISTER(Register, fp, x29);
// ALIAS_REGISTER(Register, lr, x30);
// ALIAS_REGISTER(Register, xzr, x31);
// ALIAS_REGISTER(Register, wzr, w31);
_handle_c_polling_page_entry:
    sub sp, sp, 0x10 // keep returning address
    stp fp, xzr, [sp, 0]
    add fp, sp, 0

    sub sp, sp, 256
    stp x0, x1, [fp, -16]
    stp x2, x3, [fp, -32]
    stp x4, x5, [fp, -48]
    stp x6, x7, [fp, -64]
    stp x8, x9, [fp, -80]
    stp x10, x11, [fp, -96]
    stp x12, x13, [fp, -112]
    stp x14, x15, [fp, -128]
    stp x16, x17, [fp, -144]
    stp x18, x19, [fp, -160]
    stp x20, x21, [fp, -176]
    stp x22, x23, [fp, -192]
    stp x24, x25, [fp, -208]
    stp x26, x27, [fp, -224]
    stp x28, x31, [fp, -240]

    blr _current_mach // x0
    blr _handle_polling_page_exception
    add x0, x0, 4     // Fixed ARM64 instruction size=4
    str x0, [fp, 8]

    ldp x0, x1, [fp, -16]
    ldp x2, x3, [fp, -32]
    ldp x4, x5, [fp, -48]
    ldp x6, x7, [fp, -64]
    ldp x8, x9, [fp, -80]
    ldp x10, x11, [fp, -96]
    ldp x12, x13, [fp, -112]
    ldp x14, x15, [fp, -128]
    ldp x16, x17, [fp, -144]
    ldp x18, x19, [fp, -160]
    ldp x20, x21, [fp, -176]
    ldp x22, x23, [fp, -192]
    ldp x24, x25, [fp, -208]
    ldp x26, x27, [fp, -224]
    ldp x28, x31, [fp, -240]
    add sp, sp, 256

    ldp fp, lr, [sp, 0]
    add sp, sp, 0x10
    ret


.global _fast_poll_page,_mm_polling_page

_fast_poll_page:
    sub sp, sp, 0x10 // keep returning address
    stp fp, lr, [sp, 0]

    adrp x0, _mm_polling_page@PAGE
    add x0, x0, _mm_polling_page@PAGEOFF
    ldr xzr, [x0] // Polling

    ldp fp, lr, [sp, 0]
    add sp, sp, 0x10
    ret