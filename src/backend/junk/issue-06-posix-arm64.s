.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "tests/45-code-gen-enum-types/src/issue06" "foo.yalx"
.p2align 2

; functions
.global _issue06_Zoissue06_Zd_Z4init
_issue06_Zoissue06_Zd_Z4init:
.cfi_startproc
Lblk15:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _yalx_Zplang_Zolang_Zd_Z4init@PAGE
    add x0, x19, _yalx_Zplang_Zolang_Zd_Z4init@PAGEOFF
    adrp x19, Lkzs.1@PAGE
    add x1, x19, Lkzs.1@PAGEOFF
    bl _pkg_init_once
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue1
_issue06_Zoissue06_Zdissue1:
.cfi_startproc
Lblk16:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #0
    stur w0, [fp, #-16]
    mov w0, #4
    stur w0, [fp, #-32]
    ldur x19, [fp, #-32]
    str x19, [fp, #16]
    ldur x19, [fp, #-24]
    str x19, [fp, #24]
    ldur x19, [fp, #-16]
    str x19, [fp, #32]
    ldur x19, [fp, #-8]
    str x19, [fp, #40]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue1_had
_issue06_Zoissue06_Zdissue1_had:
.cfi_startproc
Lblk17:
    sub sp, sp, #128
    stp fp, lr, [sp, #112]
    add fp, sp, #112
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stp x19, x20, [sp, #96]
    stp x21, x22, [sp, #80]
    stp x23, x24, [sp, #64]
    stp x25, x26, [sp, #48]
    stp x27, x28, [sp, #32]
    add fp, sp, #32
    bl _current_root
    mov x26, x0
    bl _issue06_Zoissue06_Zdissue1
    mov x0, #32
    bl _reserve_handle_returning_vals
    mov x1, sp
    mov x2, #32
    bl _memcpy
    ldp x19, x20, [sp, #96]
    ldp x21, x22, [sp, #80]
    ldp x23, x24, [sp, #64]
    ldp x25, x26, [sp, #48]
    ldp x27, x28, [sp, #32]
    ldp fp, lr, [sp, #112]
    add sp, sp, #128
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue2
_issue06_Zoissue06_Zdissue2:
.cfi_startproc
Lblk18:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #1
    stur w0, [fp, #-16]
    mov w0, #122
    sturb w0, [fp, #-8]
    ldur x19, [fp, #-16]
    str x19, [fp, #16]
    ldur x19, [fp, #-8]
    str x19, [fp, #24]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue2_had
_issue06_Zoissue06_Zdissue2_had:
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
    bl _issue06_Zoissue06_Zdissue2
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
.global _issue06_Zoissue06_Zdissue3
_issue06_Zoissue06_Zdissue3:
.cfi_startproc
Lblk20:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #3
    stur w0, [fp, #-16]
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-8]
    ldur x19, [fp, #-16]
    str x19, [fp, #16]
    ldur x19, [fp, #-8]
    str x19, [fp, #24]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue3_had
_issue06_Zoissue06_Zdissue3_had:
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
    bl _issue06_Zoissue06_Zdissue3
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
.global _issue06_Zoissue06_Zdissue4
_issue06_Zoissue06_Zdissue4:
.cfi_startproc
Lblk22:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _issue06_Zoissue06_ZdBaz$class@PAGE
    add x0, x19, _issue06_Zoissue06_ZdBaz$class@PAGEOFF
    bl _heap_alloc
    mov w1, #996
    mov w2, #700
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    stur w2, [fp, #-16]
    bl _issue06_Zoissue06_ZdBaz_ZdBaz_Z4constructor
    ldur x0, [fp, #-8]
    str x0, [fp, #24]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue4_had
_issue06_Zoissue06_Zdissue4_had:
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
    bl _issue06_Zoissue06_Zdissue4
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
.global _issue06_Zoissue06_Zdissue5
_issue06_Zoissue06_Zdissue5:
.cfi_startproc
Lblk24:
    sub sp, sp, #96
    stp fp, lr, [sp, #80]
    add fp, sp, #80
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _issue06_Zoissue06_ZdBaz$class@PAGE
    add x0, x19, _issue06_Zoissue06_ZdBaz$class@PAGEOFF
    bl _heap_alloc
    mov w1, #996
    mov w2, #700
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    stur w2, [fp, #-16]
    bl _issue06_Zoissue06_ZdBaz_ZdBaz_Z4constructor
    ldur x0, [fp, #-8]
    add x1, fp, #-48
    mov w2, #100
    adrp x19, Kstr.3@PAGE
    add x19, x19, Kstr.3@PAGEOFF
    ldr x3, [x19, #0]
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    stur w2, [fp, #-52]
    stur x3, [fp, #-60]
    ldur x0, [fp, #-16]
    ldur w1, [fp, #-52]
    ldur x2, [fp, #-60]
    bl _issue06_Zoissue06_ZdBzz_ZdBzz_Z4constructor
    ldur x0, [fp, #-8]
    stur x0, [fp, #-8]
    ldur x0, [fp, #-8]
    ldur x0, [fp, #-8]
    add x1, x0, #24
    sub x2, fp, #48
    bl _put_field_chunk
    adrp x19, Kstr.4@PAGE
    add x19, x19, Kstr.4@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-16]
    ldur x0, [fp, #-8]
    add x0, x0, #56
    ldur x1, [fp, #-16]
    bl _put_field
    adrp x19, Kstr.5@PAGE
    add x19, x19, Kstr.5@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-16]
    stur x0, [fp, #-24]
    ldur x0, [fp, #-8]
    add x0, x0, #64
    ldur x1, [fp, #-16]
    bl _put_field
    ldur x19, [fp, #-8]
    str x19, [fp, #24]
    ldp fp, lr, [sp, #80]
    add sp, sp, #96
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue5_had
_issue06_Zoissue06_Zdissue5_had:
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
    bl _issue06_Zoissue06_Zdissue5
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
.global _issue06_Zoissue06_Zdissue10
_issue06_Zoissue06_Zdissue10:
.cfi_startproc
Lblk26:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #0
    stur w0, [fp, #-8]
    ldur x19, [fp, #-8]
    str x19, [fp, #24]
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue10_had
_issue06_Zoissue06_Zdissue10_had:
.cfi_startproc
Lblk27:
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
    bl _issue06_Zoissue06_Zdissue10
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
.global _issue06_Zoissue06_Zdissue11
_issue06_Zoissue06_Zdissue11:
.cfi_startproc
Lblk28:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, Kstr.6@PAGE
    add x19, x19, Kstr.6@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-8]
    adrp x19, Kstr.0@PAGE
    add x19, x19, Kstr.0@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-16]
    ldur x0, [fp, #-8]
    ldur x1, [fp, #-16]
    add sp, sp, #0
    bl _issue06_Zoissue06_ZdOption_Dkstring_Dl_ZdunwarpOr
    sub sp, sp, #0
    ldur x19, [fp, #-24]
    str x19, [fp, #24]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue11_had
_issue06_Zoissue06_Zdissue11_had:
.cfi_startproc
Lblk29:
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
    bl _issue06_Zoissue06_Zdissue11
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
.global _issue06_Zoissue06_Zdissue12
_issue06_Zoissue06_Zdissue12:
.cfi_startproc
Lblk30:
    sub sp, sp, #128
    stp fp, lr, [sp, #112]
    add fp, sp, #112
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    add x0, fp, #-24
    mov w1, #1
    mov w2, #2
    stur w1, [fp, #-28]
    stur w2, [fp, #-32]
    bl _issue06_Zoissue06_ZdBar_ZdBar_Z4constructor
    mov w0, #1
    stur w0, [fp, #-64]
    ldur x19, [fp, #-24]
    stur x19, [fp, #-56]
    ldur x19, [fp, #-16]
    stur x19, [fp, #-48]
    ldur x19, [fp, #-8]
    stur x19, [fp, #-40]
    add x0, fp, #-24
    mov w1, #996
    mov w2, #700
    stur w1, [fp, #-28]
    stur w2, [fp, #-32]
    bl _issue06_Zoissue06_ZdBar_ZdBar_Z4constructor
    stur x0, [fp, #-32]
    ldur x19, [fp, #-64]
    str x19, [x0, #0]
    ldur x19, [fp, #-56]
    str x19, [x0, #8]
    ldur x19, [fp, #-48]
    str x19, [x0, #16]
    ldur x19, [fp, #-40]
    str x19, [x0, #24]
    ldur x19, [fp, #-24]
    str x19, [x1, #0]
    ldur x19, [fp, #-16]
    str x19, [x1, #8]
    ldur x19, [fp, #-8]
    str x19, [x1, #16]
    add sp, sp, #16
    bl _yalx_Zplang_Zolang_ZdOptional_Dkissue06_Zoissue06_ZdBar_Dl_ZdunwarpOr
    sub sp, sp, #16
    ldur x19, [fp, #-88]
    str x19, [fp, #24]
    ldur x19, [fp, #-80]
    str x19, [fp, #32]
    ldur x19, [fp, #-72]
    str x19, [fp, #40]
    ldp fp, lr, [sp, #112]
    add sp, sp, #128
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue12_had
_issue06_Zoissue06_Zdissue12_had:
.cfi_startproc
Lblk31:
    sub sp, sp, #128
    stp fp, lr, [sp, #112]
    add fp, sp, #112
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stp x19, x20, [sp, #96]
    stp x21, x22, [sp, #80]
    stp x23, x24, [sp, #64]
    stp x25, x26, [sp, #48]
    stp x27, x28, [sp, #32]
    add fp, sp, #32
    bl _current_root
    mov x26, x0
    bl _issue06_Zoissue06_Zdissue12
    mov x0, #32
    bl _reserve_handle_returning_vals
    mov x1, sp
    mov x2, #32
    bl _memcpy
    ldp x19, x20, [sp, #96]
    ldp x21, x22, [sp, #80]
    ldp x23, x24, [sp, #64]
    ldp x25, x26, [sp, #48]
    ldp x27, x28, [sp, #32]
    ldp fp, lr, [sp, #112]
    add sp, sp, #128
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue13
_issue06_Zoissue06_Zdissue13:
.cfi_startproc
Lblk32:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-8]
    mov w0, #2
    mov x19, #0
    stur x19, [fp, #-16]
    ldur x19, [fp, #-8]
    stur x19, [fp, #-24]
    ldur x19, [fp, #-16]
    stur x19, [fp, #-32]
    stur w0, [fp, #-36]
    mov w19, #1
    stur w19, [fp, #-40]
    stur w0, [fp, #-44]
    adrp x19, _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$class@PAGE
    add x0, x19, _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$class@PAGEOFF
    sub x1, fp, #40
    mov w2, #2
    bl _array_alloc
    str x0, [fp, #24]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue13_had
_issue06_Zoissue06_Zdissue13_had:
.cfi_startproc
Lblk33:
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
    bl _issue06_Zoissue06_Zdissue13
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
.global _issue06_Zoissue06_Zdissue14
_issue06_Zoissue06_Zdissue14:
.cfi_startproc
Lblk34:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #2
    mov w1, #2
    mov x19, #0
    stur x19, [fp, #-8]
    ldur x19, [fp, #-8]
    stur x19, [fp, #-16]
    stur w1, [fp, #-20]
    stur w0, [fp, #-24]
    mov w19, #2
    stur w19, [fp, #-28]
    stur w0, [fp, #-32]
    stur w1, [fp, #-36]
    adrp x19, _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$class@PAGE
    add x0, x19, _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$class@PAGEOFF
    sub x1, fp, #28
    bl _array_fill
    mov x2, x0
    str x2, [fp, #24]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue14_had
_issue06_Zoissue06_Zdissue14_had:
.cfi_startproc
Lblk35:
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
    bl _issue06_Zoissue06_Zdissue14
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
.global _issue06_Zoissue06_Zdissue15
_issue06_Zoissue06_Zdissue15:
.cfi_startproc
Lblk36:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #2
    mov w1, #2
    mov x19, #0
    stur x19, [fp, #-8]
    ldur x19, [fp, #-8]
    stur x19, [fp, #-16]
    stur w1, [fp, #-20]
    stur w0, [fp, #-24]
    mov w19, #2
    stur w19, [fp, #-28]
    stur w0, [fp, #-32]
    stur w1, [fp, #-36]
    adrp x19, _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$class@PAGE
    add x0, x19, _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$class@PAGEOFF
    sub x1, fp, #28
    bl _array_fill
    mov x2, x0
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-8]
    mov w0, #0
    mov w1, #0
    stur w0, [fp, #-12]
    stur w1, [fp, #-16]
    stur x2, [fp, #-24]
    ldur x0, [fp, #-24]
    ldur w1, [fp, #-12]
    ldur w2, [fp, #-16]
    ldur x3, [fp, #-8]
    bl _array_set_ref2
    adrp x19, Kstr.7@PAGE
    add x19, x19, Kstr.7@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-8]
    mov w0, #1
    mov w1, #1
    stur w0, [fp, #-12]
    stur w1, [fp, #-16]
    ldur x0, [fp, #-24]
    ldur w1, [fp, #-12]
    ldur w2, [fp, #-16]
    ldur x3, [fp, #-8]
    bl _array_set_ref2
    ldur x19, [fp, #-24]
    str x19, [fp, #24]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _issue06_Zoissue06_Zdissue15_had
_issue06_Zoissue06_Zdissue15_had:
.cfi_startproc
Lblk37:
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
    bl _issue06_Zoissue06_Zdissue15
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
.global _issue06_Zoissue06_ZdBaz_ZdBaz_Z4constructor
_issue06_Zoissue06_ZdBaz_ZdBaz_Z4constructor:
.cfi_startproc
Lblk0:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov x3, x0
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    stur w2, [fp, #-16]
    stur x3, [fp, #-24]
    ldur x0, [fp, #-24]
    bl _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    ldur x0, [fp, #-8]
    ldur w1, [fp, #-12]
    ldur w2, [fp, #-16]
    str w1, [x0, #16]
    str w2, [x0, #20]
    add x1, fp, #-56
    mov w2, #0
    adrp x19, Kstr.0@PAGE
    add x19, x19, Kstr.0@PAGEOFF
    ldr x3, [x19, #0]
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    stur w2, [fp, #-20]
    stur x3, [fp, #-28]
    ldur x0, [fp, #-16]
    ldur w1, [fp, #-20]
    ldur x2, [fp, #-28]
    bl _issue06_Zoissue06_ZdBzz_ZdBzz_Z4constructor
    ldur x0, [fp, #-8]
    stur x0, [fp, #-8]
    ldur x0, [fp, #-8]
    ldur x0, [fp, #-8]
    add x1, x0, #24
    sub x2, fp, #56
    bl _put_field_chunk
    adrp x19, Kstr.0@PAGE
    add x19, x19, Kstr.0@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-16]
    ldur x0, [fp, #-8]
    add x0, x0, #56
    ldur x1, [fp, #-16]
    bl _put_field
    mov x19, #0
    stur x19, [fp, #-16]
    ldur x0, [fp, #-8]
    add x0, x0, #64
    ldur x1, [fp, #-16]
    bl _put_field
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _issue06_Zoissue06_ZdBar_ZdBar_Z4constructor
_issue06_Zoissue06_ZdBar_ZdBar_Z4constructor:
.cfi_startproc
Lblk1:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    str w1, [x0, #16]
    str w2, [x0, #20]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue06_Zoissue06_ZdBzz_ZdBzz_Z4constructor
_issue06_Zoissue06_ZdBzz_ZdBzz_Z4constructor:
.cfi_startproc
Lblk2:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    str w1, [x0, #16]
    str x2, [x0, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue06_Zoissue06_ZdOption_Dki32_Dl_ZdunwarpOr
_issue06_Zoissue06_ZdOption_Dki32_Dl_ZdunwarpOr:
.cfi_startproc
Lblk3:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    b Lblk4
    nop
Lblk4:
    ldrh w2, [x0, #0]
    cmp w2, #0
    b.ne Lblk6
Lblk5:
    mov w2, w1
    b Lblk8
    nop
Lblk6:
    ldrh w3, [x0, #0]
    cmp w3, #1
    b.ne Lblk8
Lblk7:
    ldr w3, [x0, #4]
    mov w2, w3
    b Lblk8
    nop
Lblk8:
    str w2, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue06_Zoissue06_ZdOption_Dkstring_Dl_ZdunwarpOr
_issue06_Zoissue06_ZdOption_Dkstring_Dl_ZdunwarpOr:
.cfi_startproc
Lblk9:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    b Lblk10
    nop
Lblk10:
    cmp x0, #0
    b.ne Lblk12
Lblk11:
    mov x2, x1
    b Lblk14
    nop
Lblk12:
    cmp x0, #0
    b.ne Lblk13
Lblk13:
    mov x3, x0
    mov x2, x3
    b Lblk14
    nop
Lblk14:
    str x2, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz ""
Lkzs.1:
    .asciz "yalx/lang:lang"
Lkzs.2:
    .asciz "ok"
Lkzs.3:
    .asciz "bzz"
Lkzs.4:
    .asciz "baz"
Lkzs.5:
    .asciz "niko"
Lkzs.6:
    .asciz "option"
Lkzs.7:
    .asciz "hello"
Lkzs.8:
    .asciz "fun (issue06:issue06.Baz,i32,i32)->(void)"
Lkzs.9:
    .asciz "Baz$constructor"
Lkzs.10:
    .asciz "x"
Lkzs.11:
    .asciz "y"
Lkzs.12:
    .asciz "name"
Lkzs.13:
    .asciz "nickName"
Lkzs.14:
    .asciz "Baz"
Lkzs.15:
    .asciz "issue06:issue06.Baz"
Lkzs.16:
    .asciz "fun (issue06:issue06.Bar,i32,i32)->(void)"
Lkzs.17:
    .asciz "Bar$constructor"
Lkzs.18:
    .asciz "Bar"
Lkzs.19:
    .asciz "issue06:issue06.Bar"
Lkzs.20:
    .asciz "fun (issue06:issue06.Bzz,i32,string)->(void)"
Lkzs.21:
    .asciz "Bzz$constructor"
Lkzs.22:
    .asciz "id"
Lkzs.23:
    .asciz "Bzz"
Lkzs.24:
    .asciz "issue06:issue06.Bzz"
Lkzs.25:
    .asciz "A"
Lkzs.26:
    .asciz "B"
Lkzs.27:
    .asciz "C"
Lkzs.28:
    .asciz "D"
Lkzs.29:
    .asciz "E"
Lkzs.30:
    .asciz "$enum_code$"
Lkzs.31:
    .asciz "Foo"
Lkzs.32:
    .asciz "issue06:issue06.Foo"
Lkzs.33:
    .asciz "fun (issue06:issue06.Option<i32>,i32)->(i32)"
Lkzs.34:
    .asciz "unwarpOr"
Lkzs.35:
    .asciz "None"
Lkzs.36:
    .asciz "Some"
Lkzs.37:
    .asciz "Option<i32>"
Lkzs.38:
    .asciz "issue06:issue06.Option<i32>"
Lkzs.39:
    .asciz "fun (issue06:issue06.Option<string>,string)->(string)"
Lkzs.40:
    .asciz "Option<string>"
Lkzs.41:
    .asciz "issue06:issue06.Option<string>"
.section __DATA,__data
.p2align 4
; classes:
.global _issue06_Zoissue06_ZdBaz$class
_issue06_Zoissue06_ZdBaz$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 72 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.14 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.15 ; location
    .long 19 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 5 ; n_fields
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdBaz$fields ; fields
    .quad _issue06_Zoissue06_ZdBaz$ctor ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdBaz$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 3 ; refs_mark_len
    .space 4
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 48
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 56
    .quad _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$class ; type
    .long 64
_issue06_Zoissue06_ZdBaz$fields:
    ; Baz::x
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.10 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; Baz::y
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.11 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 20 ; offset_of_head
    .space 4 ; padding
    ; Baz::bzz
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.3 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdBzz$class ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
    ; Baz::name
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.12 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 56 ; offset_of_head
    .space 4 ; padding
    ; Baz::nickName
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.13 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$class ; type
    .long 64 ; offset_of_head
    .space 4 ; padding
_issue06_Zoissue06_ZdBaz$methods:
_issue06_Zoissue06_ZdBaz$ctor:
    ; Baz::Baz$constructor
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.9 ; name
    .long 15 ; name
    .space 4 ; padding
    .quad Lkzs.8 ; prototype_desc
    .long 41 ; prototype_desc
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdBaz_ZdBaz_Z4constructor ; entry
.global _issue06_Zoissue06_ZdBar$class
_issue06_Zoissue06_ZdBar$class:
    .quad 0 ; id
    .byte 1 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 24 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.18 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.19 ; location
    .long 19 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 2 ; n_fields
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdBar$fields ; fields
    .quad _issue06_Zoissue06_ZdBar$ctor ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdBar$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_issue06_Zoissue06_ZdBar$fields:
    ; Bar::x
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.10 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; Bar::y
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.11 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 20 ; offset_of_head
    .space 4 ; padding
_issue06_Zoissue06_ZdBar$methods:
_issue06_Zoissue06_ZdBar$ctor:
    ; Bar::Bar$constructor
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.17 ; name
    .long 15 ; name
    .space 4 ; padding
    .quad Lkzs.16 ; prototype_desc
    .long 41 ; prototype_desc
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdBar_ZdBar_Z4constructor ; entry
.global _issue06_Zoissue06_ZdBzz$class
_issue06_Zoissue06_ZdBzz$class:
    .quad 0 ; id
    .byte 1 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 32 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.23 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.24 ; location
    .long 19 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 2 ; n_fields
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdBzz$fields ; fields
    .quad _issue06_Zoissue06_ZdBzz$ctor ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdBzz$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 1 ; refs_mark_len
    .space 4
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 24
_issue06_Zoissue06_ZdBzz$fields:
    ; Bzz::id
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.22 ; name
    .long 2 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; Bzz::name
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.12 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
_issue06_Zoissue06_ZdBzz$methods:
_issue06_Zoissue06_ZdBzz$ctor:
    ; Bzz::Bzz$constructor
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.21 ; name
    .long 15 ; name
    .space 4 ; padding
    .quad Lkzs.20 ; prototype_desc
    .long 44 ; prototype_desc
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdBzz_ZdBzz_Z4constructor ; entry
.global _issue06_Zoissue06_ZdFoo$class
_issue06_Zoissue06_ZdFoo$class:
    .quad 0 ; id
    .byte 2 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 16 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.31 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.32 ; location
    .long 19 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 6 ; n_fields
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdFoo$fields ; fields
    .quad 0 ; ctor
    .long 0 ; n_methods
    .space 4 ; padding
    .quad 0 ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 1 ; refs_mark_len
    .space 4
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 8
_issue06_Zoissue06_ZdFoo$fields:
    ; Foo::A
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.25 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad 0 ; type
    .long 2 ; offset_of_head
    .space 4 ; padding
    ; Foo::B
    .long 16 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.26 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+456 ; type
    .long 8 ; offset_of_head
    .space 4 ; padding
    ; Foo::C
    .long 32 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.27 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 8 ; offset_of_head
    .space 4 ; padding
    ; Foo::D
    .long 48 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.28 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 8 ; offset_of_head
    .space 4 ; padding
    ; Foo::E
    .long 64 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.29 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad 0 ; type
    .long 2 ; offset_of_head
    .space 4 ; padding
    ; Foo::$enum_code$
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.30 ; name
    .long 11 ; name
    .space 4 ; padding
    .quad _builtin_classes+760 ; type
    .long 0 ; offset_of_head
    .space 4 ; padding
.global _issue06_Zoissue06_ZdOption_Dki32_Dl$class
_issue06_Zoissue06_ZdOption_Dki32_Dl$class:
    .quad 0 ; id
    .byte 2 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 8 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.37 ; name
    .long 11 ; name
    .space 4 ; padding
    .quad Lkzs.38 ; location
    .long 27 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 3 ; n_fields
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdOption_Dki32_Dl$fields ; fields
    .quad 0 ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdOption_Dki32_Dl$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_issue06_Zoissue06_ZdOption_Dki32_Dl$fields:
    ; Option<i32>::None
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.35 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad 0 ; type
    .long 2 ; offset_of_head
    .space 4 ; padding
    ; Option<i32>::Some
    .long 16 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.36 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 4 ; offset_of_head
    .space 4 ; padding
    ; Option<i32>::$enum_code$
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.30 ; name
    .long 11 ; name
    .space 4 ; padding
    .quad _builtin_classes+760 ; type
    .long 0 ; offset_of_head
    .space 4 ; padding
_issue06_Zoissue06_ZdOption_Dki32_Dl$methods:
    ; Option<i32>::unwarpOr
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.34 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.33 ; prototype_desc
    .long 44 ; prototype_desc
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdOption_Dki32_Dl_ZdunwarpOr ; entry
.global _issue06_Zoissue06_ZdOption_Dkstring_Dl$class
_issue06_Zoissue06_ZdOption_Dkstring_Dl$class:
    .quad 0 ; id
    .byte 2 ; constraint
    .byte 1 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 8 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.40 ; name
    .long 14 ; name
    .space 4 ; padding
    .quad Lkzs.41 ; location
    .long 30 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 2 ; n_fields
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdOption_Dkstring_Dl$fields ; fields
    .quad 0 ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdOption_Dkstring_Dl$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 1 ; refs_mark_len
    .space 4
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 0
_issue06_Zoissue06_ZdOption_Dkstring_Dl$fields:
    ; Option<string>::None
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.35 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad 0 ; type
    .long 0 ; offset_of_head
    .space 4 ; padding
    ; Option<string>::Some
    .long 16 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.36 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 0 ; offset_of_head
    .space 4 ; padding
_issue06_Zoissue06_ZdOption_Dkstring_Dl$methods:
    ; Option<string>::unwarpOr
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.34 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.39 ; prototype_desc
    .long 53 ; prototype_desc
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdOption_Dkstring_Dl_ZdunwarpOr ; entry
.section __DATA,__data
.p2align 4
.global _issue06_Zoissue06$global_slots
_issue06_Zoissue06$global_slots:
    .quad 0 ; size_in_bytes
    .quad 0 ; slots
    .quad 0 ; mark_size
; string constants:
; Yalx-String constants
.global _issue06_Zoissue06_Lksz
_issue06_Zoissue06_Lksz:
    .long 42
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
    .quad Lkzs.34
    .quad Lkzs.35
    .quad Lkzs.36
    .quad Lkzs.37
    .quad Lkzs.38
    .quad Lkzs.39
    .quad Lkzs.40
    .quad Lkzs.41
.global _issue06_Zoissue06_Kstr
_issue06_Zoissue06_Kstr:
    .long 42
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
Kstr.34:
    .quad 0
Kstr.35:
    .quad 0
Kstr.36:
    .quad 0
Kstr.37:
    .quad 0
Kstr.38:
    .quad 0
Kstr.39:
    .quad 0
Kstr.40:
    .quad 0
Kstr.41:
    .quad 0
