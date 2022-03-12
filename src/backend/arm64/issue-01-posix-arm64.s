.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 3 sdk_version 12, 1
.file 1 "tests/40-code-gen-sanity/src/main" "main.yalx"
.p2align 2

; functions
.global _main_Zomain_Zd_Z4init
_main_Zomain_Zd_Z4init:
Lblk0:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    adrp x19, _yalx_Zplang_Zolang_Zd_Z4init@PAGE
    add x0, x19, _yalx_Zplang_Zolang_Zd_Z4init@PAGEOFF
    adrp x19, Lkzs.0@PAGE
    add x1, x19, Lkzs.0@PAGEOFF
    bl _pkg_init_once
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.global _main_Zomain_Zdmain
_main_Zomain_Zdmain:
Lblk1:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    adrp x19, Kstr.1@PAGE
    add x19, x19, Kstr.1@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-40]
    mov x0, x0
    add sp, sp, #0
    bl _main_Zomain_Zddisplay
    sub sp, sp, #0
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.global _main_Zomain_Zdmain_had
_main_Zomain_Zdmain_had:
Lblk2:
    sub sp, sp, #96
    stp fp, lr, [sp, #80]
    stp x19, x20, [sp, #64]
    stp x21, x22, [sp, #48]
    stp x23, x24, [sp, #32]
    stp x25, x26, [sp, #16]
    stp x27, x28, [sp, #0]
    add fp, sp, #0
    bl _current_root
    mov x26, x0
    bl _main_Zomain_Zdmain
    ldp x19, x20, [sp, #64]
    ldp x21, x22, [sp, #48]
    ldp x23, x24, [sp, #32]
    ldp x25, x26, [sp, #16]
    ldp x27, x28, [sp, #0]
    ldp fp, lr, [sp, #80]
    add sp, sp, #96
    ret
.global _main_Zomain_Zdissue1
_main_Zomain_Zdissue1:
Lblk3:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    add sp, sp, #0
    bl _main_Zomain_Zdfoo
    sub sp, sp, #0
    ldur w0, [fp, #-36]
    ldur w1, [fp, #-40]
    add w2, w0, w1
    str w2, [fp, #28]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.global _main_Zomain_Zdissue2
_main_Zomain_Zdissue2:
Lblk4:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    mov w0, #1
    mov w1, #2
    mov w2, #3
    stur w0, [fp, #-36]
    stur w1, [fp, #-40]
    stur w2, [fp, #-44]
    mov w0, w0
    mov w1, w1
    mov w2, w2
    add sp, sp, #16
    bl _main_Zomain_Zddoo
    sub sp, sp, #16
    ldur w19, [fp, #-52]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.global _main_Zomain_Zdissue3
_main_Zomain_Zdissue3:
Lblk5:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    add w2, w0, w1
    stur w0, [fp, #-36]
    stur w1, [fp, #-40]
    mov w0, w1
    mov w1, w0
    add sp, sp, #16
    bl _main_Zomain_Zddoo
    sub sp, sp, #16
    ldur w19, [fp, #-52]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.global _main_Zomain_Zdissue4
_main_Zomain_Zdissue4:
Lblk6:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    add w2, w0, w1
    add sp, sp, #0
    bl _main_Zomain_Zddoo
    sub sp, sp, #0
    ldur w19, [fp, #-36]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.global _main_Zomain_Zdissue5
_main_Zomain_Zdissue5:
Lblk7:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    mov w0, #4
    str w0, [fp, #16]
    mov w1, #3
    str w1, [fp, #20]
    mov w2, #2
    str w2, [fp, #24]
    mov w3, #1
    str w3, [fp, #28]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.global _main_Zomain_Zdissue6
_main_Zomain_Zdissue6:
Lblk8:
    sub sp, sp, #96
    stp fp, lr, [sp, #80]
    add fp, sp, #80
    cmp w0, w1
    b.le Lblk10
Lblk9:
    stur w0, [fp, #-36]
    stur w1, [fp, #-40]
    stur w2, [fp, #-44]
    add sp, sp, #16
    bl _main_Zomain_Zdissue5
    sub sp, sp, #16
    ldur w0, [fp, #-36]
    ldur w1, [fp, #-40]
    ldur w2, [fp, #-44]
    ldur w3, [fp, #-52]
    add w4, w3, w0
    add w0, w4, w1
    mov w2, w0
    b Lblk11
    nop
Lblk10:
    adrp x19, Knnn.0@PAGE
    add x19, x19, Knnn.0@PAGEOFF
    ldr w1, [x19, #0]
    mov w2, w1
    b Lblk11
    nop
Lblk11:
    str w2, [fp, #28]
    ldp fp, lr, [sp, #80]
    add sp, sp, #96
    ret
.global _main_Zomain_Zdissue6_had
_main_Zomain_Zdissue6_had:
Lblk12:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    stp x19, x20, [sp, #80]
    stp x21, x22, [sp, #64]
    stp x23, x24, [sp, #48]
    stp x25, x26, [sp, #32]
    stp x27, x28, [sp, #16]
    add fp, sp, #16
    stur w0, [fp, #-36]
    stur w1, [fp, #-40]
    bl _current_root
    mov x26, x0
    ldur w0, [fp, #-36]
    ldur w1, [fp, #-40]
    bl _main_Zomain_Zdissue6
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
.global _main_Zomain_Zdissue7
_main_Zomain_Zdissue7:
Lblk13:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    fcmp s0, s1
    b.pl Lblk15
Lblk14:
    adrp x19, Knnn.0@PAGE
    add x19, x19, Knnn.0@PAGEOFF
    ldr w1, [x19, #0]
    mov w0, w1
    b Lblk19
    nop
Lblk15:
    fcmp s0, s1
    b.le Lblk17
Lblk16:
    mov w3, #1
    mov w2, w3
    b Lblk18
    nop
Lblk17:
    mov w4, #0
    mov w2, w4
    b Lblk18
    nop
Lblk18:
    mov w0, w2
    b Lblk19
    nop
Lblk19:
    str w0, [fp, #28]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.global _main_Zomain_Zdissue8
_main_Zomain_Zdissue8:
Lblk20:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    cmp w0, w1
    mov w19, #1
    csel w2, w19, wzr, LT
    strb w2, [fp, #28]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.global _main_Zomain_Zdissue9
_main_Zomain_Zdissue9:
Lblk21:
    sub sp, sp, #144
    stp fp, lr, [sp, #128]
    add fp, sp, #128
    stur w0, [fp, #-84]
    stur x1, [fp, #-92]
    sub x0, fp, #80
    add x1, fp, #16
    mov x2, #16
    adr x3, _main_Zomain_Zdissue9
    bl _associate_stub_returning_vals
    ldur w0, [fp, #-84]
    sub x1, fp, #92
    bl _issue9_stub
    sub x0, fp, #80
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #128]
    add sp, sp, #144
    ret
.global _main_Zomain_Zdfoo
_main_Zomain_Zdfoo:
Lblk22:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    adrp x19, Knnn.1@PAGE
    add x19, x19, Knnn.1@PAGEOFF
    ldr s0, [x19, #0]
    str s0, [fp, #20]
    mov w0, #2
    str w0, [fp, #24]
    mov w1, #1
    str w1, [fp, #28]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.global _main_Zomain_Zddoo
_main_Zomain_Zddoo:
Lblk23:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    add w3, w0, w1
    add w0, w3, w2
    str w0, [fp, #28]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.global _main_Zomain_Zddisplay
_main_Zomain_Zddisplay:
Lblk24:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    stur x0, [fp, #-40]
    sub x0, fp, #40
    bl _yalx_Zplang_Zolang_Zdprintln_stub
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
_yalx_Zplang_Zolang_Zd_Z4init:
    ret
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "yalx/lang:lang"
Lkzs.1:
    .asciz "Hello, World"
; Number constants
.section __TEXT,__const
.p2align 2
Knnn.1:
    .long 0x3f8ccccd    ; float.1.100000
Knnn.0:
    .long -1
.section __DATA,__data
.p2align 4
; Yalx-String constants
.global _main_Zomain_Lksz
_main_Zomain_Lksz:
    .long 2
    .long 0 ; padding for struct lksz_header
    .quad Lkzs.0
    .quad Lkzs.1
.global _main_Zomain_Kstr
_main_Zomain_Kstr:
    .long 2
    .long 0 ; padding for struct kstr_header
Kstr.0:
    .quad 0
Kstr.1:
    .quad 0
