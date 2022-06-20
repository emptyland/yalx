.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "tests/42-code-gen-globals/src/issue03" "foo.yalx"
.p2align 2

; functions
.global _issue03_Zoissue03_Zd_Z4init
_issue03_Zoissue03_Zd_Z4init:
.cfi_startproc
Lblk3:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _yalx_Zplang_Zolang_Zd_Z4init@PAGE
    add x0, x19, _yalx_Zplang_Zolang_Zd_Z4init@PAGEOFF
    adrp x19, Lkzs.1@PAGE
    add x1, x19, Lkzs.1@PAGEOFF
    bl _pkg_init_once
    mov x0, #0
    adrp x19, _issue03_Zoissue03_ZdBaz@PAGE
    add x19, x19, _issue03_Zoissue03_ZdBaz@PAGEOFF
    str x0, [x19, #0]
    mov w0, #1
    adrp x19, _issue03_Zoissue03_Zda@PAGE
    add x19, x19, _issue03_Zoissue03_Zda@PAGEOFF
    str w0, [x19, #0]
    mov w0, #2
    adrp x19, _issue03_Zoissue03_Zdb@PAGE
    add x19, x19, _issue03_Zoissue03_Zdb@PAGEOFF
    str w0, [x19, #0]
    add x0, fp, #-32
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x1, [x19, #0]
    adrp x19, Kstr.3@PAGE
    add x19, x19, Kstr.3@PAGEOFF
    ldr x2, [x19, #0]
    stur x1, [fp, #-40]
    stur x2, [fp, #-48]
    bl _issue03_Zoissue03_ZdBar_ZdBar_Z4constructor
    adrp x20, _issue03_Zoissue03_Zdbar@PAGE
    add x20, x20, _issue03_Zoissue03_Zdbar@PAGEOFF
    ldur x19, [fp, #-32]
    str x19, [x20, #0]
    ldur x19, [fp, #-24]
    str x19, [x20, #8]
    ldur x19, [fp, #-16]
    str x19, [x20, #16]
    ldur x19, [fp, #-8]
    str x19, [x20, #24]
    stur x0, [fp, #-8]
    adrp x19, _issue03_Zoissue03_ZdFoo$class@PAGE
    add x0, x19, _issue03_Zoissue03_ZdFoo$class@PAGEOFF
    bl _heap_alloc
    mov w1, #1
    mov w2, #2
    adrp x19, Kstr.4@PAGE
    add x19, x19, Kstr.4@PAGEOFF
    ldr x3, [x19, #0]
    stur x0, [fp, #-16]
    stur w1, [fp, #-20]
    stur w2, [fp, #-24]
    stur x3, [fp, #-32]
    bl _issue03_Zoissue03_ZdFoo_ZdFoo_Z4constructor
    ldur x0, [fp, #-16]
    adrp x19, _issue03_Zoissue03_Zdfoo@PAGE
    add x19, x19, _issue03_Zoissue03_Zdfoo@PAGEOFF
    str x0, [x19, #0]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.cfi_endproc
.global _issue03_Zoissue03_Zdissue1
_issue03_Zoissue03_Zdissue1:
.cfi_startproc
Lblk4:
    sub sp, sp, #32
    stp fp, lr, [sp, #16]
    add fp, sp, #16
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _issue03_Zoissue03_ZdBaz@PAGE
    add x0, x19, _issue03_Zoissue03_ZdBaz@PAGEOFF
    adrp x19, _issue03_Zoissue03_ZdBaz_Z4ShadowClass$class@PAGE
    add x1, x19, _issue03_Zoissue03_ZdBaz_Z4ShadowClass$class@PAGEOFF
    bl _lazy_load_object
    ldr w1, [x0, #16]
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    adrp x19, _issue03_Zoissue03_ZdBaz@PAGE
    add x0, x19, _issue03_Zoissue03_ZdBaz@PAGEOFF
    adrp x19, _issue03_Zoissue03_ZdBaz_Z4ShadowClass$class@PAGE
    add x1, x19, _issue03_Zoissue03_ZdBaz_Z4ShadowClass$class@PAGEOFF
    bl _lazy_load_object
    ldur w1, [fp, #-12]
    ldr w2, [x0, #20]
    str w2, [fp, #24]
    str w1, [fp, #28]
    ldp fp, lr, [sp, #16]
    add sp, sp, #32
    ret
.cfi_endproc
.global _issue03_Zoissue03_Zdissue1_had
_issue03_Zoissue03_Zdissue1_had:
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
    bl _current_root
    mov x26, x0
    bl _issue03_Zoissue03_Zdissue1
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
.global _issue03_Zoissue03_Zddisplay
_issue03_Zoissue03_Zddisplay:
.cfi_startproc
Lblk6:
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
.global _issue03_Zoissue03_ZdFoo_ZdFoo_Z4constructor
_issue03_Zoissue03_ZdFoo_ZdFoo_Z4constructor:
.cfi_startproc
Lblk0:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov x4, x0
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    stur w2, [fp, #-16]
    stur x3, [fp, #-24]
    stur x4, [fp, #-32]
    ldur x0, [fp, #-32]
    bl _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    ldur x0, [fp, #-8]
    ldur w1, [fp, #-12]
    ldur w2, [fp, #-16]
    ldur x3, [fp, #-24]
    str w1, [x0, #16]
    str w2, [x0, #20]
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    stur w2, [fp, #-16]
    stur x3, [fp, #-24]
    add x0, x0, #24
    mov x1, x3
    bl _put_field
    ldur x0, [fp, #-8]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _issue03_Zoissue03_ZdBaz_Z4ShadowClass_ZdBaz_Z4ShadowClass_Z4constructor
_issue03_Zoissue03_ZdBaz_Z4ShadowClass_ZdBaz_Z4ShadowClass_Z4constructor:
.cfi_startproc
Lblk1:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov x1, x0
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    ldur x0, [fp, #-16]
    bl _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    ldur x0, [fp, #-8]
    mov w1, #996
    str w1, [x0, #16]
    mov w1, #777
    str w1, [x0, #20]
    adrp x19, Kstr.0@PAGE
    add x19, x19, Kstr.0@PAGEOFF
    ldr x1, [x19, #0]
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    add x0, x0, #24
    bl _put_field
    ldur x0, [fp, #-8]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue03_Zoissue03_ZdBar_ZdBar_Z4constructor
_issue03_Zoissue03_ZdBar_ZdBar_Z4constructor:
.cfi_startproc
Lblk2:
    sub sp, sp, #16
    stp fp, lr, [sp, #0]
    add fp, sp, #0
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    str x1, [x0, #16]
    str x2, [x0, #24]
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
    ldp fp, lr, [sp, #0]
    add sp, sp, #16
    ret
.cfi_endproc
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "world"
Lkzs.1:
    .asciz "yalx/lang:lang"
Lkzs.2:
    .asciz "Hello"
Lkzs.3:
    .asciz "World"
Lkzs.4:
    .asciz "hello"
Lkzs.5:
    .asciz "fun (issue03:issue03.Foo,i32,i32,string)->(void)"
Lkzs.6:
    .asciz "Foo$constructor"
Lkzs.7:
    .asciz "i"
Lkzs.8:
    .asciz "j"
Lkzs.9:
    .asciz "s"
Lkzs.10:
    .asciz "Foo"
Lkzs.11:
    .asciz "issue03:issue03.Foo"
Lkzs.12:
    .asciz "fun (issue03:issue03.Baz$ShadowClass)->(void)"
Lkzs.13:
    .asciz "Baz$ShadowClass$constructor"
Lkzs.14:
    .asciz "Baz$ShadowClass"
Lkzs.15:
    .asciz "issue03:issue03.Baz$ShadowClass"
Lkzs.16:
    .asciz "fun (issue03:issue03.Bar,string,string)->(void)"
Lkzs.17:
    .asciz "Bar$constructor"
Lkzs.18:
    .asciz "firstName"
Lkzs.19:
    .asciz "lastName"
Lkzs.20:
    .asciz "Bar"
Lkzs.21:
    .asciz "issue03:issue03.Bar"
.section __DATA,__data
.p2align 4
; classes:
.global _issue03_Zoissue03_ZdFoo$class
_issue03_Zoissue03_ZdFoo$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .space 3 ; padding
    .long 8 ; reference_size
    .long 32 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.10 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.11 ; location
    .long 19 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 3 ; n_fields
    .space 4 ; padding
    .quad _issue03_Zoissue03_ZdFoo$fields ; fields
    .quad _issue03_Zoissue03_ZdFoo$ctor ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue03_Zoissue03_ZdFoo$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
_issue03_Zoissue03_ZdFoo$fields:
    ; Foo::i
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.7 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+864 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; Foo::j
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.8 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+864 ; type
    .long 20 ; offset_of_head
    .space 4 ; padding
    ; Foo::s
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.9 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
_issue03_Zoissue03_ZdFoo$methods:
_issue03_Zoissue03_ZdFoo$ctor:
    ; Foo::Foo$constructor
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.6 ; name
    .long 15 ; name
    .space 4 ; padding
    .quad Lkzs.5 ; prototype_desc
    .long 48 ; prototype_desc
    .space 4 ; padding
    .quad _issue03_Zoissue03_ZdFoo_ZdFoo_Z4constructor ; entry
.global _issue03_Zoissue03_ZdBaz_Z4ShadowClass$class
_issue03_Zoissue03_ZdBaz_Z4ShadowClass$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .space 3 ; padding
    .long 8 ; reference_size
    .long 32 ; instance_size
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.14 ; name
    .long 15 ; name
    .space 4 ; padding
    .quad Lkzs.15 ; location
    .long 31 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 3 ; n_fields
    .space 4 ; padding
    .quad _issue03_Zoissue03_ZdBaz_Z4ShadowClass$fields ; fields
    .quad _issue03_Zoissue03_ZdBaz_Z4ShadowClass$ctor ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue03_Zoissue03_ZdBaz_Z4ShadowClass$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
_issue03_Zoissue03_ZdBaz_Z4ShadowClass$fields:
    ; Baz$ShadowClass::i
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.7 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+864 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; Baz$ShadowClass::j
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.8 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+864 ; type
    .long 20 ; offset_of_head
    .space 4 ; padding
    ; Baz$ShadowClass::s
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.9 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
_issue03_Zoissue03_ZdBaz_Z4ShadowClass$methods:
_issue03_Zoissue03_ZdBaz_Z4ShadowClass$ctor:
    ; Baz$ShadowClass::Baz$ShadowClass$constructor
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.13 ; name
    .long 27 ; name
    .space 4 ; padding
    .quad Lkzs.12 ; prototype_desc
    .long 45 ; prototype_desc
    .space 4 ; padding
    .quad _issue03_Zoissue03_ZdBaz_Z4ShadowClass_ZdBaz_Z4ShadowClass_Z4constructor ; entry
.global _issue03_Zoissue03_ZdBar$class
_issue03_Zoissue03_ZdBar$class:
    .quad 0 ; id
    .byte 1 ; constraint
    .space 3 ; padding
    .long 8 ; reference_size
    .long 32 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.20 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.21 ; location
    .long 19 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 2 ; n_fields
    .space 4 ; padding
    .quad _issue03_Zoissue03_ZdBar$fields ; fields
    .quad _issue03_Zoissue03_ZdBar$ctor ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue03_Zoissue03_ZdBar$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
_issue03_Zoissue03_ZdBar$fields:
    ; Bar::firstName
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.18 ; name
    .long 9 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; Bar::lastName
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.19 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
_issue03_Zoissue03_ZdBar$methods:
_issue03_Zoissue03_ZdBar$ctor:
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
    .long 47 ; prototype_desc
    .space 4 ; padding
    .quad _issue03_Zoissue03_ZdBar_ZdBar_Z4constructor ; entry
.section __DATA,__data
.p2align 4
; global slots:
pkg_global_slots:
_issue03_Zoissue03_Zdfoo:
    .quad 0
_issue03_Zoissue03_Zda:
    .long 0
_issue03_Zoissue03_Zdb:
    .long 0
_issue03_Zoissue03_Zdbar:
    .space 32
_issue03_Zoissue03_ZdBaz:
    .quad 0
.global _issue03_Zoissue03$global_slots
_issue03_Zoissue03$global_slots:
    .quad 56 ; size_in_bytes
    .quad pkg_global_slots ; slots
    .quad 4 ; mark_size
    .long 0
    .long 32
    .long 40
    .long 48
; string constants:
; Yalx-String constants
.global _issue03_Zoissue03_Lksz
_issue03_Zoissue03_Lksz:
    .long 22
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
.global _issue03_Zoissue03_Kstr
_issue03_Zoissue03_Kstr:
    .long 22
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
