.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "tests/48-code-gen-loops/src/issue09" "foo.yalx"
.p2align 2

; functions
.global _issue09_Zoissue09_Zd_Z4init
_issue09_Zoissue09_Zd_Z4init:
.cfi_startproc
Lblk0:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _yalx_Zplang_Zolang_Zd_Z4init@PAGE
    add x0, x19, _yalx_Zplang_Zolang_Zd_Z4init@PAGEOFF
    adrp x19, Lkzs.0@PAGE
    add x1, x19, Lkzs.0@PAGEOFF
    bl _pkg_init_once
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue09_Zoissue09_Zdissue1
_issue09_Zoissue09_Zdissue1:
.cfi_startproc
Lblk1:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w1, #1
    stur w1, [fp, #-4]
    mov w2, #0
    stur w2, [fp, #-8]
    b Lblk3
    nop
Lblk2:
    ldursw x3, [fp, #-4]
    add w4, w3, #3
    ldur w3, [fp, #-8]
    add w5, w3, #1
    stur w4, [fp, #-4]
    stur w5, [fp, #-8]
    b Lblk3
    nop
Lblk3:
    ldur w1, [fp, #-8]
    cmp w1, w0
    b.gt Lblk4
    b Lblk2
    nop
Lblk4:
    ldursw x19, [fp, #-4]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
.global _issue09_Zoissue09_Zdissue1_had
_issue09_Zoissue09_Zdissue1_had:
.cfi_startproc
Lblk5:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stp x19, x20, [sp, #80]
    stp x21, x22, [sp, #64]
    stp x23, x24, [sp, #48]
    stp x25, x26, [sp, #32]
    stp x27, x28, [sp, #16]
    add fp, sp, #16
    stur w0, [fp, #-4]
    bl _current_root
    mov x26, x0
    ldur w0, [fp, #-4]
    bl _issue09_Zoissue09_Zdissue1
    mov x0, #16
    bl _reserve_handle_returning_vals
    mov x1, sp
    mov x2, #16
    bl _memcpy
    ldp x19, x20, [sp, #80]
    ldp x21, x22, [sp, #64]
    ldp x23, x24, [sp, #48]
    ldp x25, x26, [sp, #32]
    ldp x27, x28, [sp, #16]
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _issue09_Zoissue09_Zdissue2
_issue09_Zoissue09_Zdissue2:
.cfi_startproc
Lblk6:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w1, #1
    stur w1, [fp, #-4]
    mov w2, #0
    stur w2, [fp, #-8]
    b Lblk8
    nop
Lblk7:
    ldursw x3, [fp, #-4]
    add w4, w3, #3
    ldur w3, [fp, #-8]
    add w5, w3, #1
    stur w4, [fp, #-4]
    stur w5, [fp, #-8]
    b Lblk8
    nop
Lblk8:
    ldur w1, [fp, #-8]
    cmp w1, w0
    b.ge Lblk9
    b Lblk7
    nop
Lblk9:
    ldursw x19, [fp, #-4]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
.global _issue09_Zoissue09_Zdissue2_had
_issue09_Zoissue09_Zdissue2_had:
.cfi_startproc
Lblk10:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stp x19, x20, [sp, #80]
    stp x21, x22, [sp, #64]
    stp x23, x24, [sp, #48]
    stp x25, x26, [sp, #32]
    stp x27, x28, [sp, #16]
    add fp, sp, #16
    stur w0, [fp, #-4]
    bl _current_root
    mov x26, x0
    ldur w0, [fp, #-4]
    bl _issue09_Zoissue09_Zdissue2
    mov x0, #16
    bl _reserve_handle_returning_vals
    mov x1, sp
    mov x2, #16
    bl _memcpy
    ldp x19, x20, [sp, #80]
    ldp x21, x22, [sp, #64]
    ldp x23, x24, [sp, #48]
    ldp x25, x26, [sp, #32]
    ldp x27, x28, [sp, #16]
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "yalx/lang:lang"
.section __DATA,__data
.p2align 4
.global _issue09_Zoissue09$global_slots
_issue09_Zoissue09$global_slots:
    .quad 0 ; size_in_bytes
    .quad 0 ; slots
    .quad 0 ; mark_size
; string constants:
; Yalx-String constants
.global _issue09_Zoissue09_Lksz
_issue09_Zoissue09_Lksz:
    .long 1
    .long 0 ; padding for struct lksz_header
    .quad Lkzs.0
.global _issue09_Zoissue09_Kstr
_issue09_Zoissue09_Kstr:
    .long 1
    .long 0 ; padding for struct kstr_header
Kstr.0:
    .quad 0
