.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "tests/46-code-gen-call-virtual/src/issue07" "foo.yalx"
.p2align 2

; functions
.global _issue07_Zoissue07_Zd_Z4init
_issue07_Zoissue07_Zd_Z4init:
.cfi_startproc
Lblk3:
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
.global _issue07_Zoissue07_Zdissue1
_issue07_Zoissue07_Zdissue1:
.cfi_startproc
Lblk4:
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
Lblk6:
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
.global _issue07_Zoissue07_Zdissue2_had
_issue07_Zoissue07_Zdissue2_had:
.cfi_startproc
Lblk7:
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
.global _issue07_Zoissue07_ZdFoo_ZdFoo_Z4constructor
_issue07_Zoissue07_ZdFoo_ZdFoo_Z4constructor:
.cfi_startproc
Lblk2:
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
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "Foo"
Lkzs.1:
    .asciz "yalx/lang:lang"
Lkzs.2:
    .asciz "fun (issue07:issue07.Foo)->(u32)"
Lkzs.3:
    .asciz "hashCode"
Lkzs.4:
    .asciz "fun (issue07:issue07.Foo)->(string)"
Lkzs.5:
    .asciz "toString"
Lkzs.6:
    .asciz "fun (issue07:issue07.Foo,u32)->(void)"
Lkzs.7:
    .asciz "Foo$constructor"
Lkzs.8:
    .asciz "n"
Lkzs.9:
    .asciz "issue07:issue07.Foo"
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
    .quad Lkzs.9 ; location
    .long 19 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 1 ; n_fields
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo$fields ; fields
    .quad _issue07_Zoissue07_ZdFoo$ctor ; ctor
    .long 3 ; n_methods
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo$methods ; methods
    .long 5 ; n_vtab
    .long 0 ; n_itab
    .quad _issue07_Zoissue07_ZdFoo$vtab ; vtab
    .quad 0 ; itab
    .long 0 ; refs_mark_len
    .space 4
_issue07_Zoissue07_ZdFoo$fields:
    ; Foo::n
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.8 ; name
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
    .quad Lkzs.3 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.2 ; prototype_desc
    .long 32 ; prototype_desc
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo_ZdhashCode ; entry
    ; Foo::toString
    .long 1 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.5 ; name
    .long 8 ; name
    .space 4 ; padding
    .quad Lkzs.4 ; prototype_desc
    .long 35 ; prototype_desc
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo_ZdtoString ; entry
_issue07_Zoissue07_ZdFoo$ctor:
    ; Foo::Foo$constructor
    .long 2 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.7 ; name
    .long 15 ; name
    .space 4 ; padding
    .quad Lkzs.6 ; prototype_desc
    .long 37 ; prototype_desc
    .space 4 ; padding
    .quad _issue07_Zoissue07_ZdFoo_ZdFoo_Z4constructor ; entry
_issue07_Zoissue07_ZdFoo$vtab:
    .quad _yalx_Zplang_Zolang_ZdAny_Zdfinalize
    .quad _issue07_Zoissue07_ZdFoo_ZdhashCode
    .quad _yalx_Zplang_Zolang_ZdAny_Zdid
    .quad _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
    .quad _issue07_Zoissue07_ZdFoo_ZdtoString
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
.global _issue07_Zoissue07_Kstr
_issue07_Zoissue07_Kstr:
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
