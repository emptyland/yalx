.section __TEXT,__text,regular,pure_instructions
.build_version macos, 12, 0 sdk_version 12, 1
; libc symbols:
.global _memcpy,_memset
.file 1 "tests/40-code-gen-sanity/src/main" "main.yalx"
; External symbols:
.global _yalx_Zplang_Zolang_Zd_Z4init
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
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.global _main_Zomain_Zdissue1
_main_Zomain_Zdissue1:
Lblk2:
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
Lblk3:
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
    mov w3, w2
    add sp, sp, #16
    bl _main_Zomain_Zddoo
    sub sp, sp, #16
    ldur w19, [fp, #-48]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.global _main_Zomain_Zdissue3
_main_Zomain_Zdissue3:
Lblk4:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    add w2, w0, w1
    stur w0, [fp, #-36]
    stur w1, [fp, #-40]
    stur w2, [fp, #-44]
    mov w0, w1
    mov w1, w0
    mov w3, w2
    add sp, sp, #16
    bl _main_Zomain_Zddoo
    sub sp, sp, #16
    ldur w19, [fp, #-48]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.global _main_Zomain_Zdissue4
_main_Zomain_Zdissue4:
Lblk5:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    add w2, w0, w1
    stur w2, [fp, #-36]
    mov w3, w2
    add sp, sp, #0
    bl _main_Zomain_Zddoo
    sub sp, sp, #0
    ldur w19, [fp, #-40]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.global _main_Zomain_Zdissue5
_main_Zomain_Zdissue5:
Lblk6:
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
.global _main_Zomain_Zdfoo
_main_Zomain_Zdfoo:
Lblk7:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    adrp x19, Knnn.0@PAGE
    add x19, x19, Knnn.0@PAGEOFF
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
Lblk8:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    add w2, w0, w1
    add w0, w2, w3
    str w0, [fp, #28]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
_yalx_Zplang_Zolang_Zd_Z4init:
    ret
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "yalx/lang:lang"
; Number constants
.section __TEXT,__literal8,8byte_literals
.p2align 4
Knnn.0:
    .long 0x3f8ccccd    ; float.1.100000
.section __DATA,__data
.p2align 4
; Yalx-String constants
.global _main_Zomain_Lksz
_main_Zomain_Lksz:
    .long 1
    .quad Lkzs.0
.global _main_Zomain_Kstr
_main_Zomain_Kstr:
    .long 1
Kstr.0:
    .quad 0
