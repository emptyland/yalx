.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "tests/46-code-gen-call-virtual/src/issue07" "foo.yalx"
.p2align 2

; functions
.global _issue07_Zoissue07_Zd_Z4init
_issue07_Zoissue07_Zd_Z4init:
.cfi_startproc
Lblk11:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _yalx_Zplang_Zolang_Zd_Z4init@PAGE
    add x0, x19, _yalx_Zplang_Zolang_Zd_Z4init@PAGEOFF
    adrp x19, Lkzs.3@PAGE
    add x1, x19, Lkzs.3@PAGEOFF
    bl _pkg_init_once
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue07_Zoissue07_Zdissue1
_issue07_Zoissue07_Zdissue1:
.cfi_startproc
Lblk12:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _issue07_Zoissue07_ZdFoo$class@PAGE
    add x0, x19, _issue07_Zoissue07_ZdFoo$class@PAGEOFF
    bl _heap_alloc
    mov w1, #222
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    bl _issue07_Zoissue07_ZdFoo_ZdFoo_Z4constructor
    ldur x0, [fp, #-8]
    stur x0, [fp, #-8]
    add sp, sp, #32
    ldr x19, [x0, #0]
    and x19, x19, #-2
    ldr x19, [x19, #128]
    ldr x19, [x19, #8]
    blr x19
    sub sp, sp, #32
    ldur x0, [fp, #-8]
    add sp, sp, #16
    ldr x19, [x0, #0]
    and x19, x19, #-2
    ldr x19, [x19, #128]
    ldr x19, [x19, #32]
    blr x19
    sub sp, sp, #16
    ldur x19, [fp, #-40]
    str x19, [fp, #20]
    ldur w19, [fp, #-20]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.cfi_endproc
.global _issue07_Zoissue07_Zdissue1_had
_issue07_Zoissue07_Zdissue1_had:
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
    bl _current_root
    mov x26, x0
    bl _issue07_Zoissue07_Zdissue1
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
.global _issue07_Zoissue07_Zdissue2
_issue07_Zoissue07_Zdissue2:
.cfi_startproc
Lblk14:
    sub sp, sp, #96
    stp fp, lr, [sp, #80]
    add fp, sp, #80
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _issue07_Zoissue07_ZdFoo$class@PAGE
    add x0, x19, _issue07_Zoissue07_ZdFoo$class@PAGEOFF
    bl _heap_alloc
    mov w1, #222
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    bl _issue07_Zoissue07_ZdFoo_ZdFoo_Z4constructor
    ldur x0, [fp, #-8]
    mov x1, x0
    stur x0, [fp, #-8]
    stur x1, [fp, #-20]
    ldur x0, [fp, #-20]
    add sp, sp, #32
    ldr x19, [x0, #0]
    and x19, x19, #-2
    ldr x19, [x19, #128]
    ldr x19, [x19, #8]
    blr x19
    sub sp, sp, #32
    ldur x0, [fp, #-20]
    add sp, sp, #16
    ldr x19, [x0, #0]
    and x19, x19, #-2
    ldr x19, [x19, #128]
    ldr x19, [x19, #32]
    blr x19
    sub sp, sp, #16
    ldur x19, [fp, #-56]
    str x19, [fp, #20]
    ldur w19, [fp, #-36]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #80]
    add sp, sp, #96
    ret
.cfi_endproc
.global _issue07_Zoissue07_Zdissue2_had
_issue07_Zoissue07_Zdissue2_had:
.cfi_startproc
Lblk15:
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
    bl _issue07_Zoissue07_Zdissue2
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
.global _issue07_Zoissue07_Zdissue3
_issue07_Zoissue07_Zdissue3:
.cfi_startproc
Lblk16:
    sub sp, sp, #96
    stp fp, lr, [sp, #80]
    add fp, sp, #80
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _issue07_Zoissue07_ZdFoo$class@PAGE
    add x0, x19, _issue07_Zoissue07_ZdFoo$class@PAGEOFF
    bl _heap_alloc
    mov w1, #111
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    bl _issue07_Zoissue07_ZdFoo_ZdFoo_Z4constructor
    ldur x0, [fp, #-8]
    stur x0, [fp, #-28]
    ldr x19, [x0, #0]
    and x19, x19, #-2
    ldr x19, [x19, #136]
    stur x19, [fp, #-20]
    stur x0, [fp, #-8]
    sub x0, fp, #28
    add sp, sp, #32
    mov x19, x0
    ldr x0, [x19, #0]
    ldr x19, [x19, #8]
    ldr x19, [x19, #0]
    blr x19
    sub sp, sp, #32
    stur x0, [fp, #-44]
    sub x0, fp, #28
    add sp, sp, #16
    mov x19, x0
    ldr x0, [x19, #0]
    ldr x19, [x19, #8]
    ldr x19, [x19, #8]
    blr x19
    sub sp, sp, #16
    ldur x19, [fp, #-56]
    str x19, [fp, #20]
    ldur w19, [fp, #-36]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #80]
    add sp, sp, #96
    ret
.cfi_endproc
.global _issue07_Zoissue07_Zdissue3_had
_issue07_Zoissue07_Zdissue3_had:
.cfi_startproc
Lblk17:
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
    bl _issue07_Zoissue07_Zdissue3
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
.global _issue07_Zoissue07_Zdissue4
_issue07_Zoissue07_Zdissue4:
.cfi_startproc
Lblk18:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #228
    stur w0, [fp, #-4]
    stur w0, [fp, #-8]
    adrp x19, _issue07_Zoissue07_Zdanonymous_Zdfun0_Zdclosure$class@PAGE
    add x0, x19, _issue07_Zoissue07_Zdanonymous_Zdfun0_Zdclosure$class@PAGEOFF
    sub x1, fp, #4
    mov x2, #4
    bl _closure
    mov x3, x0
    str x3, [fp, #24]
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
.global _issue07_Zoissue07_Zdissue4_had
_issue07_Zoissue07_Zdissue4_had:
.cfi_startproc
Lblk19:
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
    bl _issue07_Zoissue07_Zdissue4
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
.global _issue07_Zoissue07_Zdissue5
_issue07_Zoissue07_Zdissue5:
.cfi_startproc
Lblk20:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #515
    mov w1, #119
    stur w0, [fp, #-4]
    sturb w1, [fp, #-8]
    stur w0, [fp, #-12]
    sturb w1, [fp, #-16]
    adrp x19, _issue07_Zoissue07_Zdanonymous_Zdfun1_Zdclosure$class@PAGE
    add x0, x19, _issue07_Zoissue07_Zdanonymous_Zdfun1_Zdclosure$class@PAGEOFF
    sub x1, fp, #8
    mov x2, #8
    bl _closure
    mov x3, x0
    str x3, [fp, #24]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue07_Zoissue07_Zdissue5_had
_issue07_Zoissue07_Zdissue5_had:
.cfi_startproc
Lblk21:
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
    bl _issue07_Zoissue07_Zdissue5
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
.global _issue07_Zoissue07_Zdissue6
_issue07_Zoissue07_Zdissue6:
.cfi_startproc
Lblk22:
    sub sp, sp, #96
    stp fp, lr, [sp, #80]
    add fp, sp, #80
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #711
    stur w0, [fp, #-4]
    stur w0, [fp, #-8]
    adrp x19, _issue07_Zoissue07_Zdanonymous_Zdfun2_Zdclosure$class@PAGE
    add x0, x19, _issue07_Zoissue07_Zdanonymous_Zdfun2_Zdclosure$class@PAGEOFF
    sub x1, fp, #4
    mov x2, #4
    bl _closure
    mov x3, x0
    mov w0, #1
    stur w0, [fp, #-4]
    stur x3, [fp, #-16]
    ldur x0, [fp, #-16]
    ldur w1, [fp, #-4]
    add sp, sp, #48
    ldr x19, [x0, #16]
    blr x19
    sub sp, sp, #48
    ldur x0, [fp, #-16]
    mov w1, #2
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    add sp, sp, #32
    ldr x19, [x0, #16]
    blr x19
    sub sp, sp, #32
    ldur x0, [fp, #-8]
    mov w1, #3
    stur w1, [fp, #-4]
    add sp, sp, #16
    ldr x19, [x0, #16]
    blr x19
    sub sp, sp, #16
    ldur w19, [fp, #-52]
    str w19, [fp, #20]
    ldur w19, [fp, #-36]
    str w19, [fp, #24]
    ldur w19, [fp, #-20]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #80]
    add sp, sp, #96
    ret
.cfi_endproc
.global _issue07_Zoissue07_Zdissue6_had
_issue07_Zoissue07_Zdissue6_had:
.cfi_startproc
Lblk23:
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
    bl _issue07_Zoissue07_Zdissue6
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
.global _issue07_Zoissue07_Zdissue7
_issue07_Zoissue07_Zdissue7:
.cfi_startproc
Lblk24:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _issue07_Zoissue07_ZdFoo$class@PAGE
    add x0, x19, _issue07_Zoissue07_ZdFoo$class@PAGEOFF
    bl _heap_alloc
    mov w1, #1
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    bl _issue07_Zoissue07_ZdFoo_ZdFoo_Z4constructor
    ldur x0, [fp, #-8]
    mov w1, #911
    stur w1, [fp, #-16]
    stur x0, [fp, #-24]
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    adrp x19, _issue07_Zoissue07_Zdanonymous_Zdfun3_Zdclosure$class@PAGE
    add x0, x19, _issue07_Zoissue07_Zdanonymous_Zdfun3_Zdclosure$class@PAGEOFF
    sub x1, fp, #24
    mov x2, #12
    bl _closure
    mov x3, x0
    str x3, [fp, #24]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue07_Zoissue07_Zdissue7_had
_issue07_Zoissue07_Zdissue7_had:
.cfi_startproc
Lblk25:
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
    bl _issue07_Zoissue07_Zdissue7
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
.global _issue07_Zoissue07_ZdFoo_ZdhashCode
_issue07_Zoissue07_ZdFoo_ZdhashCode:
.cfi_startproc
Lblk0:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    str w1, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue07_Zoissue07_ZdFoo_ZdtoString
_issue07_Zoissue07_ZdFoo_ZdtoString:
.cfi_startproc
Lblk1:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, Kstr.0@PAGE
    add x19, x19, Kstr.0@PAGEOFF
    ldr x1, [x19, #0]
    str x1, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue07_Zoissue07_ZdFoo_Zdfoo0
_issue07_Zoissue07_ZdFoo_Zdfoo0:
.cfi_startproc
Lblk2:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w1, #101
    str w1, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue07_Zoissue07_ZdFoo_Zdfoo1
_issue07_Zoissue07_ZdFoo_Zdfoo1:
.cfi_startproc
Lblk3:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, Kstr.1@PAGE
    add x19, x19, Kstr.1@PAGEOFF
    ldr x1, [x19, #0]
    str x1, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue07_Zoissue07_ZdFoo_Zdfoo2
_issue07_Zoissue07_ZdFoo_Zdfoo2:
.cfi_startproc
Lblk4:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w1, #201
    str w1, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue07_Zoissue07_ZdFoo_Zdfoo3
_issue07_Zoissue07_ZdFoo_Zdfoo3:
.cfi_startproc
Lblk5:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x1, [x19, #0]
    str x1, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue07_Zoissue07_ZdFoo_ZdFoo_Z4constructor
_issue07_Zoissue07_ZdFoo_ZdFoo_Z4constructor:
.cfi_startproc
Lblk6:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov x2, x0
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    stur x2, [fp, #-20]
    ldur x0, [fp, #-20]
    bl _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    ldur x0, [fp, #-8]
    ldur w1, [fp, #-12]
    str w1, [x0, #16]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _anonymous_Zdfun0_Zdclosure_Zdapply
_anonymous_Zdfun0_Zdclosure_Zdapply:
.cfi_startproc
Lblk7:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w2, [x0, #24]
    add w3, w2, #1
    str w3, [x0, #24]
    ldr w2, [x0, #24]
    add w0, w2, w1
    str w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _anonymous_Zdfun1_Zdclosure_Zdapply
_anonymous_Zdfun1_Zdclosure_Zdapply:
.cfi_startproc
Lblk8:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w2, [x0, #24]
    add w3, w2, #1
    strb w3, [x0, #24]
    ldr w2, [x0, #26]
    add w3, w2, #2
    str w3, [x0, #26]
    ldr w2, [x0, #24]
    add w3, w2, w1
    ldr w1, [x0, #26]
    str w1, [fp, #24]
    strb w3, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _anonymous_Zdfun2_Zdclosure_Zdapply
_anonymous_Zdfun2_Zdclosure_Zdapply:
.cfi_startproc
Lblk9:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w2, [x0, #24]
    add w3, w2, #1
    str w3, [x0, #24]
    ldr w2, [x0, #24]
    add w0, w2, w1
    str w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _anonymous_Zdfun3_Zdclosure_Zdapply
_anonymous_Zdfun3_Zdclosure_Zdapply:
.cfi_startproc
Lblk10:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr x1, [x0, #24]
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    ldur x0, [fp, #-16]
    add sp, sp, #32
    bl _issue07_Zoissue07_ZdFoo_Zdfoo0
    sub sp, sp, #32
    ldur x0, [fp, #-8]
    ldr x1, [x0, #24]
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    ldur x0, [fp, #-16]
    add sp, sp, #16
    bl _issue07_Zoissue07_ZdFoo_Zdfoo1
    sub sp, sp, #16
    ldur x0, [fp, #-8]
    ldr w1, [x0, #32]
    str w1, [fp, #16]
    ldur x19, [fp, #-40]
    str x19, [fp, #20]
    ldur w19, [fp, #-20]
    str w19, [fp, #28]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.cfi_endproc
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "Foo"
Lkzs.1:
    .asciz "call foo1"
Lkzs.2:
    .asciz "call foo3"
Lkzs.3:
    .asciz "yalx/lang:lang"
Lkzs.4:
    .asciz "fun (issue07:issue07.Foo)->(u32)"
Lkzs.5:
    .asciz "hashCode"
Lkzs.6:
    .asciz "fun (issue07:issue07.Foo)->(string)"
Lkzs.7:
    .asciz "toString"
Lkzs.8:
    .asciz "fun (issue07:issue07.Foo)->(i32)"
Lkzs.9:
    .asciz "foo0"
Lkzs.10:
    .asciz "foo1"
Lkzs.11:
    .asciz "foo2"
Lkzs.12:
    .asciz "foo3"
Lkzs.13:
    .asciz "fun (issue07:issue07.Foo,u32)->(void)"
Lkzs.14:
    .asciz "Foo$constructor"
Lkzs.15:
    .asciz "n"
Lkzs.16:
    .asciz "issue07:issue07.Foo"
Lkzs.17:
    .asciz "fun (issue07:issue07.anonymous.fun0.closure,i32)->(i32)"
Lkzs.18:
    .asciz "apply"
Lkzs.19:
    .asciz "$fun_entry$"
Lkzs.20:
    .asciz "a"
Lkzs.21:
    .asciz "anonymous.fun0.closure"
Lkzs.22:
    .asciz "issue07:issue07.anonymous.fun0.closure"
Lkzs.23:
    .asciz "fun (issue07:issue07.anonymous.fun1.closure,u8)->(u8,u16)"
Lkzs.24:
    .asciz "b"
Lkzs.25:
    .asciz "anonymous.fun1.closure"
Lkzs.26:
    .asciz "issue07:issue07.anonymous.fun1.closure"
Lkzs.27:
    .asciz "fun (issue07:issue07.anonymous.fun2.closure,i32)->(i32)"
Lkzs.28:
    .asciz "anonymous.fun2.closure"
Lkzs.29:
    .asciz "issue07:issue07.anonymous.fun2.closure"
Lkzs.30:
    .asciz "fun (issue07:issue07.anonymous.fun3.closure)->(i32,string,i32)"
Lkzs.31:
    .asciz "foo"
Lkzs.32:
    .asciz "anonymous.fun3.closure"
Lkzs.33:
    .asciz "issue07:issue07.anonymous.fun3.closure"
.section __DATA,__data
.p2align 4
; classes:
.global _issue07_Zoissue07_ZdFoo$class
_issue07_Zoissue07_ZdFoo$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 24 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.0 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.16 ; location
    .long 19 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 1 ; n_fields
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo$fields ; fields
    .quad _issue07_Zoissue07_ZdFoo$ctor ; ctor
    .long 7 ; n_methods
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo$methods ; methods
    .long 5 ; n_vtab
    .long 4 ; n_itab
    .quad _issue07_Zoissue07_ZdFoo$vtab ; vtab
    .quad _issue07_Zoissue07_ZdFoo$itab ; itab
    .long 0 ; refs_mark_len
    .space 4
_issue07_Zoissue07_ZdFoo$fields:
    ; Foo::n
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.15 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+1064 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
_issue07_Zoissue07_ZdFoo$methods:
    ; Foo::hashCode
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.5 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.4 ; prototype_desc
    .long 32 ; prototype_desc
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo_ZdhashCode ; entry
    ; Foo::toString
    .long 1 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.7 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.6 ; prototype_desc
    .long 35 ; prototype_desc
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo_ZdtoString ; entry
    ; Foo::foo0
    .long 2 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.9 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad Lkzs.8 ; prototype_desc
    .long 32 ; prototype_desc
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo_Zdfoo0 ; entry
    ; Foo::foo1
    .long 3 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.10 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad Lkzs.6 ; prototype_desc
    .long 35 ; prototype_desc
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo_Zdfoo1 ; entry
    ; Foo::foo2
    .long 4 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.11 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad Lkzs.8 ; prototype_desc
    .long 32 ; prototype_desc
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo_Zdfoo2 ; entry
    ; Foo::foo3
    .long 5 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.12 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad Lkzs.6 ; prototype_desc
    .long 35 ; prototype_desc
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo_Zdfoo3 ; entry
_issue07_Zoissue07_ZdFoo$ctor:
    ; Foo::Foo$constructor
    .long 6 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.14 ; name
    .long 15 ; name
    .space 4 ; padding
    .quad Lkzs.13 ; prototype_desc
    .long 37 ; prototype_desc
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo_ZdFoo_Z4constructor ; entry
_issue07_Zoissue07_ZdFoo$vtab:
    .quad _yalx_Zplang_Zolang_ZdAny_Zdfinalize
    .quad _issue07_Zoissue07_ZdFoo_ZdhashCode
    .quad _yalx_Zplang_Zolang_ZdAny_Zdid
    .quad _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
    .quad _issue07_Zoissue07_ZdFoo_ZdtoString
_issue07_Zoissue07_ZdFoo$itab:
    .quad _issue07_Zoissue07_ZdFoo_Zdfoo0
    .quad _issue07_Zoissue07_ZdFoo_Zdfoo1
    .quad _issue07_Zoissue07_ZdFoo_Zdfoo2
    .quad _issue07_Zoissue07_ZdFoo_Zdfoo3
.global _issue07_Zoissue07_Zdanonymous_Zdfun0_Zdclosure$class
_issue07_Zoissue07_Zdanonymous_Zdfun0_Zdclosure$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 32 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.21 ; name
    .long 22 ; name
    .space 4 ; padding
    .quad Lkzs.22 ; location
    .long 38 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 2 ; n_fields
    .space 4 ; padding
    .quad _issue07_Zoissue07_Zdanonymous_Zdfun0_Zdclosure$fields ; fields
    .quad 0 ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue07_Zoissue07_Zdanonymous_Zdfun0_Zdclosure$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_issue07_Zoissue07_Zdanonymous_Zdfun0_Zdclosure$fields:
    ; anonymous.fun0.closure::$fun_entry$
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.19 ; name
    .long 11 ; name
    .space 4 ; padding
    .quad _builtin_classes+1064 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; anonymous.fun0.closure::a
    .long 1 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.20 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
_issue07_Zoissue07_Zdanonymous_Zdfun0_Zdclosure$methods:
    ; anonymous.fun0.closure::apply
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.18 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.17 ; prototype_desc
    .long 55 ; prototype_desc
    .space 4 ; padding
    .quad _anonymous_Zdfun0_Zdclosure_Zdapply ; entry
.global _issue07_Zoissue07_Zdanonymous_Zdfun1_Zdclosure$class
_issue07_Zoissue07_Zdanonymous_Zdfun1_Zdclosure$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 32 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.25 ; name
    .long 22 ; name
    .space 4 ; padding
    .quad Lkzs.26 ; location
    .long 38 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 3 ; n_fields
    .space 4 ; padding
    .quad _issue07_Zoissue07_Zdanonymous_Zdfun1_Zdclosure$fields ; fields
    .quad 0 ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue07_Zoissue07_Zdanonymous_Zdfun1_Zdclosure$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_issue07_Zoissue07_Zdanonymous_Zdfun1_Zdclosure$fields:
    ; anonymous.fun1.closure::$fun_entry$
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.19 ; name
    .long 11 ; name
    .space 4 ; padding
    .quad _builtin_classes+1064 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; anonymous.fun1.closure::a
    .long 1 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.20 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+456 ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
    ; anonymous.fun1.closure::b
    .long 1 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.24 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+760 ; type
    .long 26 ; offset_of_head
    .space 4 ; padding
_issue07_Zoissue07_Zdanonymous_Zdfun1_Zdclosure$methods:
    ; anonymous.fun1.closure::apply
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.18 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.23 ; prototype_desc
    .long 57 ; prototype_desc
    .space 4 ; padding
    .quad _anonymous_Zdfun1_Zdclosure_Zdapply ; entry
.global _issue07_Zoissue07_Zdanonymous_Zdfun2_Zdclosure$class
_issue07_Zoissue07_Zdanonymous_Zdfun2_Zdclosure$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 32 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.28 ; name
    .long 22 ; name
    .space 4 ; padding
    .quad Lkzs.29 ; location
    .long 38 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 2 ; n_fields
    .space 4 ; padding
    .quad _issue07_Zoissue07_Zdanonymous_Zdfun2_Zdclosure$fields ; fields
    .quad 0 ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue07_Zoissue07_Zdanonymous_Zdfun2_Zdclosure$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_issue07_Zoissue07_Zdanonymous_Zdfun2_Zdclosure$fields:
    ; anonymous.fun2.closure::$fun_entry$
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.19 ; name
    .long 11 ; name
    .space 4 ; padding
    .quad _builtin_classes+1064 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; anonymous.fun2.closure::a
    .long 1 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.20 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
_issue07_Zoissue07_Zdanonymous_Zdfun2_Zdclosure$methods:
    ; anonymous.fun2.closure::apply
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.18 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.27 ; prototype_desc
    .long 55 ; prototype_desc
    .space 4 ; padding
    .quad _anonymous_Zdfun2_Zdclosure_Zdapply ; entry
.global _issue07_Zoissue07_Zdanonymous_Zdfun3_Zdclosure$class
_issue07_Zoissue07_Zdanonymous_Zdfun3_Zdclosure$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 40 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.32 ; name
    .long 22 ; name
    .space 4 ; padding
    .quad Lkzs.33 ; location
    .long 38 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 3 ; n_fields
    .space 4 ; padding
    .quad _issue07_Zoissue07_Zdanonymous_Zdfun3_Zdclosure$fields ; fields
    .quad 0 ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue07_Zoissue07_Zdanonymous_Zdfun3_Zdclosure$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 1 ; refs_mark_len
    .space 4
    .quad _issue07_Zoissue07_ZdFoo$class ; type
    .long 24
_issue07_Zoissue07_Zdanonymous_Zdfun3_Zdclosure$fields:
    ; anonymous.fun3.closure::$fun_entry$
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.19 ; name
    .long 11 ; name
    .space 4 ; padding
    .quad _builtin_classes+1064 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; anonymous.fun3.closure::foo
    .long 1 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.31 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo$class ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
    ; anonymous.fun3.closure::a
    .long 1 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.20 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 32 ; offset_of_head
    .space 4 ; padding
_issue07_Zoissue07_Zdanonymous_Zdfun3_Zdclosure$methods:
    ; anonymous.fun3.closure::apply
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.18 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.30 ; prototype_desc
    .long 62 ; prototype_desc
    .space 4 ; padding
    .quad _anonymous_Zdfun3_Zdclosure_Zdapply ; entry
.section __DATA,__data
.p2align 4
.global _issue07_Zoissue07$global_slots
_issue07_Zoissue07$global_slots:
    .quad 0 ; size_in_bytes
    .quad 0 ; slots
    .quad 0 ; mark_size
; string constants:
; Yalx-String constants
.global _issue07_Zoissue07_Lksz
_issue07_Zoissue07_Lksz:
    .long 34
    .long 0 ; padding for struct lksz_header
    .quad Lkzs.0
    .quad Lkzs.1
    .quad Lkzs.2
    .quad Lkzs.3
    .quad Lkzs.4
    .quad Lkzs.5
    .quad Lkzs.6
    .quad Lkzs.7
    .quad Lkzs.8
    .quad Lkzs.9
    .quad Lkzs.10
    .quad Lkzs.11
    .quad Lkzs.12
    .quad Lkzs.13
    .quad Lkzs.14
    .quad Lkzs.15
    .quad Lkzs.16
    .quad Lkzs.17
    .quad Lkzs.18
    .quad Lkzs.19
    .quad Lkzs.20
    .quad Lkzs.21
    .quad Lkzs.22
    .quad Lkzs.23
    .quad Lkzs.24
    .quad Lkzs.25
    .quad Lkzs.26
    .quad Lkzs.27
    .quad Lkzs.28
    .quad Lkzs.29
    .quad Lkzs.30
    .quad Lkzs.31
    .quad Lkzs.32
    .quad Lkzs.33
.global _issue07_Zoissue07_Kstr
_issue07_Zoissue07_Kstr:
    .long 34
    .long 0 ; padding for struct kstr_header
Kstr.0:
    .quad 0
Kstr.1:
    .quad 0
Kstr.2:
    .quad 0
Kstr.3:
    .quad 0
Kstr.4:
    .quad 0
Kstr.5:
    .quad 0
Kstr.6:
    .quad 0
Kstr.7:
    .quad 0
Kstr.8:
    .quad 0
Kstr.9:
    .quad 0
Kstr.10:
    .quad 0
Kstr.11:
    .quad 0
Kstr.12:
    .quad 0
Kstr.13:
    .quad 0
Kstr.14:
    .quad 0
Kstr.15:
    .quad 0
Kstr.16:
    .quad 0
Kstr.17:
    .quad 0
Kstr.18:
    .quad 0
Kstr.19:
    .quad 0
Kstr.20:
    .quad 0
Kstr.21:
    .quad 0
Kstr.22:
    .quad 0
Kstr.23:
    .quad 0
Kstr.24:
    .quad 0
Kstr.25:
    .quad 0
Kstr.26:
    .quad 0
Kstr.27:
    .quad 0
Kstr.28:
    .quad 0
Kstr.29:
    .quad 0
Kstr.30:
    .quad 0
Kstr.31:
    .quad 0
Kstr.32:
    .quad 0
Kstr.33:
    .quad 0
