.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
# libc symbols:
.file 1 "tests/41-code-gen-structs/src/issue02" "foo.yalx"
.p2align 4, 0x90

# functions
.global _issue02_Zoissue02_Zd_Z4init
_issue02_Zoissue02_Zd_Z4init:
Lblk3:
    pushq %rbp
    movq %rsp, %rbp
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
.global _issue02_Zoissue02_Zdissue1
_issue02_Zoissue02_Zdissue1:
Lblk4:
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
    movq 24(%rax), %rcx
    movq %rax, -8(%rbp)
    movq %rcx, -16(%rbp)
    movl %edx, -20(%rbp)
    movl %esi, -24(%rbp)
    movq -16(%rbp), %rdi
    addq $16, %rsp
    callq _issue02_Zoissue02_Zddisplay
    subq $16, %rsp
    movq -8(%rbp), %rax
    movl -20(%rbp), %ecx
    movl -24(%rbp), %edx
    addq $64, %rsp
    popq %rbp
    retq
.global _issue02_Zoissue02_Zdissue1_had
_issue02_Zoissue02_Zdissue1_had:
Lblk5:
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
.global _issue02_Zoissue02_Zddisplay
_issue02_Zoissue02_Zddisplay:
Lblk6:
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
.global _issue02_Zoissue02_ZdFoo_ZddoIt
_issue02_Zoissue02_ZdFoo_ZddoIt:
Lblk0:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq 24(%rdi), %rax
    movq %rax, -8(%rbp)
    movq %rdi, -16(%rbp)
    movq -8(%rbp), %rdi
    addq $16, %rsp
    callq _issue02_Zoissue02_Zddisplay
    subq $16, %rsp
    movq -16(%rbp), %rax
    addq $32, %rsp
    popq %rbp
    retq
    addq $32, %rsp
    popq %rbp
    retq
.global _issue02_Zoissue02_ZdFoo_ZddoThat
_issue02_Zoissue02_ZdFoo_ZddoThat:
Lblk1:
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
Lblk2:
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
_yalx_Zplang_Zolang_Zd_Z4init:
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
    .asciz "fun (issue02:issue02.Foo)->(void)"
Lkzs.4:
    .asciz "doIt"
Lkzs.5:
    .asciz "fun (issue02:issue02.Foo,i32,i32,string)->(void)"
Lkzs.6:
    .asciz "Foo$constructor"
Lkzs.7:
    .asciz "x"
Lkzs.8:
    .asciz "y"
Lkzs.9:
    .asciz "Foo"
Lkzs.10:
    .asciz "issue02:issue02.Foo"
.section __DATA,__data
.p2align 4
# classes:
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
    .quad Lkzs.9 # name
    .long 3 # name
    .long 0 # padding
    .quad Lkzs.10 # location
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
    .quad Lkzs.7 # name
    .long 1 # name
    .long 0 # padding
    .quad 0 # type
    .long 16 # offset_of_head
    .long 0 # padding
    # Foo::y
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.8 # name
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
    .quad Lkzs.4 # name
    .long 4 # name
    .long 0 # padding
    .quad Lkzs.3 # prototype_desc
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
    .quad Lkzs.3 # prototype_desc
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
    .quad Lkzs.6 # name
    .long 15 # name
    .long 0 # padding
    .quad Lkzs.5 # prototype_desc
    .long 48 # prototype_desc
    .long 0 # padding
    .quad _issue02_Zoissue02_ZdFoo_ZdFoo_Z4constructor # entry
.section __DATA,__data
.p2align 4
# Yalx-String constants
.global _issue02_Zoissue02_Lksz
_issue02_Zoissue02_Lksz:
    .long 11
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
.global _issue02_Zoissue02_Kstr
_issue02_Zoissue02_Kstr:
    .long 11
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
