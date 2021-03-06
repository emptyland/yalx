.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
# libc symbols:
.file 1 "tests/42-code-gen-globals/src/issue03" "foo.yalx"
.p2align 4, 0x90

# functions
.global _issue03_Zoissue03_Zd_Z4init
_issue03_Zoissue03_Zd_Z4init:
.cfi_startproc
Lblk3:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $80, %rsp
    leaq _yalx_Zplang_Zolang_Zd_Z4init(%rip), %r13
    movq %r13, %rax
    movq %rax, -8(%rbp)
    movq %rax, %rdi
    leaq Lkzs.1(%rip), %rsi
    callq _pkg_init_once
    movq $0, _issue03_Zoissue03_ZdBaz(%rip)
    movl $1, _issue03_Zoissue03_Zda(%rip)
    movl $2, _issue03_Zoissue03_Zdb(%rip)
    leaq -40(%rbp), %rax
    movq %rax, -8(%rbp)
    movq %rsi, -48(%rbp)
    movq %rdi, -56(%rbp)
    movq -8(%rbp), %rdi
    movq Kstr.2(%rip), %rsi
    movq Kstr.3(%rip), %rdx
    addq $16, %rsp
    callq _issue03_Zoissue03_ZdBar_ZdBar_Z4constructor
    subq $16, %rsp
    movq -40(%rbp), %r13
    movq %r13, _issue03_Zoissue03_Zdbar(%rip)
    movq -32(%rbp), %r13
    movq %r13, _issue03_Zoissue03_Zdbar+8(%rip)
    movq -24(%rbp), %r13
    movq %r13, _issue03_Zoissue03_Zdbar+16(%rip)
    movq -16(%rbp), %r13
    movq %r13, _issue03_Zoissue03_Zdbar+24(%rip)
    leaq _issue03_Zoissue03_ZdFoo$class(%rip), %rdi
    callq _heap_alloc
    movq %rax, -8(%rbp)
    movq -8(%rbp), %rdi
    movl $1, %esi
    movl $2, %edx
    movq Kstr.4(%rip), %rcx
    addq $16, %rsp
    callq _issue03_Zoissue03_ZdFoo_ZdFoo_Z4constructor
    subq $16, %rsp
    movq -8(%rbp), %rax
    movq %rax, _issue03_Zoissue03_Zdfoo(%rip)
    addq $80, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue03_Zoissue03_Zdissue1
_issue03_Zoissue03_Zdissue1:
.cfi_startproc
Lblk4:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $16, %rsp
    leaq _issue03_Zoissue03_ZdBaz(%rip), %rdi
    leaq _issue03_Zoissue03_ZdBaz_Z4ShadowClass$class(%rip), %rsi
    callq _lazy_load_object
    movl 16(%rax), %ecx
    movq %rax, -8(%rbp)
    movl %ecx, -12(%rbp)
    leaq _issue03_Zoissue03_ZdBaz(%rip), %rdi
    leaq _issue03_Zoissue03_ZdBaz_Z4ShadowClass$class(%rip), %rsi
    callq _lazy_load_object
    movl -12(%rbp), %ecx
    movl 20(%rax), %edx
    movl %edx, 24(%rbp)
    movl %ecx, 28(%rbp)
    addq $16, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue03_Zoissue03_Zdissue1_had
_issue03_Zoissue03_Zdissue1_had:
.cfi_startproc
Lblk5:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $48, %rsp
    movq %rbx, -8(%rbp)
    movq %r12, -16(%rbp)
    movq %r13, -24(%rbp)
    movq %r14, -32(%rbp)
    movq %r15, -40(%rbp)
    callq _current_root
    movq %rax, %r14
    callq _issue03_Zoissue03_Zdissue1
    movq $16, %rdi
    callq _reserve_handle_returning_vals
    movq %rax, %rdi
    movq %rsp, %rsi
    movq $16, %rdx
    callq _memcpy
    movq -8(%rbp), %rbx
    movq -16(%rbp), %r12
    movq -24(%rbp), %r13
    movq -32(%rbp), %r14
    movq -40(%rbp), %r15
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue03_Zoissue03_Zddisplay
_issue03_Zoissue03_Zddisplay:
.cfi_startproc
Lblk6:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $16, %rsp
    movq %rdi, -8(%rbp)
    leaq -8(%rbp), %rdi
    callq _yalx_Zplang_Zolang_Zdprintln_stub
    callq _current_root
    movq %rax, %r14
    addq $16, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue03_Zoissue03_ZdFoo_ZdFoo_Z4constructor
_issue03_Zoissue03_ZdFoo_ZdFoo_Z4constructor:
.cfi_startproc
Lblk0:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $48, %rsp
    movq %rdi, %rax
    movq %rax, -8(%rbp)
    movq %rcx, -16(%rbp)
    movl %edx, -20(%rbp)
    movl %esi, -24(%rbp)
    movq %rdi, -32(%rbp)
    movq -8(%rbp), %rdi
    addq $16, %rsp
    callq _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    subq $16, %rsp
    movq -16(%rbp), %rax
    movl -20(%rbp), %ecx
    movl -24(%rbp), %edx
    movq -32(%rbp), %rsi
    movl %edx, 16(%rsi)
    movl %ecx, 20(%rsi)
    movq %rax, -8(%rbp)
    movl %ecx, -12(%rbp)
    movl %edx, -16(%rbp)
    movq %rsi, -24(%rbp)
    leaq 24(%rsi), %rdi
    movq %rax, %rsi
    callq _put_field
    addq $48, %rsp
    popq %rbp
    retq
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue03_Zoissue03_ZdBaz_Z4ShadowClass_ZdBaz_Z4ShadowClass_Z4constructor
_issue03_Zoissue03_ZdBaz_Z4ShadowClass_ZdBaz_Z4ShadowClass_Z4constructor:
.cfi_startproc
Lblk1:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $32, %rsp
    movq %rdi, %rax
    movq %rax, -8(%rbp)
    movq %rdi, -16(%rbp)
    movq -8(%rbp), %rdi
    addq $16, %rsp
    callq _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    subq $16, %rsp
    movq -16(%rbp), %rax
    movl $996, 16(%rax)
    movl $777, 20(%rax)
    movq %rax, -8(%rbp)
    leaq 24(%rax), %rdi
    movq Kstr.0(%rip), %rsi
    callq _put_field
    addq $32, %rsp
    popq %rbp
    retq
    addq $32, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue03_Zoissue03_ZdBar_ZdBar_Z4constructor
_issue03_Zoissue03_ZdBar_ZdBar_Z4constructor:
.cfi_startproc
Lblk2:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movq %rsi, 16(%rdi)
    movq %rdx, 24(%rdi)
    popq %rbp
    retq
    popq %rbp
    retq
.cfi_endproc
# CString constants
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
# classes:
.global _issue03_Zoissue03_ZdFoo$class
_issue03_Zoissue03_ZdFoo$class:
    .quad 0 # id
    .byte 0 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 32 # instance_size
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny$class # super
    .quad Lkzs.10 # name
    .long 3 # name
    .space 4 # padding
    .quad Lkzs.11 # location
    .long 19 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 3 # n_fields
    .space 4 # padding
    .quad _issue03_Zoissue03_ZdFoo$fields # fields
    .quad _issue03_Zoissue03_ZdFoo$ctor # ctor
    .long 1 # n_methods
    .space 4 # padding
    .quad _issue03_Zoissue03_ZdFoo$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_issue03_Zoissue03_ZdFoo$fields:
    # Foo::i
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.7 # name
    .long 1 # name
    .space 4 # padding
    .quad _builtin_classes+864 # type
    .long 16 # offset_of_head
    .space 4 # padding
    # Foo::j
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.8 # name
    .long 1 # name
    .space 4 # padding
    .quad _builtin_classes+864 # type
    .long 20 # offset_of_head
    .space 4 # padding
    # Foo::s
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.9 # name
    .long 1 # name
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString$class # type
    .long 24 # offset_of_head
    .space 4 # padding
_issue03_Zoissue03_ZdFoo$methods:
_issue03_Zoissue03_ZdFoo$ctor:
    # Foo::Foo$constructor
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.6 # name
    .long 15 # name
    .space 4 # padding
    .quad Lkzs.5 # prototype_desc
    .long 48 # prototype_desc
    .space 4 # padding
    .quad _issue03_Zoissue03_ZdFoo_ZdFoo_Z4constructor # entry
.global _issue03_Zoissue03_ZdBaz_Z4ShadowClass$class
_issue03_Zoissue03_ZdBaz_Z4ShadowClass$class:
    .quad 0 # id
    .byte 0 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 32 # instance_size
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny$class # super
    .quad Lkzs.14 # name
    .long 15 # name
    .space 4 # padding
    .quad Lkzs.15 # location
    .long 31 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 3 # n_fields
    .space 4 # padding
    .quad _issue03_Zoissue03_ZdBaz_Z4ShadowClass$fields # fields
    .quad _issue03_Zoissue03_ZdBaz_Z4ShadowClass$ctor # ctor
    .long 1 # n_methods
    .space 4 # padding
    .quad _issue03_Zoissue03_ZdBaz_Z4ShadowClass$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_issue03_Zoissue03_ZdBaz_Z4ShadowClass$fields:
    # Baz$ShadowClass::i
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.7 # name
    .long 1 # name
    .space 4 # padding
    .quad _builtin_classes+864 # type
    .long 16 # offset_of_head
    .space 4 # padding
    # Baz$ShadowClass::j
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.8 # name
    .long 1 # name
    .space 4 # padding
    .quad _builtin_classes+864 # type
    .long 20 # offset_of_head
    .space 4 # padding
    # Baz$ShadowClass::s
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.9 # name
    .long 1 # name
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString$class # type
    .long 24 # offset_of_head
    .space 4 # padding
_issue03_Zoissue03_ZdBaz_Z4ShadowClass$methods:
_issue03_Zoissue03_ZdBaz_Z4ShadowClass$ctor:
    # Baz$ShadowClass::Baz$ShadowClass$constructor
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.13 # name
    .long 27 # name
    .space 4 # padding
    .quad Lkzs.12 # prototype_desc
    .long 45 # prototype_desc
    .space 4 # padding
    .quad _issue03_Zoissue03_ZdBaz_Z4ShadowClass_ZdBaz_Z4ShadowClass_Z4constructor # entry
.global _issue03_Zoissue03_ZdBar$class
_issue03_Zoissue03_ZdBar$class:
    .quad 0 # id
    .byte 1 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 32 # instance_size
    .space 4 # padding
    .quad 0 # super
    .quad Lkzs.20 # name
    .long 3 # name
    .space 4 # padding
    .quad Lkzs.21 # location
    .long 19 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 2 # n_fields
    .space 4 # padding
    .quad _issue03_Zoissue03_ZdBar$fields # fields
    .quad _issue03_Zoissue03_ZdBar$ctor # ctor
    .long 1 # n_methods
    .space 4 # padding
    .quad _issue03_Zoissue03_ZdBar$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_issue03_Zoissue03_ZdBar$fields:
    # Bar::firstName
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.18 # name
    .long 9 # name
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString$class # type
    .long 16 # offset_of_head
    .space 4 # padding
    # Bar::lastName
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.19 # name
    .long 8 # name
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString$class # type
    .long 24 # offset_of_head
    .space 4 # padding
_issue03_Zoissue03_ZdBar$methods:
_issue03_Zoissue03_ZdBar$ctor:
    # Bar::Bar$constructor
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.17 # name
    .long 15 # name
    .space 4 # padding
    .quad Lkzs.16 # prototype_desc
    .long 47 # prototype_desc
    .space 4 # padding
    .quad _issue03_Zoissue03_ZdBar_ZdBar_Z4constructor # entry
.section __DATA,__data
.p2align 4
# global slots:
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
    .quad 56 # size_in_bytes
    .quad pkg_global_slots # slots
    .quad 4 # mark_size
    .long 0
    .long 32
    .long 40
    .long 48
# string constants:
# Yalx-String constants
.global _issue03_Zoissue03_Lksz
_issue03_Zoissue03_Lksz:
    .long 22
    .long 0 # padding for struct lksz_header
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
    .long 0 # padding for struct kstr_header
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
