.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
# libc symbols:
.file 1 "tests/40-code-gen-sanity/src/main" "main.yalx"
.p2align 4, 0x90

# functions
.global _main_Zomain_Zd_Z4init
_main_Zomain_Zd_Z4init:
Lblk0:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    leaq _yalx_Zplang_Zolang_Zd_Z4init(%rip), %r13
    movq %r13, %rax
    movq %rax, %rdi
    leaq Lkzs.0(%rip), %rsi
    callq _pkg_init_once
    addq $0, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdmain
_main_Zomain_Zdmain:
Lblk1:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    movq Kstr.1(%rip), %rdi
    addq $0, %rsp
    callq _main_Zomain_Zddisplay
    subq $0, %rsp
    addq $0, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdmain_had
_main_Zomain_Zdmain_had:
Lblk2:
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
    callq _main_Zomain_Zdmain
    movq -8(%rbp), %rbx
    movq -16(%rbp), %r12
    movq -24(%rbp), %r13
    movq -32(%rbp), %r14
    movq -40(%rbp), %r15
    addq $48, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue1
_main_Zomain_Zdissue1:
Lblk3:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    addq $0, %rsp
    callq _main_Zomain_Zdfoo
    subq $0, %rsp
    movl -4(%rbp), %eax
    addl -8(%rbp), %eax
    movl %eax, 28(%rbp)
    addq $16, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue2
_main_Zomain_Zdissue2:
Lblk4:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    movl $1, %edi
    movl $2, %esi
    movl $3, %edx
    addq $0, %rsp
    callq _main_Zomain_Zddoo
    subq $0, %rsp
    movl -4(%rbp), %r13d
    movl %r13d, 28(%rbp)
    addq $16, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue3
_main_Zomain_Zdissue3:
Lblk5:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    movl %edi, %eax
    addl %esi, %eax
    movl %esi, %edi
    movl %edi, %esi
    movl %eax, %edx
    addq $0, %rsp
    callq _main_Zomain_Zddoo
    subq $0, %rsp
    movl -4(%rbp), %r13d
    movl %r13d, 28(%rbp)
    addq $16, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue4
_main_Zomain_Zdissue4:
Lblk6:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    movl %edi, %eax
    addl %esi, %eax
    movl %eax, %edx
    addq $0, %rsp
    callq _main_Zomain_Zddoo
    subq $0, %rsp
    movl -4(%rbp), %r13d
    movl %r13d, 28(%rbp)
    addq $16, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue5
_main_Zomain_Zdissue5:
Lblk7:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    movl $4, 16(%rbp)
    movl $3, 20(%rbp)
    movl $2, 24(%rbp)
    movl $1, 28(%rbp)
    addq $0, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue6
_main_Zomain_Zdissue6:
Lblk8:
    pushq %rbp
    movq %rsp, %rbp
    subq $48, %rsp
    cmpl %esi, %edi
    jle Lblk10
Lblk9:
    movl %eax, -4(%rbp)
    movl %esi, -8(%rbp)
    movl %edi, -12(%rbp)
    addq $16, %rsp
    callq _main_Zomain_Zdissue5
    subq $16, %rsp
    movl -4(%rbp), %eax
    movl -8(%rbp), %ecx
    movl -12(%rbp), %edx
    movl -20(%rbp), %esi
    addl %edx, %esi
    movl %esi, %edx
    addl %ecx, %edx
    movl %edx, %eax
    jmp Lblk11
    nop
Lblk10:
    movl $-1, %eax
    jmp Lblk11
    nop
Lblk11:
    movl %eax, 28(%rbp)
    addq $48, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue6_had
_main_Zomain_Zdissue6_had:
Lblk12:
    pushq %rbp
    movq %rsp, %rbp
    subq $64, %rsp
    movq %rbx, -8(%rbp)
    movq %r12, -16(%rbp)
    movq %r13, -24(%rbp)
    movq %r14, -32(%rbp)
    movq %r15, -40(%rbp)
    movl %edi, -44(%rbp)
    movl %esi, -48(%rbp)
    callq _current_root
    movq %rax, %r14
    movl -44(%rbp), %edi
    movl -48(%rbp), %esi
    callq _main_Zomain_Zdissue6
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
    addq $64, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue7
_main_Zomain_Zdissue7:
Lblk13:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    ucomiss %xmm0, %xmm1
    jge Lblk15
Lblk14:
    movl $-1, %eax
    jmp Lblk19
    nop
Lblk15:
    ucomiss %xmm0, %xmm1
    jle Lblk17
Lblk16:
    movl $1, %ecx
    jmp Lblk18
    nop
Lblk17:
    movl $0, %ecx
    jmp Lblk18
    nop
Lblk18:
    movl %ecx, %eax
    jmp Lblk19
    nop
Lblk19:
    movl %eax, 28(%rbp)
    addq $0, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue8
_main_Zomain_Zdissue8:
Lblk20:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    cmpl %esi, %edi
    setl %al
    movb %al, 28(%rbp)
    addq $0, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue9
_main_Zomain_Zdissue9:
Lblk21:
    pushq %rbp
    movq %rsp, %rbp
    subq $96, %rsp
    movl %eax, -52(%rbp)
    movq %rcx, -60(%rbp)
    movq %rbp, %rdi
    subq $48, %rdi
    movq %rbp, %rsi
    addq $16, %rsi
    movq $16, %rdx
    leaq _main_Zomain_Zdissue9(%rip), %rcx
    callq _associate_stub_returning_vals
    movl -52(%rbp), %eax
    leaq -60(%rbp), %rcx
    callq _issue9_stub
    movq %rbp, %rdi
    subq $48, %rdi
    callq _yalx_exit_returning_scope
    callq _current_root
    movq %rax, %r14
    addq $96, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue10
_main_Zomain_Zdissue10:
Lblk22:
    pushq %rbp
    movq %rsp, %rbp
    subq $96, %rsp
    movq %rbp, %rdi
    subq $48, %rdi
    movq %rbp, %rsi
    addq $16, %rsi
    movq $32, %rdx
    leaq _main_Zomain_Zdissue10(%rip), %rcx
    callq _associate_stub_returning_vals
    callq _issue10_stub
    movq %rbp, %rdi
    subq $48, %rdi
    callq _yalx_exit_returning_scope
    callq _current_root
    movq %rax, %r14
    addq $96, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue11
_main_Zomain_Zdissue11:
Lblk23:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    addq $0, %rsp
    callq _main_Zomain_Zdissue10
    subq $0, %rsp
    movl $1, %edi
    movl -4(%rbp), %esi
    addq $0, %rsp
    callq _main_Zomain_ZdassertInt
    subq $0, %rsp
    movl $2, %edi
    movl -8(%rbp), %esi
    addq $0, %rsp
    callq _main_Zomain_ZdassertInt
    subq $0, %rsp
    movl $3, %edi
    movl -12(%rbp), %esi
    addq $0, %rsp
    callq _main_Zomain_ZdassertInt
    subq $0, %rsp
    movq Kstr.2(%rip), %rdi
    movq -20(%rbp), %rsi
    addq $0, %rsp
    callq _main_Zomain_ZdassertString
    subq $0, %rsp
    addq $32, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue11_had
_main_Zomain_Zdissue11_had:
Lblk24:
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
    callq _main_Zomain_Zdissue11
    movq -8(%rbp), %rbx
    movq -16(%rbp), %r12
    movq -24(%rbp), %r13
    movq -32(%rbp), %r14
    movq -40(%rbp), %r15
    addq $48, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdfoo
_main_Zomain_Zdfoo:
Lblk25:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    movss Knnn.0(%rip), %xmm13
    movss %xmm13, 20(%rbp)
    movl $2, 24(%rbp)
    movl $1, 28(%rbp)
    addq $0, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zddoo
_main_Zomain_Zddoo:
Lblk26:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    movl %edi, %eax
    addl %esi, %eax
    movl %eax, %ecx
    addl %edx, %ecx
    movl %ecx, 28(%rbp)
    addq $0, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zddisplay
_main_Zomain_Zddisplay:
Lblk27:
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
.global _main_Zomain_ZdassertString
_main_Zomain_ZdassertString:
Lblk28:
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
.global _main_Zomain_ZdassertInt
_main_Zomain_ZdassertInt:
Lblk29:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    callq _assert_int_stub
    callq _current_root
    movq %rax, %r14
    addq $0, %rsp
    popq %rbp
    retq
# CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "yalx/lang:lang"
Lkzs.1:
    .asciz "Hello, World"
Lkzs.2:
    .asciz "hello"
.section __TEXT,__const
.p2align 4
# Number constants
Knnn.0:
    .long 0x3f8ccccd    # float.1.100000
.section __DATA,__data
.p2align 4
# Yalx-String constants
.global _main_Zomain_Lksz
_main_Zomain_Lksz:
    .long 3
    .long 0 # padding for struct lksz_header
    .quad Lkzs.0
    .quad Lkzs.1
    .quad Lkzs.2
.global _main_Zomain_Kstr
_main_Zomain_Kstr:
    .long 3
    .long 0 # padding for struct kstr_header
Kstr.0:
    .quad 0
Kstr.1:
    .quad 0
Kstr.2:
    .quad 0
