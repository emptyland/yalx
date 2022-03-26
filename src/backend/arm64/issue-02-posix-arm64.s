.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "tests/41-code-gen-structs/src/issue02" "foo.yalx"
.p2align 2

; functions
.global _issue02_Zoissue02_Zd_Z4init
_issue02_Zoissue02_Zd_Z4init:
.cfi_startproc
Lblk5:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _yalx_Zplang_Zolang_Zd_Z4init@PAGE
    add x0, x19, _yalx_Zplang_Zolang_Zd_Z4init@PAGEOFF
    adrp x19, Lkzs.1@PAGE
    add x1, x19, Lkzs.1@PAGEOFF
    bl _pkg_init_once
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue02_Zoissue02_Zdissue1
_issue02_Zoissue02_Zdissue1:
.cfi_startproc
Lblk6:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    add x0, fp, #-64
    mov w1, #1
    mov w2, #2
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x3, [x19, #0]
    stur x0, [fp, #-72]
    bl _issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor
    ldur x0, [fp, #-72]
    ldr w1, [x0, #16]
    ldr w2, [x0, #20]
    add w3, w1, w2
    mov w1, #3
    stur x0, [fp, #-72]
    mov w0, w1
    mov w1, w3
    bl _issue02_Zoissue02_ZdassertInt
    ldur x0, [fp, #-72]
    ldr x1, [x0, #24]
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x0, [x19, #0]
    bl _issue02_Zoissue02_ZdassertString
    adrp x19, _issue02_Zoissue02_ZdBar$class@PAGE
    add x0, x19, _issue02_Zoissue02_ZdBar$class@PAGEOFF
    bl _heap_alloc
    mov w1, #2
    mov w2, #3
    adrp x19, Kstr.3@PAGE
    add x19, x19, Kstr.3@PAGEOFF
    ldr x3, [x19, #0]
    stur x0, [fp, #-40]
    bl _issue02_Zoissue02_ZdBar_ZdBar_Z4constructor
    ldur x0, [fp, #-40]
    ldr w1, [x0, #16]
    ldr w2, [x0, #20]
    add w3, w1, w2
    mov w1, #5
    stur x0, [fp, #-40]
    mov w0, w1
    mov w1, w3
    bl _issue02_Zoissue02_ZdassertInt
    ldur x0, [fp, #-40]
    ldr x1, [x0, #24]
    adrp x19, Kstr.3@PAGE
    add x19, x19, Kstr.3@PAGEOFF
    ldr x0, [x19, #0]
    bl _issue02_Zoissue02_ZdassertString
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _issue02_Zoissue02_Zdissue1_had
_issue02_Zoissue02_Zdissue1_had:
.cfi_startproc
Lblk7:
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
    bl _issue02_Zoissue02_Zdissue1
    ldp x19, x20, [sp, #64]
    ldp x21, x22, [sp, #48]
    ldp x23, x24, [sp, #32]
    ldp x25, x26, [sp, #16]
    ldp x27, x28, [sp, #0]
    ldp fp, lr, [sp, #80]
    add sp, sp, #96
    ret
.cfi_endproc
.global _issue02_Zoissue02_Zdissue2
_issue02_Zoissue02_Zdissue2:
.cfi_startproc
Lblk8:
    sub sp, sp, #112
    stp fp, lr, [sp, #96]
    add fp, sp, #96
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    add x0, fp, #-64
    mov w1, #1
    mov w2, #2
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x3, [x19, #0]
    stur x0, [fp, #-72]
    bl _issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor
    ldur x0, [fp, #-72]
    ldr x1, [x0, #24]
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x2, [x19, #0]
    stur x0, [fp, #-72]
    mov x0, x2
    bl _issue02_Zoissue02_ZdassertString
    ldur x0, [fp, #-72]
    adrp x19, Kstr.4@PAGE
    add x19, x19, Kstr.4@PAGEOFF
    ldr x1, [x19, #0]
    str x1, [x0, #24]
    ldr x19, [x0, #24]
    str x19, [x0, #0]
    ldr x19, [x0, #32]
    str x19, [x0, #8]
    ldr x19, [x0, #40]
    str x19, [x0, #16]
    ldr x19, [x0, #48]
    str x19, [x0, #24]
    adrp x19, Kstr.4@PAGE
    add x19, x19, Kstr.4@PAGEOFF
    ldr x0, [x19, #0]
    mov x1, x0
    bl _issue02_Zoissue02_ZdassertString
    ldp fp, lr, [sp, #96]
    add sp, sp, #112
    ret
.cfi_endproc
.global _issue02_Zoissue02_Zdissue2_had
_issue02_Zoissue02_Zdissue2_had:
.cfi_startproc
Lblk9:
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
    bl _issue02_Zoissue02_Zdissue2
    ldp x19, x20, [sp, #64]
    ldp x21, x22, [sp, #48]
    ldp x23, x24, [sp, #32]
    ldp x25, x26, [sp, #16]
    ldp x27, x28, [sp, #0]
    ldp fp, lr, [sp, #80]
    add sp, sp, #96
    ret
.cfi_endproc
.global _issue02_Zoissue02_Zdissue3
_issue02_Zoissue02_Zdissue3:
.cfi_startproc
Lblk10:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _yalx_Zplang_Zolang_ZdException$class@PAGE
    add x0, x19, _yalx_Zplang_Zolang_ZdException$class@PAGEOFF
    bl _heap_alloc
    mov x2, #0
    mov x1, x2
    adrp x19, Kstr.5@PAGE
    add x19, x19, Kstr.5@PAGEOFF
    ldr x2, [x19, #0]
    stur x0, [fp, #-40]
    mov x1, x2
    mov x2, x1
    bl _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
    ldur x0, [fp, #-40]
    ldr x1, [x0, #16]
    adrp x19, Kstr.5@PAGE
    add x19, x19, Kstr.5@PAGEOFF
    ldr x0, [x19, #0]
    bl _issue02_Zoissue02_ZdassertString
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _issue02_Zoissue02_Zdissue3_had
_issue02_Zoissue02_Zdissue3_had:
.cfi_startproc
Lblk11:
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
    bl _issue02_Zoissue02_Zdissue3
    ldp x19, x20, [sp, #64]
    ldp x21, x22, [sp, #48]
    ldp x23, x24, [sp, #32]
    ldp x25, x26, [sp, #16]
    ldp x27, x28, [sp, #0]
    ldp fp, lr, [sp, #80]
    add sp, sp, #96
    ret
.cfi_endproc
.global _issue02_Zoissue02_Zddisplay
_issue02_Zoissue02_Zddisplay:
.cfi_startproc
Lblk12:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-40]
    sub x0, fp, #40
    bl _yalx_Zplang_Zolang_Zdprintln_stub
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _issue02_Zoissue02_ZdassertString
_issue02_Zoissue02_ZdassertString:
.cfi_startproc
Lblk13:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    stur x0, [fp, #-40]
    stur x1, [fp, #-48]
    sub x0, fp, #40
    sub x1, fp, #48
    bl _assert_string_stub
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.cfi_endproc
.global _issue02_Zoissue02_ZdassertInt
_issue02_Zoissue02_ZdassertInt:
.cfi_startproc
Lblk14:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    bl _assert_int_stub
    bl _current_root
    mov x26, x0
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue02_Zoissue02_ZdBar_ZdtoString
_issue02_Zoissue02_ZdBar_ZdtoString:
.cfi_startproc
Lblk0:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr x1, [x0, #24]
    str x1, [fp, #24]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue02_Zoissue02_ZdBar_ZdBar_Z4constructor
_issue02_Zoissue02_ZdBar_ZdBar_Z4constructor:
.cfi_startproc
Lblk1:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov x4, x0
    stur x0, [fp, #-40]
    stur w1, [fp, #-44]
    stur w2, [fp, #-48]
    stur x3, [fp, #-56]
    mov x0, x4
    bl _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    ldur x0, [fp, #-40]
    ldur w1, [fp, #-44]
    ldur w2, [fp, #-48]
    ldur x3, [fp, #-56]
    str w1, [x0, #16]
    str w2, [x0, #20]
    str x3, [x0, #24]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.cfi_endproc
.global _issue02_Zoissue02_ZdFoo_ZddoIt
_issue02_Zoissue02_ZdFoo_ZddoIt:
.cfi_startproc
Lblk2:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr x1, [x0, #24]
    mov x0, x1
    bl _issue02_Zoissue02_Zddisplay
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue02_Zoissue02_ZdFoo_ZddoThat
_issue02_Zoissue02_ZdFoo_ZddoThat:
.cfi_startproc
Lblk3:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, Kstr.0@PAGE
    add x19, x19, Kstr.0@PAGEOFF
    ldr x1, [x19, #0]
    stur x0, [fp, #-40]
    mov x0, x1
    bl _issue02_Zoissue02_Zddisplay
    ldur x0, [fp, #-40]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor
_issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor:
.cfi_startproc
Lblk4:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    str w1, [x0, #16]
    str w2, [x0, #20]
    str x3, [x0, #24]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "doThat"
Lkzs.1:
    .asciz "yalx/lang:lang"
Lkzs.2:
    .asciz "name"
Lkzs.3:
    .asciz "bar"
Lkzs.4:
    .asciz "Doom"
Lkzs.5:
    .asciz "Error"
Lkzs.6:
    .asciz "fun (issue02:issue02.Bar)->(string)"
Lkzs.7:
    .asciz "toString"
Lkzs.8:
    .asciz "fun (issue02:issue02.Bar,i32,i32,string)->(void)"
Lkzs.9:
    .asciz "Bar$constructor"
Lkzs.10:
    .asciz "x"
Lkzs.11:
    .asciz "y"
Lkzs.12:
    .asciz "Bar"
Lkzs.13:
    .asciz "issue02:issue02.Bar"
Lkzs.14:
    .asciz "fun (issue02:issue02.Foo)->(void)"
Lkzs.15:
    .asciz "doIt"
Lkzs.16:
    .asciz "fun (issue02:issue02.Foo,i32,i32,string)->(void)"
Lkzs.17:
    .asciz "Foo$constructor"
Lkzs.18:
    .asciz "Foo"
Lkzs.19:
    .asciz "issue02:issue02.Foo"
.section __DATA,__data
.p2align 4
; classes:
.global _issue02_Zoissue02_ZdBar$class
_issue02_Zoissue02_ZdBar$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; padding
    .byte 0
    .byte 0
    .long 8 ; reference_size
    .long 32 ; instance_size
    .long 0 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.12 ; name
    .long 3 ; name
    .long 0 ; padding
    .quad Lkzs.13 ; location
    .long 19 ; location
    .long 0 ; padding
    .long 0 ; n_annotations
    .long 0 ; padding
    .quad 0 ; reserved0
    .long 3 ; n_fields
    .long 0 ; padding
    .quad _issue02_Zoissue02_ZdBar$fields ; fields
    .quad _issue02_Zoissue02_ZdBar$ctor ; ctor
    .long 2 ; n_methods
    .long 0 ; padding
    .quad _issue02_Zoissue02_ZdBar$methods ; methods
    .long 5 ; n_vtab
    .long 0 ; n_itab
    .quad _issue02_Zoissue02_ZdBar$vtab ; vtab
    .quad 0 ; itab
_issue02_Zoissue02_ZdBar$fields:
    ; Bar::x
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.10 ; name
    .long 1 ; name
    .long 0 ; padding
    .quad 0 ; type
    .long 16 ; offset_of_head
    .long 0 ; padding
    ; Bar::y
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.11 ; name
    .long 1 ; name
    .long 0 ; padding
    .quad 0 ; type
    .long 20 ; offset_of_head
    .long 0 ; padding
    ; Bar::name
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.2 ; name
    .long 4 ; name
    .long 0 ; padding
    .quad 0 ; type
    .long 24 ; offset_of_head
    .long 0 ; padding
_issue02_Zoissue02_ZdBar$methods:
    ; Bar::toString
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .long 0 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.7 ; name
    .long 8 ; name
    .long 0 ; padding
    .quad Lkzs.6 ; prototype_desc
    .long 35 ; prototype_desc
    .long 0 ; padding
    .quad _issue02_Zoissue02_ZdBar_ZdtoString ; entry
_issue02_Zoissue02_ZdBar$ctor:
    ; Bar::Bar$constructor
    .long 1 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .long 0 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.9 ; name
    .long 15 ; name
    .long 0 ; padding
    .quad Lkzs.8 ; prototype_desc
    .long 48 ; prototype_desc
    .long 0 ; padding
    .quad _issue02_Zoissue02_ZdBar_ZdBar_Z4constructor ; entry
_issue02_Zoissue02_ZdBar$vtab:
    .quad _yalx_Zplang_Zolang_ZdAny_Zdfinalize
    .quad _yalx_Zplang_Zolang_ZdAny_ZdhashCode
    .quad _yalx_Zplang_Zolang_ZdAny_Zdid
    .quad _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
    .quad _issue02_Zoissue02_ZdBar_ZdtoString
.global _issue02_Zoissue02_ZdFoo$class
_issue02_Zoissue02_ZdFoo$class:
    .quad 0 ; id
    .byte 1 ; constraint
    .byte 0 ; padding
    .byte 0
    .byte 0
    .long 8 ; reference_size
    .long 32 ; instance_size
    .long 0 ; padding
    .quad 0 ; super
    .quad Lkzs.18 ; name
    .long 3 ; name
    .long 0 ; padding
    .quad Lkzs.19 ; location
    .long 19 ; location
    .long 0 ; padding
    .long 0 ; n_annotations
    .long 0 ; padding
    .quad 0 ; reserved0
    .long 3 ; n_fields
    .long 0 ; padding
    .quad _issue02_Zoissue02_ZdFoo$fields ; fields
    .quad _issue02_Zoissue02_ZdFoo$ctor ; ctor
    .long 3 ; n_methods
    .long 0 ; padding
    .quad _issue02_Zoissue02_ZdFoo$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
_issue02_Zoissue02_ZdFoo$fields:
    ; Foo::x
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.10 ; name
    .long 1 ; name
    .long 0 ; padding
    .quad 0 ; type
    .long 16 ; offset_of_head
    .long 0 ; padding
    ; Foo::y
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.11 ; name
    .long 1 ; name
    .long 0 ; padding
    .quad 0 ; type
    .long 20 ; offset_of_head
    .long 0 ; padding
    ; Foo::name
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.2 ; name
    .long 4 ; name
    .long 0 ; padding
    .quad 0 ; type
    .long 24 ; offset_of_head
    .long 0 ; padding
_issue02_Zoissue02_ZdFoo$methods:
    ; Foo::doIt
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .long 0 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.15 ; name
    .long 4 ; name
    .long 0 ; padding
    .quad Lkzs.14 ; prototype_desc
    .long 33 ; prototype_desc
    .long 0 ; padding
    .quad _issue02_Zoissue02_ZdFoo_ZddoIt ; entry
    ; Foo::doThat
    .long 1 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .long 0 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.0 ; name
    .long 6 ; name
    .long 0 ; padding
    .quad Lkzs.14 ; prototype_desc
    .long 33 ; prototype_desc
    .long 0 ; padding
    .quad _issue02_Zoissue02_ZdFoo_ZddoThat ; entry
_issue02_Zoissue02_ZdFoo$ctor:
    ; Foo::Foo$constructor
    .long 2 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .long 0 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.17 ; name
    .long 15 ; name
    .long 0 ; padding
    .quad Lkzs.16 ; prototype_desc
    .long 48 ; prototype_desc
    .long 0 ; padding
    .quad _issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor ; entry
.section __DATA,__data
.p2align 4
; Yalx-String constants
.global _issue02_Zoissue02_Lksz
_issue02_Zoissue02_Lksz:
    .long 20
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
.global _issue02_Zoissue02_Kstr
_issue02_Zoissue02_Kstr:
    .long 20
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
