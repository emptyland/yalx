.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "tests/44-code-gen-primitive-props/src/issue05" "foo.yalx"
.p2align 2

; functions
.global _issue05_Zoissue05_Zd_Z4init
_issue05_Zoissue05_Zd_Z4init:
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
.global _issue05_Zoissue05_Zdissue1
_issue05_Zoissue05_Zdissue1:
.cfi_startproc
Lblk1:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #2
    mov w1, #2
    mov w2, #2
    mov w3, #0
    stur w3, [fp, #-4]
    stur w2, [fp, #-8]
    stur w1, [fp, #-12]
    stur w0, [fp, #-16]
    mov w19, #3
    stur w19, [fp, #-20]
    stur w0, [fp, #-24]
    stur w1, [fp, #-28]
    stur w2, [fp, #-32]
    stur w3, [fp, #-36]
    adrp x19, _builtin_classes+864@PAGE
    add x0, x19, _builtin_classes+864@PAGEOFF
    sub x1, fp, #20
    bl _array_fill
    mov x2, x0
    ldr w0, [x2, #28]
    ldr w1, [x2, #24]
    str w1, [fp, #24]
    str w0, [fp, #28]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _issue05_Zoissue05_Zdissue1_had
_issue05_Zoissue05_Zdissue1_had:
.cfi_startproc
Lblk2:
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
    bl _current_root
    mov x26, x0
    bl _issue05_Zoissue05_Zdissue1
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
.global _issue05_Zoissue05_Zdissue2
_issue05_Zoissue05_Zdissue2:
.cfi_startproc
Lblk3:
    sub sp, sp, #128
    stp fp, lr, [sp, #112]
    add fp, sp, #112
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #3
    mov w1, #4
    mov w2, #5
    mov w3, #0
    stur w3, [fp, #-4]
    stur w2, [fp, #-8]
    stur w1, [fp, #-12]
    stur w0, [fp, #-16]
    mov w19, #3
    stur w19, [fp, #-20]
    stur w0, [fp, #-24]
    stur w1, [fp, #-28]
    stur w2, [fp, #-32]
    stur w3, [fp, #-36]
    adrp x19, _builtin_classes+864@PAGE
    add x0, x19, _builtin_classes+864@PAGEOFF
    sub x1, fp, #20
    bl _array_fill
    mov x2, x0
    mov w0, #0
    stur w0, [fp, #-4]
    stur x2, [fp, #-12]
    ldur x0, [fp, #-12]
    ldur w1, [fp, #-4]
    add sp, sp, #48
    bl _yalx_Zplang_Zolang_ZdmultiDimsArrayGetLength
    sub sp, sp, #48
    ldur x0, [fp, #-12]
    mov w1, #1
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    add sp, sp, #32
    bl _yalx_Zplang_Zolang_ZdmultiDimsArrayGetLength
    sub sp, sp, #32
    ldur x0, [fp, #-8]
    mov w1, #2
    stur w1, [fp, #-4]
    add sp, sp, #16
    bl _yalx_Zplang_Zolang_ZdmultiDimsArrayGetLength
    sub sp, sp, #16
    mov w0, #3
    stur w0, [fp, #-4]
    ldur w1, [fp, #-52]
    bl _issue05_Zoissue05_ZdassertInt
    mov w0, #4
    stur w0, [fp, #-4]
    ldur w1, [fp, #-68]
    bl _issue05_Zoissue05_ZdassertInt
    mov w0, #5
    stur w0, [fp, #-4]
    ldur w1, [fp, #-84]
    bl _issue05_Zoissue05_ZdassertInt
    ldur w19, [fp, #-84]
    str w19, [fp, #20]
    ldur w19, [fp, #-68]
    str w19, [fp, #24]
    ldur w19, [fp, #-52]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #112]
    add sp, sp, #128
    ret
.cfi_endproc
.global _issue05_Zoissue05_Zdissue2_had
_issue05_Zoissue05_Zdissue2_had:
.cfi_startproc
Lblk4:
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
    bl _current_root
    mov x26, x0
    bl _issue05_Zoissue05_Zdissue2
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
.global _issue05_Zoissue05_ZdfakeGetLength
_issue05_Zoissue05_ZdfakeGetLength:
.cfi_startproc
Lblk5:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    str w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue05_Zoissue05_ZdassertInt
_issue05_Zoissue05_ZdassertInt:
.cfi_startproc
Lblk6:
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
.section __DATA,__data
.p2align 4
.global _issue05_Zoissue05$global_slots
_issue05_Zoissue05$global_slots:
    .quad 0 ; size_in_bytes
    .quad 0 ; slots
    .quad 0 ; mark_size
; string constants:
; Yalx-String constants
.global _issue05_Zoissue05_Lksz
_issue05_Zoissue05_Lksz:
    .long 1
    .long 0 ; padding for struct lksz_header
    .quad Lkzs.0
.global _issue05_Zoissue05_Kstr
_issue05_Zoissue05_Kstr:
    .long 1
    .long 0 ; padding for struct kstr_header
Kstr.0:
    .quad 0
