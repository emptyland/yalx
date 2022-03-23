.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
# libc symbols:
.file 1 "tests/41-code-gen-structs/src/issue02" "foo.yalx"
.p2align 4, 0x90

# functions
.global _issue02_Zoissue02_Zd_Z4init
_issue02_Zoissue02_Zd_Z4init:
Lblk5:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    leaq _yalx_Zplang_Zolang_Zd_Z4init(%rip), %r13
    movq %r13, %rax
    movq %rax, %rdi
    leaq Lkzs.1(%rip), %rsi
    callq _pkg_init_once
    addq $0, %rsp
    popq %rbp
    retq
.global _issue02_Zoissue02_Zdissue1
_issue02_Zoissue02_Zdissue1:
Lblk6:
    pushq %rbp
    movq %rsp, %rbp
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
    movl $3, %edi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertInt
    subq $16, %rsp
    movq -40(%rbp), %rax
    movq 24(%rax), %rcx
    movq Kstr.2(%rip), %rdi
    movq %rcx, %rsi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertString
    subq $16, %rsp
    leaq _issue02_Zoissue02_ZdBar$class(%rip), %rdi
    callq _heap_alloc
    movq %rax, -8(%rbp)
    movq -8(%rbp), %rdi
    movl $2, %esi
    movl $3, %edx
    movq Kstr.3(%rip), %rcx
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdBar_ZdBar_Z4constructor
    subq $16, %rsp
    movq -8(%rbp), %rax
    movl 16(%rax), %ecx
    movl 20(%rax), %edx
    movl %ecx, %esi
    addl %edx, %esi
    movq %rax, -8(%rbp)
    movl $5, %edi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertInt
    subq $16, %rsp
    movq -8(%rbp), %rax
    movq 24(%rax), %rcx
    movq Kstr.3(%rip), %rdi
    movq %rcx, %rsi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertString
    subq $16, %rsp
    addq $64, %rsp
    popq %rbp
    retq
.global _issue02_Zoissue02_Zdissue1_had
_issue02_Zoissue02_Zdissue1_had:
Lblk7:
    pushq %rbp
    movq %rsp, %rbp
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
.global _issue02_Zoissue02_Zdissue2
_issue02_Zoissue02_Zdissue2:
Lblk8:
    pushq %rbp
    movq %rsp, %rbp
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
    movq Kstr.2(%rip), %rdi
    movq %rcx, %rsi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertString
    subq $16, %rsp
    movq -40(%rbp), %rax
    movq Kstr.4(%rip), %r13
    movq %r13, 24(%rax)
    movq 24(%rax), %rax
    movq Kstr.4(%rip), %rdi
    movq %rax, %rsi
    addq $16, %rsp
    callq _issue02_Zoissue02_ZdassertString
    subq $16, %rsp
    addq $64, %rsp
    popq %rbp
    retq
.global _issue02_Zoissue02_Zdissue2_had
_issue02_Zoissue02_Zdissue2_had:
Lblk9:
    pushq %rbp
    movq %rsp, %rbp
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
.global _issue02_Zoissue02_Zdissue3
_issue02_Zoissue02_Zdissue3:
Lblk10:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    leaq _yalx_Zplang_Zolang_ZdException$class(%rip), %rdi
    callq _heap_alloc
    movq $0, %rcx
    movq %rax, -8(%rbp)
    movq -8(%rbp), %rdi
    movq Kstr.5(%rip), %rsi
    movq %rcx, %rdx
    addq $0, %rsp
    callq _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
    subq $0, %rsp
    movq -8(%rbp), %rax
    movq 16(%rax), %rcx
    movq Kstr.5(%rip), %rdi
    movq %rcx, %rsi
    addq $0, %rsp
    callq _issue02_Zoissue02_ZdassertString
    subq $0, %rsp
    addq $16, %rsp
    popq %rbp
    retq
.global _issue02_Zoissue02_Zdissue3_had
_issue02_Zoissue02_Zdissue3_had:
Lblk11:
    pushq %rbp
    movq %rsp, %rbp
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
.global _issue02_Zoissue02_Zddisplay
_issue02_Zoissue02_Zddisplay:
Lblk12:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    movq %rdi, -8(%rbp)
    leaq -8(%rbp), %rdi
    callq _yalx_Zplang_Zolang_Zdprintln_stub
    callq _current_root
    movq %rax, %r14
    addq $16, %rsp
    popq %rbp
    retq
.global _issue02_Zoissue02_ZdassertString
_issue02_Zoissue02_ZdassertString:
Lblk13:
    pushq %rbp
    movq %rsp, %rbp
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
.global _issue02_Zoissue02_ZdassertInt
_issue02_Zoissue02_ZdassertInt:
Lblk14:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    callq _assert_int_stub
    callq _current_root
    movq %rax, %r14
    addq $0, %rsp
    popq %rbp
    retq
.global _issue02_Zoissue02_ZdBar_ZdtoString
_issue02_Zoissue02_ZdBar_ZdtoString:
Lblk0:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    movq 24(%rdi), %rax
    movq %rax, 24(%rbp)
    addq $0, %rsp
    popq %rbp
    retq
.global _issue02_Zoissue02_ZdBar_ZdBar_Z4constructor
_issue02_Zoissue02_ZdBar_ZdBar_Z4constructor:
Lblk1:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq %rdi, %rax
    movq %rcx, -8(%rbp)
    movl %edx, -12(%rbp)
    movl %esi, -16(%rbp)
    movq %rdi, -24(%rbp)
    movq %rax, %rdi
    addq $0, %rsp
    callq _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    subq $0, %rsp
    movq -8(%rbp), %rcx
    movl -12(%rbp), %edx
    movl -16(%rbp), %esi
    movq -24(%rbp), %rdi
    movl %esi, 16(%rdi)
    movl %edx, 20(%rdi)
    movq %rcx, 24(%rdi)
    addq $32, %rsp
    popq %rbp
    retq
    addq $32, %rsp
    popq %rbp
    retq
.global _issue02_Zoissue02_ZdFoo_ZddoIt
_issue02_Zoissue02_ZdFoo_ZddoIt:
Lblk2:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    movq 24(%rdi), %rax
    movq %rax, %rdi
    addq $0, %rsp
    callq _issue02_Zoissue02_Zddisplay
    subq $0, %rsp
    addq $0, %rsp
    popq %rbp
    retq
    addq $0, %rsp
    popq %rbp
    retq
.global _issue02_Zoissue02_ZdFoo_ZddoThat
_issue02_Zoissue02_ZdFoo_ZddoThat:
Lblk3:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    movq %rdi, -8(%rbp)
    movq Kstr.0(%rip), %rdi
    addq $0, %rsp
    callq _issue02_Zoissue02_Zddisplay
    subq $0, %rsp
    movq -8(%rbp), %rax
    addq $16, %rsp
    popq %rbp
    retq
    addq $16, %rsp
    popq %rbp
    retq
.global _issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor
_issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor:
Lblk4:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    movl %esi, 16(%rdi)
    movl %edx, 20(%rdi)
    movq %rcx, 24(%rdi)
    addq $0, %rsp
    popq %rbp
    retq
    addq $0, %rsp
    popq %rbp
    retq
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
    .byte 0 # padding
    .byte 0
    .byte 0
    .long 8 # reference_size
    .long 32 # instance_size
    .long 0 # padding
    .quad _issue02_Zoissue02_ZdBar$class # super
    .quad Lkzs.12 # name
    .long 3 # name
    .long 0 # padding
    .quad Lkzs.13 # location
    .long 19 # location
    .long 0 # padding
    .long 0 # n_annotations
    .long 0 # padding
    .quad 0 # reserved0
    .long 3 # n_fields
    .long 0 # padding
    .quad _issue02_Zoissue02_ZdBar$fields # fields
    .quad _issue02_Zoissue02_ZdBar$ctor # ctor
    .long 2 # n_methods
    .long 0 # padding
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
    .long 0 # padding
    .quad 0 # type
    .long 16 # offset_of_head
    .long 0 # padding
    # Bar::y
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.11 # name
    .long 1 # name
    .long 0 # padding
    .quad 0 # type
    .long 20 # offset_of_head
    .long 0 # padding
    # Bar::name
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.2 # name
    .long 4 # name
    .long 0 # padding
    .quad 0 # type
    .long 24 # offset_of_head
    .long 0 # padding
_issue02_Zoissue02_ZdBar$methods:
    # Bar::toString
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .long 0 # padding
    .quad 0 # reserved0
    .quad Lkzs.7 # name
    .long 8 # name
    .long 0 # padding
    .quad Lkzs.6 # prototype_desc
    .long 35 # prototype_desc
    .long 0 # padding
    .quad _issue02_Zoissue02_ZdBar_ZdtoString # entry
_issue02_Zoissue02_ZdBar$ctor:
    # Bar::Bar$constructor
    .long 1 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .long 0 # padding
    .quad 0 # reserved0
    .quad Lkzs.9 # name
    .long 15 # name
    .long 0 # padding
    .quad Lkzs.8 # prototype_desc
    .long 48 # prototype_desc
    .long 0 # padding
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
    .byte 0 # padding
    .byte 0
    .byte 0
    .long 8 # reference_size
    .long 32 # instance_size
    .long 0 # padding
    .quad 0 # super
    .quad Lkzs.18 # name
    .long 3 # name
    .long 0 # padding
    .quad Lkzs.19 # location
    .long 19 # location
    .long 0 # padding
    .long 0 # n_annotations
    .long 0 # padding
    .quad 0 # reserved0
    .long 3 # n_fields
    .long 0 # padding
    .quad _issue02_Zoissue02_ZdFoo$fields # fields
    .quad _issue02_Zoissue02_ZdFoo$ctor # ctor
    .long 3 # n_methods
    .long 0 # padding
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
    .long 0 # padding
    .quad 0 # type
    .long 16 # offset_of_head
    .long 0 # padding
    # Foo::y
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.11 # name
    .long 1 # name
    .long 0 # padding
    .quad 0 # type
    .long 20 # offset_of_head
    .long 0 # padding
    # Foo::name
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.2 # name
    .long 4 # name
    .long 0 # padding
    .quad 0 # type
    .long 24 # offset_of_head
    .long 0 # padding
_issue02_Zoissue02_ZdFoo$methods:
    # Foo::doIt
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .long 0 # padding
    .quad 0 # reserved0
    .quad Lkzs.15 # name
    .long 4 # name
    .long 0 # padding
    .quad Lkzs.14 # prototype_desc
    .long 33 # prototype_desc
    .long 0 # padding
    .quad _issue02_Zoissue02_ZdFoo_ZddoIt # entry
    # Foo::doThat
    .long 1 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .long 0 # padding
    .quad 0 # reserved0
    .quad Lkzs.0 # name
    .long 6 # name
    .long 0 # padding
    .quad Lkzs.14 # prototype_desc
    .long 33 # prototype_desc
    .long 0 # padding
    .quad _issue02_Zoissue02_ZdFoo_ZddoThat # entry
_issue02_Zoissue02_ZdFoo$ctor:
    # Foo::Foo$constructor
    .long 2 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .long 0 # padding
    .quad 0 # reserved0
    .quad Lkzs.17 # name
    .long 15 # name
    .long 0 # padding
    .quad Lkzs.16 # prototype_desc
    .long 48 # prototype_desc
    .long 0 # padding
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
