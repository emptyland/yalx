.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
# libc symbols:
.file 1 "tests/25-ir-throw-catch-expr/src/issue00" "foo.yalx"
.p2align 4, 0x90

# functions
.global _issue00_Zoissue00_Zd_Z4init
_issue00_Zoissue00_Zd_Z4init:
.cfi_startproc
Lblk0:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    leaq _yalx_Zplang_Zolang_Zd_Z4init(%rip), %r13
    movq %r13, %rax
    movq %rax, %rdi
    leaq Lkzs.0(%rip), %rsi
    callq _pkg_init_once
    popq %rbp
    retq
.cfi_endproc
.global _issue00_Zoissue00_Zdissue1
_issue00_Zoissue00_Zdissue1:
.cfi_startproc
Lblk1:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $32, %rsp
    movq 104(%r14), %rax
    movq %rax, -16(%rbp)
    leaq _issue00_Zoissue00_Zdissue1(%rip), %rax
    movq %rax, -8(%rbp)
    leaq -16(%rbp), %rax
    movq %rax, 104(%r14)
Lblk2:
    addq $16, %rsp
    callq _issue00_Zoissue00_Zdfoo
    subq $16, %rsp
    testq $-1, 112(%r14)
    jnz Lblk3
    jmp Lblk5
    nop
Lblk3:
    movq 112(%r14), %rax
    movq %rax, -24(%rbp)
    movq %rax, %rdi
    leaq _yalx_Zplang_Zolang_ZdThrowable$class(%rip), %rsi
    callq _is_instance_of
    movq -24(%rbp), %rcx
    cmpb $0, %al
    je Lblk5
Lblk4:
    movq $0, 112(%r14)
    movq %rcx, %rax
    movq 16(%rax), %rcx
    movq %rcx, %rdi
    addq $0, %rsp
    callq _issue00_Zoissue00_Zddisplay
    subq $0, %rsp
    jmp Lblk5
    nop
Lblk5:
    movq 104(%r14), %rax
    movq -16(%rbp), %r13
    movq %r13, 0(%rax)
    addq $32, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue00_Zoissue00_Zdissue1_had
_issue00_Zoissue00_Zdissue1_had:
.cfi_startproc
Lblk6:
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
Lblk7:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $16, %rsp
    leaq _yalx_Zplang_Zolang_ZdException$class(%rip), %rdi
    callq _heap_alloc
    movq $0, %rcx
    movq %rax, -8(%rbp)
    movq -8(%rbp), %rdi
    movq Kstr.1(%rip), %rsi
    movq %rcx, %rdx
    addq $0, %rsp
    callq _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
    subq $0, %rsp
    movq -8(%rbp), %rax
    movq %rax, %rdi
    callq _throw_it
    int3
    addq $16, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _issue00_Zoissue00_Zddisplay
_issue00_Zoissue00_Zddisplay:
.cfi_startproc
Lblk8:
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
Lblk9:
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
# CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "yalx/lang:lang"
Lkzs.1:
    .asciz "ok"
.section __DATA,__data
.p2align 4
# Yalx-String constants
.global _issue00_Zoissue00_Lksz
_issue00_Zoissue00_Lksz:
    .long 2
    .long 0 # padding for struct lksz_header
    .quad Lkzs.0
    .quad Lkzs.1
.global _issue00_Zoissue00_Kstr
_issue00_Zoissue00_Kstr:
    .long 2
    .long 0 # padding for struct kstr_header
Kstr.0:
    .quad 0
Kstr.1:
    .quad 0
