.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
# libc symbols:
.file 1 "tests/25-ir-throw-catch-expr/src/issue00" "foo.yalx"
.p2align 4, 0x90

# functions
.global _issue00_Zoissue00_Zd_Z4init
_issue00_Zoissue00_Zd_Z4init:
.cfi_startproc
Lblk1:
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
    leaq Lkzs.0(%rip), %rsi
    callq _pkg_init_once
    addq $16, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue00_Zoissue00_Zdissue1
_issue00_Zoissue00_Zdissue1:
.cfi_startproc
Lblk2:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $48, %rsp
    movq 104(%r14), %rax
    movq %rax, -16(%rbp)
    leaq _issue00_Zoissue00_Zdissue1(%rip), %rax
    movq %rax, -8(%rbp)
    leaq -16(%rbp), %rax
    movq %rax, 104(%r14)
Lblk3:
    addq $16, %rsp
    callq _issue00_Zoissue00_Zdbar
    subq $16, %rsp
    testq $-1, 112(%r14)
    jnz Lblk4
    addq $16, %rsp
    callq _issue00_Zoissue00_Zdfoo
    subq $16, %rsp
    testq $-1, 112(%r14)
    jnz Lblk4
    movq Kstr.1(%rip), %rdi
    movq Kstr.2(%rip), %rsi
    addq $16, %rsp
    callq _issue00_Zoissue00_ZdassertString
    subq $16, %rsp
    testq $-1, 112(%r14)
    jnz Lblk4
    jmp Lblk6
    nop
Lblk4:
    movq 112(%r14), %rax
    movq %rax, -24(%rbp)
    movq %rax, %rdi
    leaq _yalx_Zplang_Zolang_ZdThrowable$class(%rip), %rsi
    callq _is_instance_of
    movq -24(%rbp), %rcx
    cmpb $0, %al
    je Lblk6
Lblk5:
    movq $0, 112(%r14)
    movb %al, -20(%rbp)
    movq %rcx, -32(%rbp)
    movq -32(%rbp), %rdi
    leaq _yalx_Zplang_Zolang_ZdException$class(%rip), %rsi
    callq _ref_asserted_to
    movq %rax, %rcx
    movq %rax, -28(%rbp)
    movq %rcx, -40(%rbp)
    movq -40(%rbp), %rdi
    addq $0, %rsp
    callq _yalx_Zplang_Zolang_ZdThrowable_ZdprintBacktrace
    subq $0, %rsp
    jmp Lblk6
    nop
Lblk6:
    movq 104(%r14), %rax
    movq -16(%rbp), %r13
    movq %r13, 0(%rax)
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue00_Zoissue00_Zdissue1_had
_issue00_Zoissue00_Zdissue1_had:
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
    callq _issue00_Zoissue00_Zdissue1
    movq -8(%rbp), %rbx
    movq -16(%rbp), %r12
    movq -24(%rbp), %r13
    movq -32(%rbp), %r14
    movq -40(%rbp), %r15
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue00_Zoissue00_Zdfoo
_issue00_Zoissue00_Zdfoo:
.cfi_startproc
Lblk8:
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
    movq Kstr.3(%rip), %rsi
    movq -8(%rbp), %rdx
    addq $16, %rsp
    callq _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
    subq $16, %rsp
    movq -16(%rbp), %rax
    movq %rax, -8(%rbp)
    movq %rax, %rdi
    callq _throw_it
    int3
    addq $32, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue00_Zoissue00_Zdbar
_issue00_Zoissue00_Zdbar:
.cfi_startproc
Lblk9:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $16, %rsp
    leaq _issue00_Zoissue00_ZdBar$class(%rip), %rdi
    callq _heap_alloc
    movq %rax, -8(%rbp)
    movq -8(%rbp), %rdi
    movl $1, %esi
    movl $2, %edx
    addq $0, %rsp
    callq _issue00_Zoissue00_ZdBar_ZdBar_Z4constructor
    subq $0, %rsp
    movq -8(%rbp), %rax
    movq %rax, -8(%rbp)
    movq -8(%rbp), %rdi
    leaq _yalx_Zplang_Zolang_ZdException$class(%rip), %rsi
    callq _ref_asserted_to
    movq %rax, 24(%rbp)
    addq $16, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue00_Zoissue00_Zddisplay
_issue00_Zoissue00_Zddisplay:
.cfi_startproc
Lblk10:
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
.global _issue00_Zoissue00_ZdassertString
_issue00_Zoissue00_ZdassertString:
.cfi_startproc
Lblk11:
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
.global _issue00_Zoissue00_ZdBar_ZdBar_Z4constructor
_issue00_Zoissue00_ZdBar_ZdBar_Z4constructor:
.cfi_startproc
Lblk0:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $32, %rsp
    movq %rdi, %rax
    movq %rax, -8(%rbp)
    movl %edx, -12(%rbp)
    movl %esi, -16(%rbp)
    movq %rdi, -24(%rbp)
    movq -8(%rbp), %rdi
    addq $0, %rsp
    callq _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    subq $0, %rsp
    movl -12(%rbp), %eax
    movl -16(%rbp), %ecx
    movq -24(%rbp), %rdx
    movl %ecx, 16(%rdx)
    movl %eax, 20(%rdx)
    addq $32, %rsp
    popq %rbp
    retq
    addq $32, %rsp
    popq %rbp
    retq
.cfi_endproc
# CString constants
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
# classes:
.global _issue00_Zoissue00_ZdBar$class
_issue00_Zoissue00_ZdBar$class:
    .quad 0 # id
    .byte 0 # constraint
    .byte 0 # padding
    .byte 0
    .byte 0
    .long 8 # reference_size
    .long 24 # instance_size
    .long 0 # padding
    .quad _yalx_Zplang_Zolang_ZdAny$class # super
    .quad Lkzs.8 # name
    .long 3 # name
    .long 0 # padding
    .quad Lkzs.9 # location
    .long 19 # location
    .long 0 # padding
    .long 0 # n_annotations
    .long 0 # padding
    .quad 0 # reserved0
    .long 2 # n_fields
    .long 0 # padding
    .quad _issue00_Zoissue00_ZdBar$fields # fields
    .quad _issue00_Zoissue00_ZdBar$ctor # ctor
    .long 1 # n_methods
    .long 0 # padding
    .quad _issue00_Zoissue00_ZdBar$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_issue00_Zoissue00_ZdBar$fields:
    # Bar::x
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.6 # name
    .long 1 # name
    .long 0 # padding
    .quad 0 # type
    .long 16 # offset_of_head
    .long 0 # padding
    # Bar::y
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.7 # name
    .long 1 # name
    .long 0 # padding
    .quad 0 # type
    .long 20 # offset_of_head
    .long 0 # padding
_issue00_Zoissue00_ZdBar$methods:
_issue00_Zoissue00_ZdBar$ctor:
    # Bar::Bar$constructor
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .long 0 # padding
    .quad 0 # reserved0
    .quad Lkzs.5 # name
    .long 15 # name
    .long 0 # padding
    .quad Lkzs.4 # prototype_desc
    .long 41 # prototype_desc
    .long 0 # padding
    .quad _issue00_Zoissue00_ZdBar_ZdBar_Z4constructor # entry
.section __DATA,__data
.p2align 4
# Yalx-String constants
.global _issue00_Zoissue00_Lksz
_issue00_Zoissue00_Lksz:
    .long 10
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
.global _issue00_Zoissue00_Kstr
_issue00_Zoissue00_Kstr:
    .long 10
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
