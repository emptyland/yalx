.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "tests/45-code-gen-enum-types/src/issue06" "foo.yalx"
.p2align 2

; functions
.global _issue06_Zoissue06_Zd_Z4init
_issue06_Zoissue06_Zd_Z4init:
.cfi_startproc
Lblk13:
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
.global _issue06_Zoissue06_Zdissue1
_issue06_Zoissue06_Zdissue1:
.cfi_startproc
Lblk14:
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
Lblk15:
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
Lblk16:
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
Lblk18:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #3
    stur w0, [fp, #-16]
    adrp x19, Kstr.1@PAGE
    add x19, x19, Kstr.1@PAGEOFF
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
.global _issue06_Zoissue06_Zdissue10
_issue06_Zoissue06_Zdissue10:
.cfi_startproc
Lblk20:
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
Lblk22:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-8]
    adrp x19, Kstr.3@PAGE
    add x19, x19, Kstr.3@PAGEOFF
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
Lblk24:
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
Lblk25:
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
.global _issue06_Zoissue06_ZdBar_ZdBar_Z4constructor
_issue06_Zoissue06_ZdBar_ZdBar_Z4constructor:
.cfi_startproc
Lblk0:
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
.global _issue06_Zoissue06_ZdOption_Dki32_Dl_ZdunwarpOr
_issue06_Zoissue06_ZdOption_Dki32_Dl_ZdunwarpOr:
.cfi_startproc
Lblk1:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    b Lblk2
    nop
Lblk2:
    ldrh w2, [x0, #0]
    cmp w2, #0
    b.ne Lblk4
Lblk3:
    mov w2, w1
    b Lblk6
    nop
Lblk4:
    ldrh w3, [x0, #0]
    cmp w3, #1
    b.ne Lblk6
Lblk5:
    ldr w3, [x0, #4]
    mov w2, w3
    b Lblk6
    nop
Lblk6:
    str w2, [fp, #28]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
.global _issue06_Zoissue06_ZdOption_Dkstring_Dl_ZdunwarpOr
_issue06_Zoissue06_ZdOption_Dkstring_Dl_ZdunwarpOr:
.cfi_startproc
Lblk7:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    b Lblk8
    nop
Lblk8:
    cmp x0, #0
    b.ne Lblk10
Lblk9:
    mov x2, x1
    b Lblk12
    nop
Lblk10:
    cmp x0, #0
    b.ne Lblk11
Lblk11:
    mov x3, x0
    mov x2, x3
    b Lblk12
    nop
Lblk12:
    str x2, [fp, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "yalx/lang:lang"
Lkzs.1:
    .asciz "ok"
Lkzs.2:
    .asciz "option"
Lkzs.3:
    .asciz ""
Lkzs.4:
    .asciz "fun (issue06:issue06.Bar,i32,i32)->(void)"
Lkzs.5:
    .asciz "Bar$constructor"
Lkzs.6:
    .asciz "x"
Lkzs.7:
    .asciz "y"
Lkzs.8:
    .asciz "Bar"
Lkzs.9:
    .asciz "issue06:issue06.Bar"
Lkzs.10:
    .asciz "A"
Lkzs.11:
    .asciz "B"
Lkzs.12:
    .asciz "C"
Lkzs.13:
    .asciz "D"
Lkzs.14:
    .asciz "E"
Lkzs.15:
    .asciz "$enum_code$"
Lkzs.16:
    .asciz "Foo"
Lkzs.17:
    .asciz "issue06:issue06.Foo"
Lkzs.18:
    .asciz "fun (issue06:issue06.Option<i32>,i32)->(i32)"
Lkzs.19:
    .asciz "unwarpOr"
Lkzs.20:
    .asciz "None"
Lkzs.21:
    .asciz "Some"
Lkzs.22:
    .asciz "Option<i32>"
Lkzs.23:
    .asciz "issue06:issue06.Option<i32>"
Lkzs.24:
    .asciz "fun (issue06:issue06.Option<string>,string)->(string)"
Lkzs.25:
    .asciz "Option<string>"
Lkzs.26:
    .asciz "issue06:issue06.Option<string>"
.section __DATA,__data
.p2align 4
; classes:
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
    .quad Lkzs.8 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.9 ; location
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
    .quad Lkzs.6 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; Bar::y
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.7 ; name
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
    .quad Lkzs.5 ; name
    .long 15 ; name
    .space 4 ; padding
    .quad Lkzs.4 ; prototype_desc
    .long 41 ; prototype_desc
    .space 4 ; padding
    .quad _issue06_Zoissue06_ZdBar_ZdBar_Z4constructor ; entry
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
    .quad Lkzs.16 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.17 ; location
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
    .quad Lkzs.10 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad 0 ; type
    .long 2 ; offset_of_head
    .space 4 ; padding
    ; Foo::B
    .long 16 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.11 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+456 ; type
    .long 8 ; offset_of_head
    .space 4 ; padding
    ; Foo::C
    .long 32 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.12 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 8 ; offset_of_head
    .space 4 ; padding
    ; Foo::D
    .long 48 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.13 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 8 ; offset_of_head
    .space 4 ; padding
    ; Foo::E
    .long 64 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.14 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad 0 ; type
    .long 2 ; offset_of_head
    .space 4 ; padding
    ; Foo::$enum_code$
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.15 ; name
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
    .quad Lkzs.22 ; name
    .long 11 ; name
    .space 4 ; padding
    .quad Lkzs.23 ; location
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
    .quad Lkzs.20 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad 0 ; type
    .long 2 ; offset_of_head
    .space 4 ; padding
    ; Option<i32>::Some
    .long 16 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.21 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad _builtin_classes+912 ; type
    .long 4 ; offset_of_head
    .space 4 ; padding
    ; Option<i32>::$enum_code$
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.15 ; name
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
    .quad Lkzs.19 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.18 ; prototype_desc
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
    .quad Lkzs.25 ; name
    .long 14 ; name
    .space 4 ; padding
    .quad Lkzs.26 ; location
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
    .quad Lkzs.20 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad 0 ; type
    .long 0 ; offset_of_head
    .space 4 ; padding
    ; Option<string>::Some
    .long 16 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.21 ; name
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
    .quad Lkzs.19 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.24 ; prototype_desc
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
    .long 27
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
.global _issue06_Zoissue06_Kstr
_issue06_Zoissue06_Kstr:
    .long 27
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
