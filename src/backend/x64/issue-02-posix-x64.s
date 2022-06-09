.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
# libc symbols:
.file 1 "tests/41-code-gen-structs/src/issue02" "foo.yalx"
.p2align 4, 0x90

# functions
.global _issue02_Zoissue02_Zd_Z4init
_issue02_Zoissue02_Zd_Z4init:
.cfi_startproc
Lblk5:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $16, %rsp
    leaq _yalx_Zplang_Zolang_Zd_Z4init(%rip), %r13
    movq %r13, %rax
    movq %rax, -8(%rbp)
    movq %rax, %rdi
    leaq Lkzs.1(%rip), %rsi
    callq _pkg_init_once
    addq $16, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue02_Zoissue02_Zdissue1
_issue02_Zoissue02_Zdissue1:
.cfi_startproc
Lblk6:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $64, %rsp
    leaq -32(%rbp), %rax
    movq %rax, -40(%rbp)
    movq -40(%rbp), %rdi
    movl $1, %esi
    movl $2, %edx
    movq Kstr.2(%rip), %rcx
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor
    subq $16, %rsp
    movq -40(%rbp), %rax
    movl 16(%rax), %ecx
    movl 20(%rax), %edx
    movl %ecx, %esi
    addl %edx, %esi
    movq %rax, -40(%rbp)
    movl %ecx, -44(%rbp)
    movl %edx, -48(%rbp)
    movl $3, %edi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertInt
    subq $16, %rsp
    movq -40(%rbp), %rax
    movq 24(%rax), %rcx
    movq %rax, -8(%rbp)
    movq %rcx, -16(%rbp)
    movl %esi, -20(%rbp)
    movq Kstr.2(%rip), %rdi
    movq -16(%rbp), %rsi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertString
    subq $16, %rsp
    leaq _issue02_Zoissue02_ZdBar$class(%rip), %rdi
    callq _heap_alloc
    movq %rax, -16(%rbp)
    movq -16(%rbp), %rdi
    movl $2, %esi
    movl $3, %edx
    movq Kstr.3(%rip), %rcx
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdBar_ZdBar_Z4constructor
    subq $16, %rsp
    movq -16(%rbp), %rax
    movl 16(%rax), %ecx
    movl 20(%rax), %edx
    movl %ecx, %esi
    addl %edx, %esi
    movq %rax, -16(%rbp)
    movl %ecx, -24(%rbp)
    movl %edx, -28(%rbp)
    movl $5, %edi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertInt
    subq $16, %rsp
    movq -16(%rbp), %rax
    movq 24(%rax), %rcx
    movq %rax, -16(%rbp)
    movq %rcx, -36(%rbp)
    movl %esi, -40(%rbp)
    movq Kstr.3(%rip), %rdi
    movq -36(%rbp), %rsi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertString
    subq $16, %rsp
    addq $64, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue02_Zoissue02_Zdissue1_had
_issue02_Zoissue02_Zdissue1_had:
.cfi_startproc
Lblk7:
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
    callq _issue02_Zoissue02_Zdissue1
    movq -8(%rbp), %rbx
    movq -16(%rbp), %r12
    movq -24(%rbp), %r13
    movq -32(%rbp), %r14
    movq -40(%rbp), %r15
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue02_Zoissue02_Zdissue2
_issue02_Zoissue02_Zdissue2:
.cfi_startproc
Lblk8:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $64, %rsp
    leaq -32(%rbp), %rax
    movq %rax, -40(%rbp)
    movq -40(%rbp), %rdi
    movl $1, %esi
    movl $2, %edx
    movq Kstr.2(%rip), %rcx
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor
    subq $16, %rsp
    movq -40(%rbp), %rax
    movq 24(%rax), %rcx
    movq %rax, -40(%rbp)
    movq %rcx, -48(%rbp)
    movq Kstr.2(%rip), %rdi
    movq -48(%rbp), %rsi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertString
    subq $16, %rsp
    movq -40(%rbp), %rax
    movq Kstr.4(%rip), %r13
    movq %r13, 24(%rax)
    movq 24(%rax), %rax
    movq %rax, -8(%rbp)
    movq Kstr.4(%rip), %rdi
    movq -8(%rbp), %rsi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertString
    subq $16, %rsp
    addq $64, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue02_Zoissue02_Zdissue2_had
_issue02_Zoissue02_Zdissue2_had:
.cfi_startproc
Lblk9:
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
    callq _issue02_Zoissue02_Zdissue2
    movq -8(%rbp), %rbx
    movq -16(%rbp), %r12
    movq -24(%rbp), %r13
    movq -32(%rbp), %r14
    movq -40(%rbp), %r15
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue02_Zoissue02_Zdissue3
_issue02_Zoissue02_Zdissue3:
.cfi_startproc
Lblk10:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $32, %rsp
    leaq _yalx_Zplang_Zolang_ZdException$class(%rip), %rdi
    callq _heap_alloc
    movq %rax, -8(%rbp)
    movq $0, %rdi
    leaq _yalx_Zplang_Zolang_ZdException$class(%rip), %rsi
    callq _ref_asserted_to
    movq -8(%rbp), %rcx
    movq %rax, -8(%rbp)
    movq %rcx, -16(%rbp)
    movq -16(%rbp), %rdi
    movq Kstr.5(%rip), %rsi
    movq -8(%rbp), %rdx
    addq $16, %rsp
    callq _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
    subq $16, %rsp
    movq -16(%rbp), %rax
    movq 16(%rax), %rcx
    movq %rax, -8(%rbp)
    movq %rcx, -16(%rbp)
    movq Kstr.5(%rip), %rdi
    movq -16(%rbp), %rsi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertString
    subq $16, %rsp
    addq $32, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue02_Zoissue02_Zdissue3_had
_issue02_Zoissue02_Zdissue3_had:
.cfi_startproc
Lblk11:
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
    callq _issue02_Zoissue02_Zdissue3
    movq -8(%rbp), %rbx
    movq -16(%rbp), %r12
    movq -24(%rbp), %r13
    movq -32(%rbp), %r14
    movq -40(%rbp), %r15
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue02_Zoissue02_Zddisplay
_issue02_Zoissue02_Zddisplay:
.cfi_startproc
Lblk12:
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
.global _issue02_Zoissue02_ZdassertString
_issue02_Zoissue02_ZdassertString:
.cfi_startproc
Lblk13:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $32, %rsp
    movq %rdi, -8(%rbp)
    movq %rsi, -16(%rbp)
    leaq -8(%rbp), %rdi
    leaq -16(%rbp), %rsi
    callq _assert_string_stub
    callq _current_root
    movq %rax, %r14
    addq $32, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue02_Zoissue02_ZdassertInt
_issue02_Zoissue02_ZdassertInt:
.cfi_startproc
Lblk14:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    callq _assert_int_stub
    callq _current_root
    movq %rax, %r14
    popq %rbp
    retq
.cfi_endproc
.global _issue02_Zoissue02_ZdBar_ZdtoString
_issue02_Zoissue02_ZdBar_ZdtoString:
.cfi_startproc
Lblk0:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movq 24(%rdi), %rax
    movq %rax, 24(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _issue02_Zoissue02_ZdBar_ZdBar_Z4constructor
_issue02_Zoissue02_ZdBar_ZdBar_Z4constructor:
.cfi_startproc
Lblk1:
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
.global _issue02_Zoissue02_ZdFoo_ZddoIt
_issue02_Zoissue02_ZdFoo_ZddoIt:
.cfi_startproc
Lblk2:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $32, %rsp
    movq 24(%rdi), %rax
    movq %rax, -8(%rbp)
    movq %rdi, -16(%rbp)
    movq -8(%rbp), %rdi
    addq $16, %rsp
    callq _issue02_Zoissue02_Zddisplay
    subq $16, %rsp
    addq $32, %rsp
    popq %rbp
    retq
    addq $32, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue02_Zoissue02_ZdFoo_ZddoThat
_issue02_Zoissue02_ZdFoo_ZddoThat:
.cfi_startproc
Lblk3:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $16, %rsp
    movq %rdi, -8(%rbp)
    movq Kstr.0(%rip), %rdi
    addq $0, %rsp
    callq _issue02_Zoissue02_Zddisplay
    subq $0, %rsp
    addq $16, %rsp
    popq %rbp
    retq
    addq $16, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor
_issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor:
.cfi_startproc
Lblk4:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl %esi, 16(%rdi)
    movl %edx, 20(%rdi)
    movq %rcx, 24(%rdi)
    popq %rbp
    retq
    popq %rbp
    retq
.cfi_endproc
# CString constants
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
# classes:
.global _issue02_Zoissue02_ZdBar$class
_issue02_Zoissue02_ZdBar$class:
    .quad 0 # id
    .byte 0 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 32 # instance_size
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny$class # super
    .quad Lkzs.12 # name
    .long 3 # name
    .space 4 # padding
    .quad Lkzs.13 # location
    .long 19 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 3 # n_fields
    .space 4 # padding
    .quad _issue02_Zoissue02_ZdBar$fields # fields
    .quad _issue02_Zoissue02_ZdBar$ctor # ctor
    .long 2 # n_methods
    .space 4 # padding
    .quad _issue02_Zoissue02_ZdBar$methods # methods
    .long 5 # n_vtab
    .long 0 # n_itab
    .quad _issue02_Zoissue02_ZdBar$vtab # vtab
    .quad 0 # itab
_issue02_Zoissue02_ZdBar$fields:
    # Bar::x
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.10 # name
    .long 1 # name
    .space 4 # padding
    .quad _builtin_classes+864 # type
    .long 16 # offset_of_head
    .space 4 # padding
    # Bar::y
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.11 # name
    .long 1 # name
    .space 4 # padding
    .quad _builtin_classes+864 # type
    .long 20 # offset_of_head
    .space 4 # padding
    # Bar::name
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.2 # name
    .long 4 # name
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString$class # type
    .long 24 # offset_of_head
    .space 4 # padding
_issue02_Zoissue02_ZdBar$methods:
    # Bar::toString
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.7 # name
    .long 8 # name
    .space 4 # padding
    .quad Lkzs.6 # prototype_desc
    .long 35 # prototype_desc
    .space 4 # padding
    .quad _issue02_Zoissue02_ZdBar_ZdtoString # entry
_issue02_Zoissue02_ZdBar$ctor:
    # Bar::Bar$constructor
    .long 1 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.9 # name
    .long 15 # name
    .space 4 # padding
    .quad Lkzs.8 # prototype_desc
    .long 48 # prototype_desc
    .space 4 # padding
    .quad _issue02_Zoissue02_ZdBar_ZdBar_Z4constructor # entry
_issue02_Zoissue02_ZdBar$vtab:
    .quad _yalx_Zplang_Zolang_ZdAny_Zdfinalize
    .quad _yalx_Zplang_Zolang_ZdAny_ZdhashCode
    .quad _yalx_Zplang_Zolang_ZdAny_Zdid
    .quad _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
    .quad _issue02_Zoissue02_ZdBar_ZdtoString
.global _issue02_Zoissue02_ZdFoo$class
_issue02_Zoissue02_ZdFoo$class:
    .quad 0 # id
    .byte 1 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 32 # instance_size
    .space 4 # padding
    .quad 0 # super
    .quad Lkzs.18 # name
    .long 3 # name
    .space 4 # padding
    .quad Lkzs.19 # location
    .long 19 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 3 # n_fields
    .space 4 # padding
    .quad _issue02_Zoissue02_ZdFoo$fields # fields
    .quad _issue02_Zoissue02_ZdFoo$ctor # ctor
    .long 3 # n_methods
    .space 4 # padding
    .quad _issue02_Zoissue02_ZdFoo$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_issue02_Zoissue02_ZdFoo$fields:
    # Foo::x
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.10 # name
    .long 1 # name
    .space 4 # padding
    .quad _builtin_classes+864 # type
    .long 16 # offset_of_head
    .space 4 # padding
    # Foo::y
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.11 # name
    .long 1 # name
    .space 4 # padding
    .quad _builtin_classes+864 # type
    .long 20 # offset_of_head
    .space 4 # padding
    # Foo::name
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.2 # name
    .long 4 # name
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString$class # type
    .long 24 # offset_of_head
    .space 4 # padding
_issue02_Zoissue02_ZdFoo$methods:
    # Foo::doIt
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.15 # name
    .long 4 # name
    .space 4 # padding
    .quad Lkzs.14 # prototype_desc
    .long 33 # prototype_desc
    .space 4 # padding
    .quad _issue02_Zoissue02_ZdFoo_ZddoIt # entry
    # Foo::doThat
    .long 1 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.0 # name
    .long 6 # name
    .space 4 # padding
    .quad Lkzs.14 # prototype_desc
    .long 33 # prototype_desc
    .space 4 # padding
    .quad _issue02_Zoissue02_ZdFoo_ZddoThat # entry
_issue02_Zoissue02_ZdFoo$ctor:
    # Foo::Foo$constructor
    .long 2 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.17 # name
    .long 15 # name
    .space 4 # padding
    .quad Lkzs.16 # prototype_desc
    .long 48 # prototype_desc
    .space 4 # padding
    .quad _issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor # entry
.section __DATA,__data
.p2align 4
# Yalx-String constants
.global _issue02_Zoissue02_Lksz
_issue02_Zoissue02_Lksz:
    .long 20
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
.global _issue02_Zoissue02_Kstr
_issue02_Zoissue02_Kstr:
    .long 20
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
