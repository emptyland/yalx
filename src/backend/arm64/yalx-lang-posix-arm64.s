.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "libs/yalx/lang" "any.yalx"
.file 2 "libs/yalx/lang" "optional.yalx"
.file 3 "libs/yalx/lang" "throwing.yalx"
.file 4 "libs/yalx/lang" "number.yalx"
.file 5 "libs/yalx/lang" "string.yalx"
.p2align 2

; functions
.global _yalx_Zplang_Zolang_Zd_Z4init
_yalx_Zplang_Zolang_Zd_Z4init:
.cfi_startproc
Lblk58:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdmultiDimsArrayGetLength
_yalx_Zplang_Zolang_ZdmultiDimsArrayGetLength:
.cfi_startproc
Lblk59:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-56]
    stur w1, [fp, #-60]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_ZdmultiDimsArrayGetLength
    bl _associate_stub_returning_vals
    sub x0, fp, #56
    ldur w1, [fp, #-60]
    bl _yalx_Zplang_Zolang_ZdmultiDimsArrayGetLength_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdprintln
_yalx_Zplang_Zolang_Zdprintln:
.cfi_startproc
Lblk60:
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
.global _yalx_Zplang_Zolang_Zdu8ToString
_yalx_Zplang_Zolang_Zdu8ToString:
.cfi_startproc
Lblk61:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    sturb w0, [fp, #-52]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_Zdu8ToString
    bl _associate_stub_returning_vals
    ldurb w0, [fp, #-52]
    bl _yalx_Zplang_Zolang_Zdu8ToString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdu8Equals
_yalx_Zplang_Zolang_Zdu8Equals:
.cfi_startproc
Lblk62:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    cmp w0, w1
    mov w19, #1
    csel w2, w19, wzr, EQ
    strb w2, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdi8ToString
_yalx_Zplang_Zolang_Zdi8ToString:
.cfi_startproc
Lblk63:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    sturb w0, [fp, #-52]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_Zdi8ToString
    bl _associate_stub_returning_vals
    ldursb w0, [fp, #-52]
    bl _yalx_Zplang_Zolang_Zdi8ToString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdu16ToString
_yalx_Zplang_Zolang_Zdu16ToString:
.cfi_startproc
Lblk64:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur w0, [fp, #-52]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_Zdu16ToString
    bl _associate_stub_returning_vals
    ldurh w0, [fp, #-52]
    bl _yalx_Zplang_Zolang_Zdu16ToString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdi16ToString
_yalx_Zplang_Zolang_Zdi16ToString:
.cfi_startproc
Lblk65:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur w0, [fp, #-52]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_Zdi16ToString
    bl _associate_stub_returning_vals
    ldursw x0, [fp, #-52]
    bl _yalx_Zplang_Zolang_Zdi16ToString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdu32ToString
_yalx_Zplang_Zolang_Zdu32ToString:
.cfi_startproc
Lblk66:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur w0, [fp, #-52]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_Zdu32ToString
    bl _associate_stub_returning_vals
    ldur w0, [fp, #-52]
    bl _yalx_Zplang_Zolang_Zdu32ToString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdi32ToString
_yalx_Zplang_Zolang_Zdi32ToString:
.cfi_startproc
Lblk67:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur w0, [fp, #-52]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_Zdi32ToString
    bl _associate_stub_returning_vals
    ldur w0, [fp, #-52]
    bl _yalx_Zplang_Zolang_Zdi32ToString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdu64ToString
_yalx_Zplang_Zolang_Zdu64ToString:
.cfi_startproc
Lblk68:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-56]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_Zdu64ToString
    bl _associate_stub_returning_vals
    ldur x0, [fp, #-56]
    bl _yalx_Zplang_Zolang_Zdu64ToString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdi64ToString
_yalx_Zplang_Zolang_Zdi64ToString:
.cfi_startproc
Lblk69:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-56]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_Zdi64ToString
    bl _associate_stub_returning_vals
    ldur x0, [fp, #-56]
    bl _yalx_Zplang_Zolang_Zdi64ToString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdf32ToString
_yalx_Zplang_Zolang_Zdf32ToString:
.cfi_startproc
Lblk70:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur s0, [fp, #-52]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_Zdf32ToString
    bl _associate_stub_returning_vals
    ldur s0, [fp, #-52]
    bl _yalx_Zplang_Zolang_Zdf32ToString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdf64ToString
_yalx_Zplang_Zolang_Zdf64ToString:
.cfi_startproc
Lblk71:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur s0, [fp, #-56]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_Zdf64ToString
    bl _associate_stub_returning_vals
    ldur s0, [fp, #-56]
    bl _yalx_Zplang_Zolang_Zdf64ToString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdunwind
_yalx_Zplang_Zolang_Zdunwind:
.cfi_startproc
Lblk72:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_Zdunwind
    bl _associate_stub_returning_vals
    bl _yalx_Zplang_Zolang_Zdunwind_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdOptional_Dkyalx_Zplang_Zolang_ZdThrowable_Dl_ZdunwarpOr
_yalx_Zplang_Zolang_ZdOptional_Dkyalx_Zplang_Zolang_ZdThrowable_Dl_ZdunwarpOr:
.cfi_startproc
Lblk0:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    b Lblk1
    nop
Lblk1:
    cmp x0, #0
    b.ne Lblk3
Lblk2:
    mov x2, x1
    b Lblk5
    nop
Lblk3:
    cmp x0, #0
    b.ne Lblk4
Lblk4:
    mov x3, x0
    mov x2, x3
    b Lblk5
    nop
Lblk5:
    str x2, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdOptional_Dki32_Dl_ZdunwarpOr
_yalx_Zplang_Zolang_ZdOptional_Dki32_Dl_ZdunwarpOr:
.cfi_startproc
Lblk6:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    b Lblk7
    nop
Lblk7:
    ldrh w2, [x0, #0]
    cmp w2, #0
    b.ne Lblk9
Lblk8:
    mov w2, w1
    b Lblk11
    nop
Lblk9:
    ldrh w3, [x0, #0]
    cmp w3, #1
    b.ne Lblk11
Lblk10:
    ldr w3, [x0, #4]
    mov w2, w3
    b Lblk11
    nop
Lblk11:
    str w2, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl_ZdunwarpOr
_yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl_ZdunwarpOr:
.cfi_startproc
Lblk12:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    b Lblk13
    nop
Lblk13:
    cmp x0, #0
    b.ne Lblk15
Lblk14:
    mov x2, x1
    b Lblk17
    nop
Lblk15:
    cmp x0, #0
    b.ne Lblk16
Lblk16:
    mov x3, x0
    mov x2, x3
    b Lblk17
    nop
Lblk17:
    str x2, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdAny_Zdid
_yalx_Zplang_Zolang_ZdAny_Zdid:
.cfi_startproc
Lblk18:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-56]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_ZdAny_Zdid
    bl _associate_stub_returning_vals
    sub x0, fp, #56
    bl _yalx_Zplang_Zolang_ZdAny_Zdid_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdAny_ZdhashCode
_yalx_Zplang_Zolang_ZdAny_ZdhashCode:
.cfi_startproc
Lblk19:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-56]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_ZdAny_ZdhashCode
    bl _associate_stub_returning_vals
    sub x0, fp, #56
    bl _yalx_Zplang_Zolang_ZdAny_ZdhashCode_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdAny_ZdtoString
_yalx_Zplang_Zolang_ZdAny_ZdtoString:
.cfi_startproc
Lblk20:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-56]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_ZdAny_ZdtoString
    bl _associate_stub_returning_vals
    sub x0, fp, #56
    bl _yalx_Zplang_Zolang_ZdAny_ZdtoString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
_yalx_Zplang_Zolang_ZdAny_ZdisEmpty:
.cfi_startproc
Lblk21:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-56]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
    bl _associate_stub_returning_vals
    sub x0, fp, #56
    bl _yalx_Zplang_Zolang_ZdAny_ZdisEmpty_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdAny_Zdfinalize
_yalx_Zplang_Zolang_ZdAny_Zdfinalize:
.cfi_startproc
Lblk22:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
_yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor:
.cfi_startproc
Lblk23:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdString_ZdString_Z4constructor
_yalx_Zplang_Zolang_ZdString_ZdString_Z4constructor:
.cfi_startproc
Lblk24:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    stur w2, [fp, #-16]
    stur w3, [fp, #-20]
    stur x4, [fp, #-28]
    bl _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    ldur x0, [fp, #-8]
    ldur w1, [fp, #-12]
    ldur w2, [fp, #-16]
    ldur w3, [fp, #-20]
    ldur x4, [fp, #-28]
    str w1, [x0, #16]
    str w2, [x0, #20]
    str w3, [x0, #24]
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    stur w2, [fp, #-16]
    stur w3, [fp, #-20]
    stur x4, [fp, #-28]
    add x0, x0, #32
    mov x1, x4
    bl _put_field
    ldur x0, [fp, #-8]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdI32_ZdtoString
_yalx_Zplang_Zolang_ZdI32_ZdtoString:
.cfi_startproc
Lblk25:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-56]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_ZdI32_ZdtoString
    bl _associate_stub_returning_vals
    ldur x0, [fp, #-56]
    bl _yalx_Zplang_Zolang_ZdI32_ZdtoString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdI32_ZdI32_Z4constructor
_yalx_Zplang_Zolang_ZdI32_ZdI32_Z4constructor:
.cfi_startproc
Lblk26:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov x2, x0
    stur x0, [fp, #-8]
    stur x2, [fp, #-16]
    ldur x0, [fp, #-16]
    bl _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdNumber_Dki32_Dl_Z4constructor
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdU32_ZdtoString
_yalx_Zplang_Zolang_ZdU32_ZdtoString:
.cfi_startproc
Lblk27:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-56]
    sub x0, fp, #48
    add x1, fp, #16
    mov x2, #16
    adr x3, _yalx_Zplang_Zolang_ZdU32_ZdtoString
    bl _associate_stub_returning_vals
    ldur x0, [fp, #-56]
    bl _yalx_Zplang_Zolang_ZdU32_ZdtoString_stub
    sub x0, fp, #48
    bl _yalx_exit_returning_scope
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdU32_ZdU32_Z4constructor
_yalx_Zplang_Zolang_ZdU32_ZdU32_Z4constructor:
.cfi_startproc
Lblk28:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov x2, x0
    stur x0, [fp, #-8]
    stur x2, [fp, #-16]
    ldur x0, [fp, #-16]
    bl _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdNumber_Dku32_Dl_Z4constructor
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI8
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI8:
.cfi_startproc
Lblk29:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    and w0, w0, #255
    strb w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU8
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU8:
.cfi_startproc
Lblk30:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    and w0, w0, #255
    strb w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI16
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI16:
.cfi_startproc
Lblk31:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    and w0, w0, #65535
    str w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU16
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU16:
.cfi_startproc
Lblk32:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    and w0, w0, #65535
    str w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI32
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI32:
.cfi_startproc
Lblk33:
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
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU32
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU32:
.cfi_startproc
Lblk34:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    str w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI64
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI64:
.cfi_startproc
Lblk35:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    sxtw x0, x1
    str x0, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU64
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU64:
.cfi_startproc
Lblk36:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    str x0, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoF32
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoF32:
.cfi_startproc
Lblk37:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    brk #0x3c
    str s0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoF64
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoF64:
.cfi_startproc
Lblk38:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    brk #0x3c
    str s0, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdNumber_Dki32_Dl_Z4constructor
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdNumber_Dki32_Dl_Z4constructor:
.cfi_startproc
Lblk39:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    str w1, [x0, #16]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI8
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI8:
.cfi_startproc
Lblk40:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    and w0, w0, #255
    strb w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU8
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU8:
.cfi_startproc
Lblk41:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    and w0, w0, #255
    strb w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI16
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI16:
.cfi_startproc
Lblk42:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    and w0, w0, #65535
    str w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU16
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU16:
.cfi_startproc
Lblk43:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    and w0, w0, #65535
    str w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI32
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI32:
.cfi_startproc
Lblk44:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    str w0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU32
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU32:
.cfi_startproc
Lblk45:
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
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI64
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI64:
.cfi_startproc
Lblk46:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    str x0, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU64
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU64:
.cfi_startproc
Lblk47:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    mov w0, w1
    str x0, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoF32
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoF32:
.cfi_startproc
Lblk48:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    brk #0x3c
    str s0, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoF64
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoF64:
.cfi_startproc
Lblk49:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr w1, [x0, #16]
    brk #0x3c
    str s0, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdNumber_Dku32_Dl_Z4constructor
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdNumber_Dku32_Dl_Z4constructor:
.cfi_startproc
Lblk50:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    str w1, [x0, #16]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdThrowable_ZdtoString
_yalx_Zplang_Zolang_ZdThrowable_ZdtoString:
.cfi_startproc
Lblk51:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr x1, [x0, #16]
    str x1, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdThrowable_ZdprintBacktrace
_yalx_Zplang_Zolang_ZdThrowable_ZdprintBacktrace:
.cfi_startproc
Lblk52:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-8]
    sub x0, fp, #8
    bl _yalx_Zplang_Zolang_ZdThrowable_ZdprintBacktrace_stub
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdThrowable_ZdThrowable_Z4constructor
_yalx_Zplang_Zolang_ZdThrowable_ZdThrowable_Z4constructor:
.cfi_startproc
Lblk53:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov x3, x0
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    stur x2, [fp, #-24]
    stur x3, [fp, #-32]
    ldur x0, [fp, #-32]
    bl _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    ldur x0, [fp, #-8]
    ldur x1, [fp, #-16]
    ldur x2, [fp, #-24]
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    stur x2, [fp, #-24]
    add x0, x0, #16
    bl _put_field
    ldur x0, [fp, #-8]
    ldur x1, [fp, #-24]
    str x1, [x0, #24]
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    add sp, sp, #0
    bl _yalx_Zplang_Zolang_Zdunwind
    sub sp, sp, #0
    ldur x0, [fp, #-8]
    stur x0, [fp, #-8]
    stur x1, [fp, #-24]
    add x0, x0, #32
    ldur x1, [fp, #-40]
    bl _put_field
    ldur x0, [fp, #-8]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
_yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor:
.cfi_startproc
Lblk54:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov x3, x0
    stur x0, [fp, #-8]
    stur x3, [fp, #-16]
    ldur x0, [fp, #-16]
    bl _yalx_Zplang_Zolang_ZdThrowable_ZdThrowable_Z4constructor
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdBadCastingException_ZdBadCastingException_Z4constructor
_yalx_Zplang_Zolang_ZdBadCastingException_ZdBadCastingException_Z4constructor:
.cfi_startproc
Lblk55:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov x2, x0
    mov x19, #0
    stur x19, [fp, #-8]
    stur x0, [fp, #-16]
    stur x2, [fp, #-24]
    ldur x0, [fp, #-24]
    ldur x2, [fp, #-8]
    bl _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException_ZdArrayIndexOutOfBoundsException_Z4constructor
_yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException_ZdArrayIndexOutOfBoundsException_Z4constructor:
.cfi_startproc
Lblk56:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov x2, x0
    mov x19, #0
    stur x19, [fp, #-8]
    stur x0, [fp, #-16]
    stur x2, [fp, #-24]
    ldur x0, [fp, #-24]
    ldur x2, [fp, #-8]
    bl _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdBacktraceFrame_ZdBacktraceFrame_Z4constructor
_yalx_Zplang_Zolang_ZdBacktraceFrame_ZdBacktraceFrame_Z4constructor:
.cfi_startproc
Lblk57:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov x5, x0
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    stur x2, [fp, #-24]
    stur x3, [fp, #-32]
    stur w4, [fp, #-36]
    stur x5, [fp, #-44]
    ldur x0, [fp, #-44]
    bl _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    ldur x0, [fp, #-8]
    ldur x1, [fp, #-16]
    ldur x2, [fp, #-24]
    ldur x3, [fp, #-32]
    ldur w4, [fp, #-36]
    str x1, [x0, #16]
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    stur x2, [fp, #-24]
    stur x3, [fp, #-32]
    stur w4, [fp, #-36]
    add x0, x0, #24
    mov x1, x2
    bl _put_field
    ldur x0, [fp, #-8]
    ldur x1, [fp, #-32]
    ldur w2, [fp, #-36]
    stur x0, [fp, #-8]
    stur x1, [fp, #-24]
    stur w2, [fp, #-28]
    add x0, x0, #32
    bl _put_field
    ldur x0, [fp, #-8]
    ldur w1, [fp, #-28]
    str w1, [x0, #40]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.cfi_endproc
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "fun (yalx/lang:lang.Optional<yalx/lang:lang.Throwable>,yalx/lang:lang.Throwable)->(yalx/lang:lang.Throwable)"
Lkzs.1:
    .asciz "unwarpOr"
Lkzs.2:
    .asciz "None"
Lkzs.3:
    .asciz "Some"
Lkzs.4:
    .asciz "Optional<yalx/lang:lang.Throwable>"
Lkzs.5:
    .asciz "yalx/lang:lang.Optional<yalx/lang:lang.Throwable>"
Lkzs.6:
    .asciz "fun (yalx/lang:lang.Optional<i32>,i32)->(i32)"
Lkzs.7:
    .asciz "$enum_code$"
Lkzs.8:
    .asciz "Optional<i32>"
Lkzs.9:
    .asciz "yalx/lang:lang.Optional<i32>"
Lkzs.10:
    .asciz "fun (yalx/lang:lang.Optional<string>,string)->(string)"
Lkzs.11:
    .asciz "Optional<string>"
Lkzs.12:
    .asciz "yalx/lang:lang.Optional<string>"
Lkzs.13:
    .asciz "fun (yalx/lang:lang.Any)->(i32)"
Lkzs.14:
    .asciz "id"
Lkzs.15:
    .asciz "fun (yalx/lang:lang.Any)->(u32)"
Lkzs.16:
    .asciz "hashCode"
Lkzs.17:
    .asciz "fun (yalx/lang:lang.Any)->(string)"
Lkzs.18:
    .asciz "toString"
Lkzs.19:
    .asciz "fun (yalx/lang:lang.Any)->(u8)"
Lkzs.20:
    .asciz "isEmpty"
Lkzs.21:
    .asciz "fun (yalx/lang:lang.Any)->(void)"
Lkzs.22:
    .asciz "finalize"
Lkzs.23:
    .asciz "Any$constructor"
Lkzs.24:
    .asciz "Any"
Lkzs.25:
    .asciz "yalx/lang:lang.Any"
Lkzs.26:
    .asciz "fun (string)->(string)"
Lkzs.27:
    .asciz "fun (string)->(u32)"
Lkzs.28:
    .asciz "fun (string,u32,u32,u32,u8[])->(void)"
Lkzs.29:
    .asciz "String$constructor"
Lkzs.30:
    .asciz "size"
Lkzs.31:
    .asciz "capacity"
Lkzs.32:
    .asciz "cachedHashCode"
Lkzs.33:
    .asciz "buf"
Lkzs.34:
    .asciz "String"
Lkzs.35:
    .asciz "yalx/lang:lang.String"
Lkzs.36:
    .asciz "fun (yalx/lang:lang.I32)->(string)"
Lkzs.37:
    .asciz "fun (yalx/lang:lang.I32,i32)->(void)"
Lkzs.38:
    .asciz "I32$constructor"
Lkzs.39:
    .asciz "I32"
Lkzs.40:
    .asciz "yalx/lang:lang.I32"
Lkzs.41:
    .asciz "fun (yalx/lang:lang.U32)->(string)"
Lkzs.42:
    .asciz "fun (yalx/lang:lang.U32,u32)->(void)"
Lkzs.43:
    .asciz "U32$constructor"
Lkzs.44:
    .asciz "U32"
Lkzs.45:
    .asciz "yalx/lang:lang.U32"
Lkzs.46:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(i8)"
Lkzs.47:
    .asciz "toI8"
Lkzs.48:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(u8)"
Lkzs.49:
    .asciz "toU8"
Lkzs.50:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(i16)"
Lkzs.51:
    .asciz "toI16"
Lkzs.52:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(u16)"
Lkzs.53:
    .asciz "toU16"
Lkzs.54:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(i32)"
Lkzs.55:
    .asciz "toI32"
Lkzs.56:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(u32)"
Lkzs.57:
    .asciz "toU32"
Lkzs.58:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(i64)"
Lkzs.59:
    .asciz "toI64"
Lkzs.60:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(u64)"
Lkzs.61:
    .asciz "toU64"
Lkzs.62:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(f32)"
Lkzs.63:
    .asciz "toF32"
Lkzs.64:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(f64)"
Lkzs.65:
    .asciz "toF64"
Lkzs.66:
    .asciz "fun (yalx/lang:lang.Number<i32>,i32)->(void)"
Lkzs.67:
    .asciz "Number<i32>$constructor"
Lkzs.68:
    .asciz "value"
Lkzs.69:
    .asciz "Number<i32>"
Lkzs.70:
    .asciz "yalx/lang:lang.Number<i32>"
Lkzs.71:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(i8)"
Lkzs.72:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(u8)"
Lkzs.73:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(i16)"
Lkzs.74:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(u16)"
Lkzs.75:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(i32)"
Lkzs.76:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(u32)"
Lkzs.77:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(i64)"
Lkzs.78:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(u64)"
Lkzs.79:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(f32)"
Lkzs.80:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(f64)"
Lkzs.81:
    .asciz "fun (yalx/lang:lang.Number<u32>,u32)->(void)"
Lkzs.82:
    .asciz "Number<u32>$constructor"
Lkzs.83:
    .asciz "Number<u32>"
Lkzs.84:
    .asciz "yalx/lang:lang.Number<u32>"
Lkzs.85:
    .asciz "fun (yalx/lang:lang.Throwable)->(string)"
Lkzs.86:
    .asciz "fun (yalx/lang:lang.Throwable)->(void)"
Lkzs.87:
    .asciz "printBacktrace"
Lkzs.88:
    .asciz "fun (yalx/lang:lang.Throwable,string,yalx/lang:lang.Optional<yalx/lang:lang.Throwable>)->(void)"
Lkzs.89:
    .asciz "Throwable$constructor"
Lkzs.90:
    .asciz "message"
Lkzs.91:
    .asciz "clause"
Lkzs.92:
    .asciz "backtrace"
Lkzs.93:
    .asciz "Throwable"
Lkzs.94:
    .asciz "yalx/lang:lang.Throwable"
Lkzs.95:
    .asciz "fun (yalx/lang:lang.Exception,string,yalx/lang:lang.Optional<yalx/lang:lang.Throwable>)->(void)"
Lkzs.96:
    .asciz "Exception$constructor"
Lkzs.97:
    .asciz "Exception"
Lkzs.98:
    .asciz "yalx/lang:lang.Exception"
Lkzs.99:
    .asciz "fun (yalx/lang:lang.BadCastingException,string)->(void)"
Lkzs.100:
    .asciz "BadCastingException$constructor"
Lkzs.101:
    .asciz "BadCastingException"
Lkzs.102:
    .asciz "yalx/lang:lang.BadCastingException"
Lkzs.103:
    .asciz "fun (yalx/lang:lang.ArrayIndexOutOfBoundsException,string)->(void)"
Lkzs.104:
    .asciz "ArrayIndexOutOfBoundsException$constructor"
Lkzs.105:
    .asciz "ArrayIndexOutOfBoundsException"
Lkzs.106:
    .asciz "yalx/lang:lang.ArrayIndexOutOfBoundsException"
Lkzs.107:
    .asciz "fun (yalx/lang:lang.BacktraceFrame,u64,string,string,u32)->(void)"
Lkzs.108:
    .asciz "BacktraceFrame$constructor"
Lkzs.109:
    .asciz "address"
Lkzs.110:
    .asciz "function"
Lkzs.111:
    .asciz "file"
Lkzs.112:
    .asciz "line"
Lkzs.113:
    .asciz "BacktraceFrame"
Lkzs.114:
    .asciz "yalx/lang:lang.BacktraceFrame"
.section __DATA,__data
.p2align 4
; classes:
.global _yalx_Zplang_Zolang_ZdOptional_Dkyalx_Zplang_Zolang_ZdThrowable_Dl$class
_yalx_Zplang_Zolang_ZdOptional_Dkyalx_Zplang_Zolang_ZdThrowable_Dl$class:
    .quad 0 ; id
    .byte 2 ; constraint
    .byte 1 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 8 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.4 ; name
    .long 34 ; name
    .space 4 ; padding
    .quad Lkzs.5 ; location
    .long 49 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 2 ; n_fields
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdOptional_Dkyalx_Zplang_Zolang_ZdThrowable_Dl$fields ; fields
    .quad 0 ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdOptional_Dkyalx_Zplang_Zolang_ZdThrowable_Dl$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 1 ; refs_mark_len
    .space 4
    .quad _yalx_Zplang_Zolang_ZdThrowable$class ; type
    .long 0
_yalx_Zplang_Zolang_ZdOptional_Dkyalx_Zplang_Zolang_ZdThrowable_Dl$fields:
    ; Optional<yalx/lang:lang.Throwable>::None
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.2 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad 0 ; type
    .long 0 ; offset_of_head
    .space 4 ; padding
    ; Optional<yalx/lang:lang.Throwable>::Some
    .long 16 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.3 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdThrowable$class ; type
    .long 0 ; offset_of_head
    .space 4 ; padding
_yalx_Zplang_Zolang_ZdOptional_Dkyalx_Zplang_Zolang_ZdThrowable_Dl$methods:
    ; Optional<yalx/lang:lang.Throwable>::unwarpOr
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.1 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.0 ; prototype_desc
    .long 108 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdOptional_Dkyalx_Zplang_Zolang_ZdThrowable_Dl_ZdunwarpOr ; entry
.global _yalx_Zplang_Zolang_ZdOptional_Dki32_Dl$class
_yalx_Zplang_Zolang_ZdOptional_Dki32_Dl$class:
    .quad 0 ; id
    .byte 2 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 8 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.8 ; name
    .long 13 ; name
    .space 4 ; padding
    .quad Lkzs.9 ; location
    .long 28 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 3 ; n_fields
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdOptional_Dki32_Dl$fields ; fields
    .quad 0 ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdOptional_Dki32_Dl$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_yalx_Zplang_Zolang_ZdOptional_Dki32_Dl$fields:
    ; Optional<i32>::None
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.2 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad 0 ; type
    .long 2 ; offset_of_head
    .space 4 ; padding
    ; Optional<i32>::Some
    .long 16 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.3 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 4 ; offset_of_head
    .space 4 ; padding
    ; Optional<i32>::$enum_code$
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.7 ; name
    .long 11 ; name
    .space 4 ; padding
    .quad _builtin_classes+760 ; type
    .long 0 ; offset_of_head
    .space 4 ; padding
_yalx_Zplang_Zolang_ZdOptional_Dki32_Dl$methods:
    ; Optional<i32>::unwarpOr
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.1 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.6 ; prototype_desc
    .long 45 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdOptional_Dki32_Dl_ZdunwarpOr ; entry
.global _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$class
_yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$class:
    .quad 0 ; id
    .byte 2 ; constraint
    .byte 1 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 8 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.11 ; name
    .long 16 ; name
    .space 4 ; padding
    .quad Lkzs.12 ; location
    .long 31 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 2 ; n_fields
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$fields ; fields
    .quad 0 ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 1 ; refs_mark_len
    .space 4
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 0
_yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$fields:
    ; Optional<string>::None
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.2 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad 0 ; type
    .long 0 ; offset_of_head
    .space 4 ; padding
    ; Optional<string>::Some
    .long 16 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.3 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 0 ; offset_of_head
    .space 4 ; padding
_yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl$methods:
    ; Optional<string>::unwarpOr
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.1 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.10 ; prototype_desc
    .long 54 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdOptional_Dkstring_Dl_ZdunwarpOr ; entry
.global _yalx_Zplang_Zolang_ZdAny$class
_yalx_Zplang_Zolang_ZdAny$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 16 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.24 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.25 ; location
    .long 18 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 0 ; n_fields
    .space 4 ; padding
    .quad 0 ; fields
    .quad _yalx_Zplang_Zolang_ZdAny$ctor ; ctor
    .long 6 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$methods ; methods
    .long 5 ; n_vtab
    .long 0 ; n_itab
    .quad _yalx_Zplang_Zolang_ZdAny$vtab ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_yalx_Zplang_Zolang_ZdAny$methods:
    ; Any::id
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.14 ; name
    .long 2 ; name
    .space 4 ; padding
    .quad Lkzs.13 ; prototype_desc
    .long 31 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny_Zdid ; entry
    ; Any::hashCode
    .long 1 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.16 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.15 ; prototype_desc
    .long 31 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny_ZdhashCode ; entry
    ; Any::toString
    .long 2 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.18 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.17 ; prototype_desc
    .long 34 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny_ZdtoString ; entry
    ; Any::isEmpty
    .long 3 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.20 ; name
    .long 7 ; name
    .space 4 ; padding
    .quad Lkzs.19 ; prototype_desc
    .long 30 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny_ZdisEmpty ; entry
    ; Any::finalize
    .long 4 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.22 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.21 ; prototype_desc
    .long 32 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny_Zdfinalize ; entry
_yalx_Zplang_Zolang_ZdAny$ctor:
    ; Any::Any$constructor
    .long 5 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.23 ; name
    .long 15 ; name
    .space 4 ; padding
    .quad Lkzs.21 ; prototype_desc
    .long 32 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor ; entry
_yalx_Zplang_Zolang_ZdAny$vtab:
    .quad _yalx_Zplang_Zolang_ZdAny_Zdfinalize
    .quad _yalx_Zplang_Zolang_ZdAny_ZdhashCode
    .quad _yalx_Zplang_Zolang_ZdAny_Zdid
    .quad _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
    .quad _yalx_Zplang_Zolang_ZdAny_ZdtoString
.global _yalx_Zplang_Zolang_ZdString$class
_yalx_Zplang_Zolang_ZdString$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 40 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.34 ; name
    .long 6 ; name
    .space 4 ; padding
    .quad Lkzs.35 ; location
    .long 21 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 4 ; n_fields
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$fields ; fields
    .quad _yalx_Zplang_Zolang_ZdString$ctor ; ctor
    .long 3 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 1 ; refs_mark_len
    .space 4
    .quad _builtin_classes+3648 ; type
    .long 32
_yalx_Zplang_Zolang_ZdString$fields:
    ; String::size
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.30 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad _builtin_classes+1064 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; String::capacity
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.31 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad _builtin_classes+1064 ; type
    .long 20 ; offset_of_head
    .space 4 ; padding
    ; String::cachedHashCode
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.32 ; name
    .long 14 ; name
    .space 4 ; padding
    .quad _builtin_classes+1064 ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
    ; String::buf
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.33 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad _builtin_classes+3648 ; type
    .long 32 ; offset_of_head
    .space 4 ; padding
_yalx_Zplang_Zolang_ZdString$methods:
    ; String::toString
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.18 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.26 ; prototype_desc
    .long 22 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString_ZdtoString ; entry
    ; String::hashCode
    .long 1 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.16 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.27 ; prototype_desc
    .long 19 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString_ZdhashCode ; entry
_yalx_Zplang_Zolang_ZdString$ctor:
    ; String::String$constructor
    .long 2 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.29 ; name
    .long 18 ; name
    .space 4 ; padding
    .quad Lkzs.28 ; prototype_desc
    .long 37 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString_ZdString_Z4constructor ; entry
.global _yalx_Zplang_Zolang_ZdI32$class
_yalx_Zplang_Zolang_ZdI32$class:
    .quad 0 ; id
    .byte 1 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 24 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$class ; super
    .quad Lkzs.39 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.40 ; location
    .long 18 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 0 ; n_fields
    .space 4 ; padding
    .quad 0 ; fields
    .quad _yalx_Zplang_Zolang_ZdI32$ctor ; ctor
    .long 2 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdI32$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_yalx_Zplang_Zolang_ZdI32$methods:
    ; I32::toString
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.18 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.36 ; prototype_desc
    .long 34 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdI32_ZdtoString ; entry
_yalx_Zplang_Zolang_ZdI32$ctor:
    ; I32::I32$constructor
    .long 1 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.38 ; name
    .long 15 ; name
    .space 4 ; padding
    .quad Lkzs.37 ; prototype_desc
    .long 36 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdI32_ZdI32_Z4constructor ; entry
.global _yalx_Zplang_Zolang_ZdU32$class
_yalx_Zplang_Zolang_ZdU32$class:
    .quad 0 ; id
    .byte 1 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 24 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$class ; super
    .quad Lkzs.44 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.45 ; location
    .long 18 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 0 ; n_fields
    .space 4 ; padding
    .quad 0 ; fields
    .quad _yalx_Zplang_Zolang_ZdU32$ctor ; ctor
    .long 2 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdU32$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_yalx_Zplang_Zolang_ZdU32$methods:
    ; U32::toString
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.18 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.41 ; prototype_desc
    .long 34 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdU32_ZdtoString ; entry
_yalx_Zplang_Zolang_ZdU32$ctor:
    ; U32::U32$constructor
    .long 1 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.43 ; name
    .long 15 ; name
    .space 4 ; padding
    .quad Lkzs.42 ; prototype_desc
    .long 36 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdU32_ZdU32_Z4constructor ; entry
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$class
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$class:
    .quad 0 ; id
    .byte 1 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 24 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.69 ; name
    .long 11 ; name
    .space 4 ; padding
    .quad Lkzs.70 ; location
    .long 26 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 1 ; n_fields
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$fields ; fields
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$ctor ; ctor
    .long 11 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$fields:
    ; Number<i32>::value
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.68 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$methods:
    ; Number<i32>::toI8
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.47 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad Lkzs.46 ; prototype_desc
    .long 38 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI8 ; entry
    ; Number<i32>::toU8
    .long 1 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.49 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad Lkzs.48 ; prototype_desc
    .long 38 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU8 ; entry
    ; Number<i32>::toI16
    .long 2 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.51 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.50 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI16 ; entry
    ; Number<i32>::toU16
    .long 3 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.53 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.52 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU16 ; entry
    ; Number<i32>::toI32
    .long 4 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.55 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.54 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI32 ; entry
    ; Number<i32>::toU32
    .long 5 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.57 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.56 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU32 ; entry
    ; Number<i32>::toI64
    .long 6 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.59 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.58 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI64 ; entry
    ; Number<i32>::toU64
    .long 7 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.61 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.60 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU64 ; entry
    ; Number<i32>::toF32
    .long 8 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.63 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.62 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoF32 ; entry
    ; Number<i32>::toF64
    .long 9 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.65 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.64 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoF64 ; entry
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$ctor:
    ; Number<i32>::Number<i32>$constructor
    .long 10 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.67 ; name
    .long 23 ; name
    .space 4 ; padding
    .quad Lkzs.66 ; prototype_desc
    .long 44 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdNumber_Dki32_Dl_Z4constructor ; entry
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$class
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$class:
    .quad 0 ; id
    .byte 1 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 24 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.83 ; name
    .long 11 ; name
    .space 4 ; padding
    .quad Lkzs.84 ; location
    .long 26 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 1 ; n_fields
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$fields ; fields
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$ctor ; ctor
    .long 11 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$fields:
    ; Number<u32>::value
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.68 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad _builtin_classes+1064 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$methods:
    ; Number<u32>::toI8
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.47 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad Lkzs.71 ; prototype_desc
    .long 38 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI8 ; entry
    ; Number<u32>::toU8
    .long 1 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.49 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad Lkzs.72 ; prototype_desc
    .long 38 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU8 ; entry
    ; Number<u32>::toI16
    .long 2 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.51 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.73 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI16 ; entry
    ; Number<u32>::toU16
    .long 3 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.53 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.74 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU16 ; entry
    ; Number<u32>::toI32
    .long 4 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.55 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.75 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI32 ; entry
    ; Number<u32>::toU32
    .long 5 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.57 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.76 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU32 ; entry
    ; Number<u32>::toI64
    .long 6 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.59 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.77 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI64 ; entry
    ; Number<u32>::toU64
    .long 7 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.61 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.78 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU64 ; entry
    ; Number<u32>::toF32
    .long 8 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.63 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.79 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoF32 ; entry
    ; Number<u32>::toF64
    .long 9 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.65 ; name
    .long 5 ; name
    .space 4 ; padding
    .quad Lkzs.80 ; prototype_desc
    .long 39 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoF64 ; entry
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$ctor:
    ; Number<u32>::Number<u32>$constructor
    .long 10 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.82 ; name
    .long 23 ; name
    .space 4 ; padding
    .quad Lkzs.81 ; prototype_desc
    .long 44 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdNumber_Dku32_Dl_Z4constructor ; entry
.global _yalx_Zplang_Zolang_ZdThrowable$class
_yalx_Zplang_Zolang_ZdThrowable$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 40 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.93 ; name
    .long 9 ; name
    .space 4 ; padding
    .quad Lkzs.94 ; location
    .long 24 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 3 ; n_fields
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdThrowable$fields ; fields
    .quad _yalx_Zplang_Zolang_ZdThrowable$ctor ; ctor
    .long 3 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdThrowable$methods ; methods
    .long 5 ; n_vtab
    .long 0 ; n_itab
    .quad _yalx_Zplang_Zolang_ZdThrowable$vtab ; vtab
    .quad 0 ; itab
    .long 3 ; refs_mark_len
    .space 4
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 16
    .quad _yalx_Zplang_Zolang_ZdOptional_Dkyalx_Zplang_Zolang_ZdThrowable_Dl$class ; type
    .long 24
    .quad _builtin_classes+3648 ; type
    .long 32
_yalx_Zplang_Zolang_ZdThrowable$fields:
    ; Throwable::message
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.90 ; name
    .long 7 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; Throwable::clause
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.91 ; name
    .long 6 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdOptional_Dkyalx_Zplang_Zolang_ZdThrowable_Dl$class ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
    ; Throwable::backtrace
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.92 ; name
    .long 9 ; name
    .space 4 ; padding
    .quad _builtin_classes+3648 ; type
    .long 32 ; offset_of_head
    .space 4 ; padding
_yalx_Zplang_Zolang_ZdThrowable$methods:
    ; Throwable::toString
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.18 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.85 ; prototype_desc
    .long 40 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdThrowable_ZdtoString ; entry
    ; Throwable::printBacktrace
    .long 1 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.87 ; name
    .long 14 ; name
    .space 4 ; padding
    .quad Lkzs.86 ; prototype_desc
    .long 38 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdThrowable_ZdprintBacktrace ; entry
_yalx_Zplang_Zolang_ZdThrowable$ctor:
    ; Throwable::Throwable$constructor
    .long 2 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.89 ; name
    .long 21 ; name
    .space 4 ; padding
    .quad Lkzs.88 ; prototype_desc
    .long 95 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdThrowable_ZdThrowable_Z4constructor ; entry
_yalx_Zplang_Zolang_ZdThrowable$vtab:
    .quad _yalx_Zplang_Zolang_ZdAny_Zdfinalize
    .quad _yalx_Zplang_Zolang_ZdAny_ZdhashCode
    .quad _yalx_Zplang_Zolang_ZdAny_Zdid
    .quad _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
    .quad _yalx_Zplang_Zolang_ZdThrowable_ZdtoString
.global _yalx_Zplang_Zolang_ZdException$class
_yalx_Zplang_Zolang_ZdException$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 40 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdThrowable$class ; super
    .quad Lkzs.97 ; name
    .long 9 ; name
    .space 4 ; padding
    .quad Lkzs.98 ; location
    .long 24 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 0 ; n_fields
    .space 4 ; padding
    .quad 0 ; fields
    .quad _yalx_Zplang_Zolang_ZdException$ctor ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdException$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_yalx_Zplang_Zolang_ZdException$methods:
_yalx_Zplang_Zolang_ZdException$ctor:
    ; Exception::Exception$constructor
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.96 ; name
    .long 21 ; name
    .space 4 ; padding
    .quad Lkzs.95 ; prototype_desc
    .long 95 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor ; entry
.global _yalx_Zplang_Zolang_ZdBadCastingException$class
_yalx_Zplang_Zolang_ZdBadCastingException$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 40 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdException$class ; super
    .quad Lkzs.101 ; name
    .long 19 ; name
    .space 4 ; padding
    .quad Lkzs.102 ; location
    .long 34 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 0 ; n_fields
    .space 4 ; padding
    .quad 0 ; fields
    .quad _yalx_Zplang_Zolang_ZdBadCastingException$ctor ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdBadCastingException$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_yalx_Zplang_Zolang_ZdBadCastingException$methods:
_yalx_Zplang_Zolang_ZdBadCastingException$ctor:
    ; BadCastingException::BadCastingException$constructor
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.100 ; name
    .long 31 ; name
    .space 4 ; padding
    .quad Lkzs.99 ; prototype_desc
    .long 55 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdBadCastingException_ZdBadCastingException_Z4constructor ; entry
.global _yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException$class
_yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 40 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdException$class ; super
    .quad Lkzs.105 ; name
    .long 30 ; name
    .space 4 ; padding
    .quad Lkzs.106 ; location
    .long 45 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 0 ; n_fields
    .space 4 ; padding
    .quad 0 ; fields
    .quad _yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException$ctor ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException$methods:
_yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException$ctor:
    ; ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException$constructor
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.104 ; name
    .long 42 ; name
    .space 4 ; padding
    .quad Lkzs.103 ; prototype_desc
    .long 66 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException_ZdArrayIndexOutOfBoundsException_Z4constructor ; entry
.global _yalx_Zplang_Zolang_ZdBacktraceFrame$class
_yalx_Zplang_Zolang_ZdBacktraceFrame$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 48 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.113 ; name
    .long 14 ; name
    .space 4 ; padding
    .quad Lkzs.114 ; location
    .long 29 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 4 ; n_fields
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdBacktraceFrame$fields ; fields
    .quad _yalx_Zplang_Zolang_ZdBacktraceFrame$ctor ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdBacktraceFrame$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 2 ; refs_mark_len
    .space 4
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 24
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 32
_yalx_Zplang_Zolang_ZdBacktraceFrame$fields:
    ; BacktraceFrame::address
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.109 ; name
    .long 7 ; name
    .space 4 ; padding
    .quad _builtin_classes+1064 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; BacktraceFrame::function
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.110 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
    ; BacktraceFrame::file
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.111 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 32 ; offset_of_head
    .space 4 ; padding
    ; BacktraceFrame::line
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.112 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad _builtin_classes+1064 ; type
    .long 40 ; offset_of_head
    .space 4 ; padding
_yalx_Zplang_Zolang_ZdBacktraceFrame$methods:
_yalx_Zplang_Zolang_ZdBacktraceFrame$ctor:
    ; BacktraceFrame::BacktraceFrame$constructor
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.108 ; name
    .long 26 ; name
    .space 4 ; padding
    .quad Lkzs.107 ; prototype_desc
    .long 65 ; prototype_desc
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdBacktraceFrame_ZdBacktraceFrame_Z4constructor ; entry
.section __DATA,__data
.p2align 4
.global _yalx_Zplang_Zolang$global_slots
_yalx_Zplang_Zolang$global_slots:
    .quad 0 ; size_in_bytes
    .quad 0 ; slots
    .quad 0 ; mark_size
; string constants:
; Yalx-String constants
.global _yalx_Zplang_Zolang_Lksz
_yalx_Zplang_Zolang_Lksz:
    .long 115
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
    .quad Lkzs.42
    .quad Lkzs.43
    .quad Lkzs.44
    .quad Lkzs.45
    .quad Lkzs.46
    .quad Lkzs.47
    .quad Lkzs.48
    .quad Lkzs.49
    .quad Lkzs.50
    .quad Lkzs.51
    .quad Lkzs.52
    .quad Lkzs.53
    .quad Lkzs.54
    .quad Lkzs.55
    .quad Lkzs.56
    .quad Lkzs.57
    .quad Lkzs.58
    .quad Lkzs.59
    .quad Lkzs.60
    .quad Lkzs.61
    .quad Lkzs.62
    .quad Lkzs.63
    .quad Lkzs.64
    .quad Lkzs.65
    .quad Lkzs.66
    .quad Lkzs.67
    .quad Lkzs.68
    .quad Lkzs.69
    .quad Lkzs.70
    .quad Lkzs.71
    .quad Lkzs.72
    .quad Lkzs.73
    .quad Lkzs.74
    .quad Lkzs.75
    .quad Lkzs.76
    .quad Lkzs.77
    .quad Lkzs.78
    .quad Lkzs.79
    .quad Lkzs.80
    .quad Lkzs.81
    .quad Lkzs.82
    .quad Lkzs.83
    .quad Lkzs.84
    .quad Lkzs.85
    .quad Lkzs.86
    .quad Lkzs.87
    .quad Lkzs.88
    .quad Lkzs.89
    .quad Lkzs.90
    .quad Lkzs.91
    .quad Lkzs.92
    .quad Lkzs.93
    .quad Lkzs.94
    .quad Lkzs.95
    .quad Lkzs.96
    .quad Lkzs.97
    .quad Lkzs.98
    .quad Lkzs.99
    .quad Lkzs.100
    .quad Lkzs.101
    .quad Lkzs.102
    .quad Lkzs.103
    .quad Lkzs.104
    .quad Lkzs.105
    .quad Lkzs.106
    .quad Lkzs.107
    .quad Lkzs.108
    .quad Lkzs.109
    .quad Lkzs.110
    .quad Lkzs.111
    .quad Lkzs.112
    .quad Lkzs.113
    .quad Lkzs.114
.global _yalx_Zplang_Zolang_Kstr
_yalx_Zplang_Zolang_Kstr:
    .long 115
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
Kstr.42:
    .quad 0
Kstr.43:
    .quad 0
Kstr.44:
    .quad 0
Kstr.45:
    .quad 0
Kstr.46:
    .quad 0
Kstr.47:
    .quad 0
Kstr.48:
    .quad 0
Kstr.49:
    .quad 0
Kstr.50:
    .quad 0
Kstr.51:
    .quad 0
Kstr.52:
    .quad 0
Kstr.53:
    .quad 0
Kstr.54:
    .quad 0
Kstr.55:
    .quad 0
Kstr.56:
    .quad 0
Kstr.57:
    .quad 0
Kstr.58:
    .quad 0
Kstr.59:
    .quad 0
Kstr.60:
    .quad 0
Kstr.61:
    .quad 0
Kstr.62:
    .quad 0
Kstr.63:
    .quad 0
Kstr.64:
    .quad 0
Kstr.65:
    .quad 0
Kstr.66:
    .quad 0
Kstr.67:
    .quad 0
Kstr.68:
    .quad 0
Kstr.69:
    .quad 0
Kstr.70:
    .quad 0
Kstr.71:
    .quad 0
Kstr.72:
    .quad 0
Kstr.73:
    .quad 0
Kstr.74:
    .quad 0
Kstr.75:
    .quad 0
Kstr.76:
    .quad 0
Kstr.77:
    .quad 0
Kstr.78:
    .quad 0
Kstr.79:
    .quad 0
Kstr.80:
    .quad 0
Kstr.81:
    .quad 0
Kstr.82:
    .quad 0
Kstr.83:
    .quad 0
Kstr.84:
    .quad 0
Kstr.85:
    .quad 0
Kstr.86:
    .quad 0
Kstr.87:
    .quad 0
Kstr.88:
    .quad 0
Kstr.89:
    .quad 0
Kstr.90:
    .quad 0
Kstr.91:
    .quad 0
Kstr.92:
    .quad 0
Kstr.93:
    .quad 0
Kstr.94:
    .quad 0
Kstr.95:
    .quad 0
Kstr.96:
    .quad 0
Kstr.97:
    .quad 0
Kstr.98:
    .quad 0
Kstr.99:
    .quad 0
Kstr.100:
    .quad 0
Kstr.101:
    .quad 0
Kstr.102:
    .quad 0
Kstr.103:
    .quad 0
Kstr.104:
    .quad 0
Kstr.105:
    .quad 0
Kstr.106:
    .quad 0
Kstr.107:
    .quad 0
Kstr.108:
    .quad 0
Kstr.109:
    .quad 0
Kstr.110:
    .quad 0
Kstr.111:
    .quad 0
Kstr.112:
    .quad 0
Kstr.113:
    .quad 0
Kstr.114:
    .quad 0
