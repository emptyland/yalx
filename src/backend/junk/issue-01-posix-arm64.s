.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "tests/40-code-gen-sanity/src/main" "main.yalx"
.p2align 2

; functions
.global _main_Zomain_Zd_Z4init
_main_Zomain_Zd_Z4init:
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
.global _main_Zomain_Zdmain
_main_Zomain_Zdmain:
.cfi_startproc
Lblk1:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, Kstr.1@PAGE
    add x19, x19, Kstr.1@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-8]
    bl _main_Zomain_Zddisplay
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
.global _main_Zomain_Zdmain_had
_main_Zomain_Zdmain_had:
.cfi_startproc
Lblk2:
    sub sp, sp, #96
    stp fp, lr, [sp, #80]
    add fp, sp, #80
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
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
.cfi_endproc
.global _main_Zomain_Zdsanity
_main_Zomain_Zdsanity:
.cfi_startproc
Lblk3:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #0
    stur w0, [fp, #-8]
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
.global _main_Zomain_Zdissue1
_main_Zomain_Zdissue1:
.cfi_startproc
Lblk4:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    add sp, sp, #0
    bl _main_Zomain_Zdfoo
    sub sp, sp, #0
    ldur w0, [fp, #-4]
    ldur w1, [fp, #-8]
    add w2, w0, w1
    str w2, [fp, #28]
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
.global _main_Zomain_Zdissue2
_main_Zomain_Zdissue2:
.cfi_startproc
Lblk5:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #1
    mov w1, #2
    mov w2, #3
    stur w0, [fp, #-4]
    stur w1, [fp, #-8]
    stur w2, [fp, #-12]
    add sp, sp, #0
    bl _main_Zomain_Zddoo
    sub sp, sp, #0
    ldur w19, [fp, #-20]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _main_Zomain_Zdissue3
_main_Zomain_Zdissue3:
.cfi_startproc
Lblk6:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    add w2, w0, w1
    stur w0, [fp, #-4]
    stur w1, [fp, #-8]
    ldur w0, [fp, #-8]
    ldur w1, [fp, #-4]
    add sp, sp, #0
    bl _main_Zomain_Zddoo
    sub sp, sp, #0
    ldur w19, [fp, #-20]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _main_Zomain_Zdissue4
_main_Zomain_Zdissue4:
.cfi_startproc
Lblk7:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    add w2, w0, w1
    add sp, sp, #0
    bl _main_Zomain_Zddoo
    sub sp, sp, #0
    ldur w19, [fp, #-4]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
.global _main_Zomain_Zdissue5
_main_Zomain_Zdissue5:
.cfi_startproc
Lblk8:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #4
    str w0, [fp, #16]
    mov w1, #3
    str w1, [fp, #20]
    mov w2, #2
    str w2, [fp, #24]
    mov w3, #1
    str w3, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _main_Zomain_Zdissue6
_main_Zomain_Zdissue6:
.cfi_startproc
Lblk9:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    cmp w0, w1
    b.le Lblk11
Lblk10:
    stur w0, [fp, #-4]
    stur w1, [fp, #-8]
    stur w2, [fp, #-12]
    add sp, sp, #16
    bl _main_Zomain_Zdissue5
    sub sp, sp, #16
    ldur w0, [fp, #-4]
    ldur w1, [fp, #-8]
    ldur w2, [fp, #-12]
    ldur w3, [fp, #-20]
    add w4, w3, w0
    add w0, w4, w1
    mov w2, w0
    b Lblk12
    nop
Lblk11:
    adrp x19, Knnn.0@PAGE
    add x19, x19, Knnn.0@PAGEOFF
    ldr w1, [x19, #0]
    mov w2, w1
    b Lblk12
    nop
Lblk12:
    str w2, [fp, #28]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _main_Zomain_Zdissue6_had
_main_Zomain_Zdissue6_had:
.cfi_startproc
Lblk13:
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
    stur w1, [fp, #-8]
    bl _current_root
    mov x26, x0
    ldur w0, [fp, #-4]
    ldur w1, [fp, #-8]
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
.cfi_endproc
.global _main_Zomain_Zdissue7
_main_Zomain_Zdissue7:
.cfi_startproc
Lblk14:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    fcmp s0, s1
    b.pl Lblk16
Lblk15:
    adrp x19, Knnn.0@PAGE
    add x19, x19, Knnn.0@PAGEOFF
    ldr w1, [x19, #0]
    mov w0, w1
    b Lblk20
    nop
Lblk16:
    fcmp s0, s1
    b.le Lblk18
Lblk17:
    mov w3, #1
    mov w2, w3
    b Lblk19
    nop
Lblk18:
    mov w4, #0
    mov w2, w4
    b Lblk19
    nop
Lblk19:
    mov w0, w2
    b Lblk20
    nop
Lblk20:
    str w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _main_Zomain_Zdissue8
_main_Zomain_Zdissue8:
.cfi_startproc
Lblk21:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    cmp w0, w1
    mov w19, #1
    csel w2, w19, wzr, LT
    strb w2, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _main_Zomain_Zdissue9
_main_Zomain_Zdissue9:
.cfi_startproc
Lblk22:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur w0, [fp, #-52]
    stur x1, [fp, #-60]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _main_Zomain_Zdissue9
    bl _associate_stub_returning_vals
    ldur w0, [fp, #-52]
    sub x1, fp, #60
    bl _issue9_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _main_Zomain_Zdissue10
_main_Zomain_Zdissue10:
.cfi_startproc
Lblk23:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #32
    adr x3, _main_Zomain_Zdissue10
    bl _associate_stub_returning_vals
    bl _issue10_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _main_Zomain_Zdissue11
_main_Zomain_Zdissue11:
.cfi_startproc
Lblk24:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    add sp, sp, #0
    bl _main_Zomain_Zdissue10
    sub sp, sp, #0
    mov w0, #1
    stur w0, [fp, #-24]
    ldur w1, [fp, #-4]
    bl _main_Zomain_ZdassertInt
    mov w0, #2
    stur w0, [fp, #-4]
    ldur w1, [fp, #-8]
    bl _main_Zomain_ZdassertInt
    mov w0, #3
    stur w0, [fp, #-4]
    ldur w1, [fp, #-12]
    bl _main_Zomain_ZdassertInt
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-8]
    ldur x1, [fp, #-20]
    bl _main_Zomain_ZdassertString
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _main_Zomain_Zdissue11_had
_main_Zomain_Zdissue11_had:
.cfi_startproc
Lblk25:
    sub sp, sp, #96
    stp fp, lr, [sp, #80]
    add fp, sp, #80
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stp x19, x20, [sp, #64]
    stp x21, x22, [sp, #48]
    stp x23, x24, [sp, #32]
    stp x25, x26, [sp, #16]
    stp x27, x28, [sp, #0]
    add fp, sp, #0
    bl _current_root
    mov x26, x0
    bl _main_Zomain_Zdissue11
    ldp x19, x20, [sp, #64]
    ldp x21, x22, [sp, #48]
    ldp x23, x24, [sp, #32]
    ldp x25, x26, [sp, #16]
    ldp x27, x28, [sp, #0]
    ldp fp, lr, [sp, #80]
    add sp, sp, #96
    ret
.cfi_endproc
.global _main_Zomain_Zdfoo
_main_Zomain_Zdfoo:
.cfi_startproc
Lblk26:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, Knnn.1@PAGE
    add x19, x19, Knnn.1@PAGEOFF
    ldr s0, [x19, #0]
    str s0, [fp, #20]
    mov w0, #2
    str w0, [fp, #24]
    mov w1, #1
    str w1, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _main_Zomain_Zddoo
_main_Zomain_Zddoo:
.cfi_startproc
Lblk27:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    add w3, w0, w1
    add w0, w3, w2
    str w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _main_Zomain_Zddisplay
_main_Zomain_Zddisplay:
.cfi_startproc
Lblk28:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-8]
    sub x0, fp, #8
    bl _yalx_Zplang_Zolang_Zdprintln_stub
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
.global _main_Zomain_ZdassertString
_main_Zomain_ZdassertString:
.cfi_startproc
Lblk29:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    sub x0, fp, #8
    sub x1, fp, #16
    bl _assert_string_stub
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _main_Zomain_ZdassertInt
_main_Zomain_ZdassertInt:
.cfi_startproc
Lblk30:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    bl _assert_int_stub
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "yalx/lang:lang"
Lkzs.1:
    .asciz "Hello, World"
Lkzs.2:
    .asciz "hello"
.section __TEXT,__const
.p2align 4
; Number constants
Knnn.1:
    .long 0x3f8ccccd    ; float.1.100000
Knnn.0:
    .long -1
.section __DATA,__data
.p2align 4
.global _main_Zomain$global_slots
_main_Zomain$global_slots:
    .quad 0 ; size_in_bytes
    .quad 0 ; slots
    .quad 0 ; mark_size
; string constants:
; Yalx-String constants
.global _main_Zomain_Lksz
_main_Zomain_Lksz:
    .long 3
    .long 0 ; padding for struct lksz_header
    .quad Lkzs.0
    .quad Lkzs.1
    .quad Lkzs.2
.global _main_Zomain_Kstr
_main_Zomain_Kstr:
    .long 3
    .long 0 ; padding for struct kstr_header
Kstr.0:
    .quad 0
Kstr.1:
    .quad 0
Kstr.2:
    .quad 0
