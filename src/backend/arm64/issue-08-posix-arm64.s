.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "tests/47-code-gen-string-template/src/issue08" "foo.yalx"
.p2align 2

; functions
.global _issue08_Zoissue08_Zd_Z4init
_issue08_Zoissue08_Zd_Z4init:
.cfi_startproc
Lblk6:
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
.global _issue08_Zoissue08_Zdissue1
_issue08_Zoissue08_Zdissue1:
.cfi_startproc
Lblk7:
    sub sp, sp, #176
    stp fp, lr, [sp, #160]
    add fp, sp, #160
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur s0, [fp, #-4]
    stur s1, [fp, #-8]
    add sp, sp, #128
    bl _yalx_Zplang_Zolang_Zdf32ToString
    sub sp, sp, #128
    ldur s0, [fp, #-4]
    ldur s1, [fp, #-8]
    stur s0, [fp, #-4]
    stur s1, [fp, #-8]
    ldur s0, [fp, #-8]
    add sp, sp, #112
    bl _yalx_Zplang_Zolang_Zdf32ToString
    sub sp, sp, #112
    ldur s0, [fp, #-4]
    ldur s1, [fp, #-8]
    adrp x19, Kstr.4@PAGE
    add x19, x19, Kstr.4@PAGEOFF
    ldr x0, [x19, #0]
    adrp x19, Kstr.5@PAGE
    add x19, x19, Kstr.5@PAGEOFF
    ldr x1, [x19, #0]
    ldur x19, [fp, #-40]
    stur x19, [fp, #-48]
    stur x1, [fp, #-56]
    ldur x19, [fp, #-24]
    stur x19, [fp, #-64]
    stur x0, [fp, #-72]
    stur x0, [fp, #-8]
    stur x1, [fp, #-80]
    stur s0, [fp, #-84]
    stur s1, [fp, #-88]
    sub x0, fp, #72
    mov x1, #4
    bl _concat
    mov x2, x0
    ldur s0, [fp, #-84]
    ldur s1, [fp, #-88]
    stur x2, [fp, #-8]
    stur s0, [fp, #-12]
    stur s1, [fp, #-16]
    ldur s0, [fp, #-16]
    add sp, sp, #48
    bl _yalx_Zplang_Zolang_Zdf32ToString
    sub sp, sp, #48
    ldur x0, [fp, #-8]
    ldur s0, [fp, #-12]
    stur x0, [fp, #-8]
    add sp, sp, #32
    bl _yalx_Zplang_Zolang_Zdf32ToString
    sub sp, sp, #32
    ldur x0, [fp, #-8]
    adrp x19, Kstr.6@PAGE
    add x19, x19, Kstr.6@PAGEOFF
    ldr x1, [x19, #0]
    adrp x19, Kstr.7@PAGE
    add x19, x19, Kstr.7@PAGEOFF
    ldr x2, [x19, #0]
    ldur x19, [fp, #-120]
    stur x19, [fp, #-128]
    stur x2, [fp, #-136]
    ldur x19, [fp, #-104]
    stur x19, [fp, #-144]
    stur x1, [fp, #-152]
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    stur x2, [fp, #-24]
    stur s0, [fp, #-28]
    sub x0, fp, #152
    mov x1, #4
    bl _concat
    mov x2, x0
    ldur x0, [fp, #-8]
    str x2, [fp, #16]
    str x0, [fp, #24]
    ldp fp, lr, [sp, #160]
    add sp, sp, #176
    ret
.cfi_endproc
.global _issue08_Zoissue08_Zdissue1_had
_issue08_Zoissue08_Zdissue1_had:
.cfi_startproc
Lblk8:
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
    stur s0, [fp, #-4]
    stur s1, [fp, #-8]
    bl _current_root
    mov x26, x0
    ldur s0, [fp, #-4]
    ldur s1, [fp, #-8]
    bl _issue08_Zoissue08_Zdissue1
    mov x0, #16
    bl _reserve_handle_returning_vals
    mov x1, sp
    mov x2, #16
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
.global _issue08_Zoissue08_ZdFoo_ZdtoString
_issue08_Zoissue08_ZdFoo_ZdtoString:
.cfi_startproc
Lblk0:
    sub sp, sp, #96
    stp fp, lr, [sp, #80]
    add fp, sp, #80
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    b Lblk1
    nop
Lblk1:
    cmp x0, #0
    b.ne Lblk3
Lblk2:
    adrp x19, Kstr.0@PAGE
    add x19, x19, Kstr.0@PAGEOFF
    ldr x2, [x19, #0]
    mov x1, x2
    b Lblk5
    nop
Lblk3:
    cmp x0, #0
    b.ne Lblk4
Lblk4:
    mov x3, x0
    adrp x19, Kstr.1@PAGE
    add x19, x19, Kstr.1@PAGEOFF
    ldr x0, [x19, #0]
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x4, [x19, #0]
    stur x4, [fp, #-8]
    stur x3, [fp, #-16]
    stur x0, [fp, #-24]
    stur x0, [fp, #-32]
    stur x1, [fp, #-40]
    stur x2, [fp, #-48]
    stur x3, [fp, #-56]
    stur x4, [fp, #-64]
    sub x0, fp, #24
    mov x1, #3
    bl _concat
    mov x2, x0
    ldur x0, [fp, #-40]
    ldur x1, [fp, #-48]
    mov x0, x2
    b Lblk5
    nop
Lblk5:
    str x0, [fp, #24]
    ldp fp, lr, [sp, #80]
    add sp, sp, #96
    ret
.cfi_endproc
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "A"
Lkzs.1:
    .asciz "B("
Lkzs.2:
    .asciz ")"
Lkzs.3:
    .asciz "yalx/lang:lang"
Lkzs.4:
    .asciz "a="
Lkzs.5:
    .asciz ",b="
Lkzs.6:
    .asciz ""
Lkzs.7:
    .asciz ","
Lkzs.8:
    .asciz "fun (issue08:issue08.Foo)->(string)"
Lkzs.9:
    .asciz "toString"
Lkzs.10:
    .asciz "B"
Lkzs.11:
    .asciz "Foo"
Lkzs.12:
    .asciz "issue08:issue08.Foo"
.section __DATA,__data
.p2align 4
; classes:
.global _issue08_Zoissue08_ZdFoo$class
_issue08_Zoissue08_ZdFoo$class:
    .quad 0 ; id
    .byte 2 ; constraint
    .byte 1 ; compact enum
    .space 2 ; padding
    .long 8 ; reference_size
    .long 8 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.11 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.12 ; location
    .long 19 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 2 ; n_fields
    .space 4 ; padding
    .quad _issue08_Zoissue08_ZdFoo$fields ; fields
    .quad 0 ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue08_Zoissue08_ZdFoo$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
    .long 1 ; refs_mark_len
    .space 4
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 0
_issue08_Zoissue08_ZdFoo$fields:
    ; Foo::A
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.0 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad 0 ; type
    .long 0 ; offset_of_head
    .space 4 ; padding
    ; Foo::B
    .long 16 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.10 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 0 ; offset_of_head
    .space 4 ; padding
_issue08_Zoissue08_ZdFoo$methods:
    ; Foo::toString
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.9 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.8 ; prototype_desc
    .long 35 ; prototype_desc
    .space 4 ; padding
    .quad _issue08_Zoissue08_ZdFoo_ZdtoString ; entry
.section __DATA,__data
.p2align 4
.global _issue08_Zoissue08$global_slots
_issue08_Zoissue08$global_slots:
    .quad 0 ; size_in_bytes
    .quad 0 ; slots
    .quad 0 ; mark_size
; string constants:
; Yalx-String constants
.global _issue08_Zoissue08_Lksz
_issue08_Zoissue08_Lksz:
    .long 13
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
.global _issue08_Zoissue08_Kstr
_issue08_Zoissue08_Kstr:
    .long 13
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
