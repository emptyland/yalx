.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
; libc symbols:
.file 1 "tests/43-code-gen-arrays/src/issue04" "foo.yalx"
.p2align 2

; functions
.global _issue04_Zoissue04_Zd_Z4init
_issue04_Zoissue04_Zd_Z4init:
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
.global _issue04_Zoissue04_Zdissue1
_issue04_Zoissue04_Zdissue1:
.cfi_startproc
Lblk2:
    sub sp, sp, #64
    stp fp, lr, [sp, #48]
    add fp, sp, #48
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #1
    mov w1, #2
    mov w2, #3
    mov w3, #4
    stur w3, [fp, #-4]
    stur w2, [fp, #-8]
    stur w1, [fp, #-12]
    stur w0, [fp, #-16]
    stur w0, [fp, #-20]
    stur w1, [fp, #-24]
    stur w2, [fp, #-28]
    stur w3, [fp, #-32]
    adrp x19, _builtin_classes+864@PAGE
    add x0, x19, _builtin_classes+864@PAGEOFF
    sub x1, fp, #16
    mov w2, #4
    bl _array_alloc
    str x0, [fp, #24]
    ldp fp, lr, [sp, #48]
    add sp, sp, #64
    ret
.cfi_endproc
.global _issue04_Zoissue04_Zdissue1_had
_issue04_Zoissue04_Zdissue1_had:
.cfi_startproc
Lblk3:
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
    bl _issue04_Zoissue04_Zdissue1
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
.global _issue04_Zoissue04_Zdissue2
_issue04_Zoissue04_Zdissue2:
.cfi_startproc
Lblk4:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    adrp x19, Kstr.1@PAGE
    add x19, x19, Kstr.1@PAGEOFF
    ldr x0, [x19, #0]
    adrp x19, Kstr.2@PAGE
    add x19, x19, Kstr.2@PAGEOFF
    ldr x1, [x19, #0]
    adrp x19, Kstr.3@PAGE
    add x19, x19, Kstr.3@PAGEOFF
    ldr x2, [x19, #0]
    stur x2, [fp, #-8]
    stur x1, [fp, #-16]
    stur x0, [fp, #-24]
    stur x0, [fp, #-32]
    stur x1, [fp, #-40]
    stur x2, [fp, #-48]
    adrp x19, _yalx_Zplang_Zolang_ZdString$class@PAGE
    add x0, x19, _yalx_Zplang_Zolang_ZdString$class@PAGEOFF
    sub x1, fp, #24
    mov w2, #3
    bl _array_alloc
    str x0, [fp, #24]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.cfi_endproc
.global _issue04_Zoissue04_Zdissue2_had
_issue04_Zoissue04_Zdissue2_had:
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
    bl _issue04_Zoissue04_Zdissue2
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
.global _issue04_Zoissue04_Zdissue3
_issue04_Zoissue04_Zdissue3:
.cfi_startproc
Lblk6:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #1
    mov w1, #2
    stur w1, [fp, #-4]
    stur w0, [fp, #-8]
    stur w0, [fp, #-12]
    stur w1, [fp, #-16]
    adrp x19, _builtin_classes+864@PAGE
    add x0, x19, _builtin_classes+864@PAGEOFF
    sub x1, fp, #8
    mov w2, #2
    bl _array_alloc
    mov w1, #3
    mov w2, #4
    stur w2, [fp, #-20]
    stur w1, [fp, #-24]
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    stur w2, [fp, #-16]
    adrp x19, _builtin_classes+864@PAGE
    add x0, x19, _builtin_classes+864@PAGEOFF
    sub x1, fp, #24
    mov w2, #2
    bl _array_alloc
    ldur x1, [fp, #-8]
    mov w2, #5
    mov w3, #6
    stur w3, [fp, #-28]
    stur w2, [fp, #-32]
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    stur w2, [fp, #-20]
    stur w3, [fp, #-24]
    adrp x19, _builtin_classes+864@PAGE
    add x0, x19, _builtin_classes+864@PAGEOFF
    sub x1, fp, #32
    mov w2, #2
    bl _array_alloc
    ldur x1, [fp, #-8]
    ldur x2, [fp, #-16]
    stur x0, [fp, #-40]
    stur x1, [fp, #-48]
    stur x2, [fp, #-56]
    stur x0, [fp, #-8]
    stur x1, [fp, #-16]
    stur x2, [fp, #-24]
    adrp x19, _builtin_classes+3456@PAGE
    add x0, x19, _builtin_classes+3456@PAGEOFF
    sub x1, fp, #56
    mov w2, #3
    bl _array_alloc
    str x0, [fp, #24]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.cfi_endproc
.global _issue04_Zoissue04_Zdissue3_had
_issue04_Zoissue04_Zdissue3_had:
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
    bl _issue04_Zoissue04_Zdissue3
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
.global _issue04_Zoissue04_Zdissue4
_issue04_Zoissue04_Zdissue4:
.cfi_startproc
Lblk8:
    sub sp, sp, #48
    stp fp, lr, [sp, #32]
    add fp, sp, #32
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #8
    mov w1, #996
    stur w0, [fp, #-4]
    stur w1, [fp, #-8]
    adrp x19, _builtin_classes+864@PAGE
    add x0, x19, _builtin_classes+864@PAGEOFF
    ldur w2, [fp, #-4]
    ldur w1, [fp, #-8]
    bl _array_fill_w32
    mov x3, x0
    mov w0, #10
    stur w0, [fp, #-4]
    stur x3, [fp, #-16]
    adrp x19, _builtin_classes+3456@PAGE
    add x0, x19, _builtin_classes+3456@PAGEOFF
    ldur w2, [fp, #-4]
    ldur x1, [fp, #-16]
    bl _array_fill_dims
    mov x3, x0
    str x3, [fp, #24]
    ldp fp, lr, [sp, #32]
    add sp, sp, #48
    ret
.cfi_endproc
.global _issue04_Zoissue04_Zdissue4_had
_issue04_Zoissue04_Zdissue4_had:
.cfi_startproc
Lblk9:
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
    bl _issue04_Zoissue04_Zdissue4
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
.global _issue04_Zoissue04_Zdissue5
_issue04_Zoissue04_Zdissue5:
.cfi_startproc
Lblk10:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    mov w0, #1
    mov w1, #2
    mov w2, #3
    mov w3, #4
    mov w4, #5
    mov w5, #6
    mov w6, #7
    stur w6, [fp, #-4]
    stur w5, [fp, #-8]
    stur w4, [fp, #-12]
    stur w3, [fp, #-16]
    stur w2, [fp, #-20]
    stur w1, [fp, #-24]
    stur w0, [fp, #-28]
    stur w0, [fp, #-32]
    stur w1, [fp, #-36]
    stur w2, [fp, #-40]
    stur w3, [fp, #-44]
    stur w4, [fp, #-48]
    stur w5, [fp, #-52]
    stur w6, [fp, #-56]
    adrp x19, _builtin_classes+864@PAGE
    add x0, x19, _builtin_classes+864@PAGEOFF
    sub x1, fp, #28
    mov w2, #7
    bl _array_alloc
    mov w1, #1
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    ldur x0, [fp, #-8]
    ldur w1, [fp, #-12]
    bl _array_location_at
    ldr w2, [x0, #0]
    ldur x0, [fp, #-8]
    mov w1, #3
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    stur w2, [fp, #-16]
    ldur x0, [fp, #-8]
    ldur w1, [fp, #-12]
    bl _array_location_at
    ldr w2, [x0, #0]
    ldur x0, [fp, #-8]
    ldur w1, [fp, #-16]
    mov w3, #5
    stur x0, [fp, #-8]
    stur w1, [fp, #-12]
    stur w2, [fp, #-16]
    stur w3, [fp, #-20]
    ldur x0, [fp, #-8]
    ldur w1, [fp, #-20]
    bl _array_location_at
    ldr w2, [x0, #0]
    ldur w0, [fp, #-12]
    ldur w1, [fp, #-16]
    str w2, [fp, #20]
    str w1, [fp, #24]
    str w0, [fp, #28]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.cfi_endproc
.global _issue04_Zoissue04_Zdissue5_had
_issue04_Zoissue04_Zdissue5_had:
.cfi_startproc
Lblk11:
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
    bl _issue04_Zoissue04_Zdissue5
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
.global _issue04_Zoissue04_Zdissue6
_issue04_Zoissue04_Zdissue6:
.cfi_startproc
Lblk12:
    sub sp, sp, #80
    stp fp, lr, [sp, #64]
    add fp, sp, #64
    .cfi_def_cfa fp, 16
    .cfi_offset lr, -8
    .cfi_offset fp, -16
    add x0, fp, #-32
    mov w1, #0
    adrp x19, Kstr.4@PAGE
    add x19, x19, Kstr.4@PAGEOFF
    ldr x2, [x19, #0]
    stur w1, [fp, #-36]
    stur x2, [fp, #-44]
    bl _issue04_Zoissue04_ZdFoo_ZdFoo_Z4constructor
    mov w0, #10
    stur w0, [fp, #-36]
    adrp x19, _issue04_Zoissue04_ZdFoo$class@PAGE
    add x0, x19, _issue04_Zoissue04_ZdFoo$class@PAGEOFF
    ldur w2, [fp, #-36]
    add x1, fp, #-32
    bl _array_fill_chunks
    mov x3, x0
    add x0, fp, #-32
    mov w1, #1
    adrp x19, Kstr.5@PAGE
    add x19, x19, Kstr.5@PAGEOFF
    ldr x2, [x19, #0]
    stur w1, [fp, #-36]
    stur x2, [fp, #-44]
    stur x3, [fp, #-52]
    bl _issue04_Zoissue04_ZdFoo_ZdFoo_Z4constructor
    ldur x1, [fp, #-52]
    mov w0, #1
    stur w0, [fp, #-36]
    stur x1, [fp, #-44]
    ldur x0, [fp, #-44]
    ldur w1, [fp, #-36]
    add x2, fp, #-32
    bl _array_set_chunk
    ldur x19, [fp, #-44]
    str x19, [fp, #24]
    ldp fp, lr, [sp, #64]
    add sp, sp, #80
    ret
.cfi_endproc
.global _issue04_Zoissue04_Zdissue6_had
_issue04_Zoissue04_Zdissue6_had:
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
    bl _issue04_Zoissue04_Zdissue6
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
.global _issue04_Zoissue04_ZdFoo_ZdFoo_Z4constructor
_issue04_Zoissue04_ZdFoo_ZdFoo_Z4constructor:
.cfi_startproc
Lblk0:
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
; CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "yalx/lang:lang"
Lkzs.1:
    .asciz "hello"
Lkzs.2:
    .asciz "world"
Lkzs.3:
    .asciz "!"
Lkzs.4:
    .asciz ""
Lkzs.5:
    .asciz "ok"
Lkzs.6:
    .asciz "fun (issue04:issue04.Foo,i32,string)->(void)"
Lkzs.7:
    .asciz "Foo$constructor"
Lkzs.8:
    .asciz "i"
Lkzs.9:
    .asciz "name"
Lkzs.10:
    .asciz "Foo"
Lkzs.11:
    .asciz "issue04:issue04.Foo"
.section __DATA,__data
.p2align 4
; classes:
.global _issue04_Zoissue04_ZdFoo$class
_issue04_Zoissue04_ZdFoo$class:
    .quad 0 ; id
    .byte 1 ; constraint
    .space 3 ; padding
    .long 8 ; reference_size
    .long 32 ; instance_size
    .space 4 ; padding
    .quad 0 ; super
    .quad Lkzs.10 ; name
    .long 3 ; name
    .space 4 ; padding
    .quad Lkzs.11 ; location
    .long 19 ; location
    .space 4 ; padding
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .long 2 ; n_fields
    .space 4 ; padding
    .quad _issue04_Zoissue04_ZdFoo$fields ; fields
    .quad _issue04_Zoissue04_ZdFoo$ctor ; ctor
    .long 1 ; n_methods
    .space 4 ; padding
    .quad _issue04_Zoissue04_ZdFoo$methods ; methods
    .long 0 ; n_vtab
    .long 0 ; n_itab
    .quad 0 ; vtab
    .quad 0 ; itab
_issue04_Zoissue04_ZdFoo$fields:
    ; Foo::i
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.8 ; name
    .long 1 ; name
    .space 4 ; padding
    .quad _builtin_classes+864 ; type
    .long 16 ; offset_of_head
    .space 4 ; padding
    ; Foo::name
    .long 0 ; access|constraint
    .long 0 ; n_annotations
    .quad 0 ; reserved0
    .quad Lkzs.9 ; name
    .long 4 ; name
    .space 4 ; padding
    .quad _yalx_Zplang_Zolang_ZdString$class ; type
    .long 24 ; offset_of_head
    .space 4 ; padding
_issue04_Zoissue04_ZdFoo$methods:
_issue04_Zoissue04_ZdFoo$ctor:
    ; Foo::Foo$constructor
    .long 0 ; index
    .long 0 ; access|is_native|is_override|...
    .long 0 ; n_annotations
    .space 4 ; padding
    .quad 0 ; reserved0
    .quad Lkzs.7 ; name
    .long 15 ; name
    .space 4 ; padding
    .quad Lkzs.6 ; prototype_desc
    .long 44 ; prototype_desc
    .space 4 ; padding
    .quad _issue04_Zoissue04_ZdFoo_ZdFoo_Z4constructor ; entry
.section __DATA,__data
.p2align 4
.global _issue04_Zoissue04$global_slots
_issue04_Zoissue04$global_slots:
    .quad 0 ; size_in_bytes
    .quad 0 ; slots
    .quad 0 ; mark_size
; string constants:
; Yalx-String constants
.global _issue04_Zoissue04_Lksz
_issue04_Zoissue04_Lksz:
    .long 12
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
.global _issue04_Zoissue04_Kstr
_issue04_Zoissue04_Kstr:
    .long 12
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
