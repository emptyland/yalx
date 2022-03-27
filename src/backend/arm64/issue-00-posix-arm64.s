.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "tests/25-ir-throw-catch-expr/src/issue00" "foo.yalx"
.p2align 2

; functions
.global _issue00_Zoissue00_Zd_Z4init
_issue00_Zoissue00_Zd_Z4init:
.cfi_startproc
Lblk1:
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
.global _issue00_Zoissue00_Zdissue1
_issue00_Zoissue00_Zdissue1:
.cfi_startproc
Lblk2:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    ldr x0, [x26, #104]
    stur x0, [fp, #-16]
    adrp x19, _issue00_Zoissue00_Zdissue1@PAGE
    add x0, x19, _issue00_Zoissue00_Zdissue1@PAGEOFF
    stur x0, [fp, #-8]
    sub x0, fp, #16
    str x0, [x26, #104]
Lblk3:
    add sp, sp, #16
    bl _issue00_Zoissue00_Zdbar
    sub sp, sp, #16
    ldr x19, [x26, #112]
    cmp x19, xzr
    b.ne Lblk4
    bl _issue00_Zoissue00_Zdfoo
    ldr x19, [x26, #112]
    cmp x19, xzr
    b.ne Lblk4
    adrp x19, Kstr.1@PAGE
    add x19, x19, Kstr.1@PAGEOFF
    ldr x0, [x19, #0]
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x1, [x19, #0]
    stur x0, [fp, #-24]
    stur x1, [fp, #-32]
    bl _issue00_Zoissue00_ZdassertString
    ldr x19, [x26, #112]
    cmp x19, xzr
    b.ne Lblk4
    b Lblk6
    nop
Lblk4:
    ldr x0, [x26, #112]
    stur x0, [fp, #-24]
    adrp x19, _yalx_Zplang_Zolang_ZdThrowable$class@PAGE
    add x1, x19, _yalx_Zplang_Zolang_ZdThrowable$class@PAGEOFF
    bl _is_instance_of
    ldur x1, [fp, #-24]
    cmp w0, #0
    b.eq Lblk6
Lblk5:
    str xzr, [x26, #112]
    sturb w0, [fp, #-20]
    stur x1, [fp, #-28]
    ldur x0, [fp, #-28]
    adrp x19, _yalx_Zplang_Zolang_ZdException$class@PAGE
    add x1, x19, _yalx_Zplang_Zolang_ZdException$class@PAGEOFF
    bl _ref_asserted_to
    mov x1, x0
    stur x0, [fp, #-28]
    stur x1, [fp, #-40]
    ldur x0, [fp, #-40]
    bl _yalx_Zplang_Zolang_ZdThrowable_ZdprintBacktrace
    b Lblk6
    nop
Lblk6:
    ldr x0, [x26, #104]
    ldur x19, [fp, #-16]
    str x19, [x0, #0]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _issue00_Zoissue00_Zdissue1_had
_issue00_Zoissue00_Zdissue1_had:
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
    bl _issue00_Zoissue00_Zdissue1
    ldp x19, x20, [sp, #64]
    ldp x21, x22, [sp, #48]
    ldp x23, x24, [sp, #32]
    ldp x25, x26, [sp, #16]
    ldp x27, x28, [sp, #0]
    ldp fp, lr, [sp, #80]
    add sp, sp, #96
    ret
.cfi_endproc
.global _issue00_Zoissue00_Zdfoo
_issue00_Zoissue00_Zdfoo:
.cfi_startproc
Lblk8:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _yalx_Zplang_Zolang_ZdException$class@PAGE
    add x0, x19, _yalx_Zplang_Zolang_ZdException$class@PAGEOFF
    bl _heap_alloc
    stur x0, [fp, #-8]
    mov x0, #0
    adrp x19, _yalx_Zplang_Zolang_ZdException$class@PAGE
    add x1, x19, _yalx_Zplang_Zolang_ZdException$class@PAGEOFF
    bl _ref_asserted_to
    mov x1, x0
    ldur x2, [fp, #-8]
    adrp x19, Kstr.3@PAGE
    add x19, x19, Kstr.3@PAGEOFF
    ldr x0, [x19, #0]
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    stur x2, [fp, #-24]
    ldur x0, [fp, #-24]
    ldur x1, [fp, #-8]
    ldur x2, [fp, #-16]
    bl _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
    ldur x0, [fp, #-24]
    bl _throw_it
    brk #0x3c
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue00_Zoissue00_Zdbar
_issue00_Zoissue00_Zdbar:
.cfi_startproc
Lblk9:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, _issue00_Zoissue00_ZdBar$class@PAGE
    add x0, x19, _issue00_Zoissue00_ZdBar$class@PAGEOFF
    bl _heap_alloc
    mov w1, #1
    mov w2, #2
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    stur w2, [fp, #-16]
    bl _issue00_Zoissue00_ZdBar_ZdBar_Z4constructor
    ldur x0, [fp, #-8]
    stur x0, [fp, #-8]
    ldur x0, [fp, #-8]
    adrp x19, _yalx_Zplang_Zolang_ZdException$class@PAGE
    add x1, x19, _yalx_Zplang_Zolang_ZdException$class@PAGEOFF
    bl _ref_asserted_to
    str x0, [fp, #24]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue00_Zoissue00_Zddisplay
_issue00_Zoissue00_Zddisplay:
.cfi_startproc
Lblk10:
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
.global _issue00_Zoissue00_ZdassertString
_issue00_Zoissue00_ZdassertString:
.cfi_startproc
Lblk11:
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
.global _issue00_Zoissue00_ZdBar_ZdBar_Z4constructor
_issue00_Zoissue00_ZdBar_ZdBar_Z4constructor:
.cfi_startproc
Lblk0:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
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
    .asciz "yalx/lang:lang"
Lkzs.1:
    .asciz "never run it"
Lkzs.2:
    .asciz ""
Lkzs.3:
    .asciz "ok"
Lkzs.4:
    .asciz "fun (issue00:issue00.Bar,i32,i32)->(void)"
Lkzs.5:
    .asciz "Bar$constructor"
Lkzs.6:
    .asciz "x"
Lkzs.7:
    .asciz "y"
Lkzs.8:
    .asciz "Bar"
Lkzs.9:
    .asciz "issue00:issue00.Bar"
.section __DATA,__data
.p2align 4
; classes:
.global _issue00_Zoissue00_ZdBar$class
_issue00_Zoissue00_ZdBar$class:
    .quad 0 ; id
    .byte 0 ; constraint
    .byte 0 ; padding
    .byte 0
    .byte 0
    .long 8 ; reference_size
    .long 24 ; instance_size
    .long 0 ; padding
    .quad _yalx_Zplang_Zolang_ZdAny$class ; super
    .quad Lkzs.8 ; name
    .long 3 ; name
    .long 0 ; padding
    .quad Lkzs.9 ; location
    .long 19 ; location
    .long 0 ; padding
    .long 0 ; n_annotations
    .long 0 ; padding
    .quad 0 ; reserved0
    .long 2 ; n_fields
    .long 0 ; padding
    .quad _issue00_Zoissue00_ZdBar$fields ; fields
    .quad _issue00_Zoissue00_ZdBar$ctor ; ctor
    .long 1 ; n_methods
    .long 0 ; padding
    .quad _issue00_Zoissue00_ZdBar$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
_issue00_Zoissue00_ZdBar$fields:
    ; Bar::x
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.6 ; name
    .long 1 ; name
    .long 0 ; padding
    .quad 0 ; type
    .long 16 ; offset_of_head
    .long 0 ; padding
    ; Bar::y
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.7 ; name
    .long 1 ; name
    .long 0 ; padding
    .quad 0 ; type
    .long 20 ; offset_of_head
    .long 0 ; padding
_issue00_Zoissue00_ZdBar$methods:
_issue00_Zoissue00_ZdBar$ctor:
    ; Bar::Bar$constructor
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .long 0 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.5 ; name
    .long 15 ; name
    .long 0 ; padding
    .quad Lkzs.4 ; prototype_desc
    .long 41 ; prototype_desc
    .long 0 ; padding
    .quad _issue00_Zoissue00_ZdBar_ZdBar_Z4constructor ; entry
.section __DATA,__data
.p2align 4
; Yalx-String constants
.global _issue00_Zoissue00_Lksz
_issue00_Zoissue00_Lksz:
    .long 10
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
.global _issue00_Zoissue00_Kstr
_issue00_Zoissue00_Kstr:
    .long 10
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
