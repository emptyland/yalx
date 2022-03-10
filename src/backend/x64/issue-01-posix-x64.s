.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
# libc symbols:
.global _memcpy,_memset
.file 1 "tests/40-code-gen-sanity/src/main" "main.yalx"
.p2align 4, 0x90

# functions
.global _main_Zomain_Zd_Z4init
_main_Zomain_Zd_Z4init:
Lblk0:
    pushq %rbp
    movq %rsp, %rbp
    subq $48, %rsp
    leaq _yalx_Zplang_Zolang_Zd_Z4init(%rip), %r13
    movq %r13, %rax
    movq %rax, -40(%rbp)
    movq %rax, %rdi
    leaq Lkzs.0(%rip), %rsi
    callq _pkg_init_once
    addq $48, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdmain
_main_Zomain_Zdmain:
Lblk1:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq Kstr.1(%rip), %rdi
    addq $0, %rsp
    callq _main_Zomain_Zddisplay
    subq $0, %rsp
    addq $32, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdmain_had
_main_Zomain_Zdmain_had:
Lblk2:
    pushq %rbp
    movq %rsp, %rbp
    subq $80, %rsp
    movq %rbx, -40(%rbp)
    movq %r12, -48(%rbp)
    movq %r13, -56(%rbp)
    movq %r14, -64(%rbp)
    movq %r15, -72(%rbp)
    callq _current_root
    movq %rax, %r14
    callq _main_Zomain_Zdmain
    movq -40(%rbp), %rbx
    movq -48(%rbp), %r12
    movq -56(%rbp), %r13
    movq -64(%rbp), %r14
    movq -72(%rbp), %r15
    addq $80, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue1
_main_Zomain_Zdissue1:
Lblk3:
    pushq %rbp
    movq %rsp, %rbp
    subq $48, %rsp
    addq $0, %rsp
    callq _main_Zomain_Zdfoo
    subq $0, %rsp
    movl -36(%rbp), %eax
    addl -40(%rbp), %eax
    movl %eax, 28(%rbp)
    addq $48, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue2
_main_Zomain_Zdissue2:
Lblk4:
    pushq %rbp
    movq %rsp, %rbp
    subq $48, %rsp
    movl $1, %edi
    movl $2, %esi
    movl $3, %edx
    addq $0, %rsp
    callq _main_Zomain_Zddoo
    subq $0, %rsp
    movl -36(%rbp), %r13d
    movl %r13d, 28(%rbp)
    addq $48, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue3
_main_Zomain_Zdissue3:
Lblk5:
    pushq %rbp
    movq %rsp, %rbp
    subq $64, %rsp
    movl %edi, %eax
    addl %esi, %eax
    movl %eax, -36(%rbp)
    movl %esi, -40(%rbp)
    movl %edi, -44(%rbp)
    movl -40(%rbp), %edi
    movl -44(%rbp), %esi
    movl -36(%rbp), %edx
    addq $16, %rsp
    callq _main_Zomain_Zddoo
    subq $16, %rsp
    movl -48(%rbp), %r13d
    movl %r13d, 28(%rbp)
    addq $64, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue4
_main_Zomain_Zdissue4:
Lblk6:
    pushq %rbp
    movq %rsp, %rbp
    subq $48, %rsp
    movl %edi, %eax
    addl %esi, %eax
    movl %eax, -36(%rbp)
    movl -36(%rbp), %edx
    addq $0, %rsp
    callq _main_Zomain_Zddoo
    subq $0, %rsp
    movl -40(%rbp), %r13d
    movl %r13d, 28(%rbp)
    addq $48, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue5
_main_Zomain_Zdissue5:
Lblk7:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movl $4, 16(%rbp)
    movl $3, 20(%rbp)
    movl $2, 24(%rbp)
    movl $1, 28(%rbp)
    addq $32, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue6
_main_Zomain_Zdissue6:
Lblk8:
    pushq %rbp
    movq %rsp, %rbp
    subq $64, %rsp
    cmpl %edi, %esi
    jle Lblk10
Lblk9:
    movl %eax, -36(%rbp)
    movl %esi, -40(%rbp)
    movl %edi, -44(%rbp)
    addq $0, %rsp
    callq _main_Zomain_Zdissue5
    subq $0, %rsp
    movl -36(%rbp), %eax
    movl -40(%rbp), %ecx
    movl -44(%rbp), %edx
    movl -48(%rbp), %esi
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
    addq $64, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue6_had
_main_Zomain_Zdissue6_had:
Lblk12:
    pushq %rbp
    movq %rsp, %rbp
    subq $96, %rsp
    movq %rbx, -40(%rbp)
    movq %r12, -48(%rbp)
    movq %r13, -56(%rbp)
    movq %r14, -64(%rbp)
    movq %r15, -72(%rbp)
    movl %edi, -76(%rbp)
    movl %esi, -80(%rbp)
    callq _current_root
    movq %rax, %r14
    movl -76(%rbp), %edi
    movl -80(%rbp), %esi
    callq _main_Zomain_Zdissue6
    movq $16, %rdi
    callq _reserve_handle_returning_vals
    movq %rax, %rdi
    movq %rsp, %rsi
    movq $16, %rdx
    callq _memcpy
    movq -40(%rbp), %rbx
    movq -48(%rbp), %r12
    movq -56(%rbp), %r13
    movq -64(%rbp), %r14
    movq -72(%rbp), %r15
    addq $96, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue7
_main_Zomain_Zdissue7:
Lblk13:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
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
    addq $32, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue8
_main_Zomain_Zdissue8:
Lblk20:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    cmpl %edi, %esi
    setl %al
    movb %al, 28(%rbp)
    addq $32, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue9
_main_Zomain_Zdissue9:
Lblk21:
    pushq %rbp
    movq %rsp, %rbp
    subq $128, %rsp
    movl %eax, -84(%rbp)
    movq %rcx, -92(%rbp)
    movq %rbp, %rdi
    subq $80, %rdi
    movq %rbp, %rsi
    addq $16, %rsi
    movq $16, %rdx
    leaq _main_Zomain_Zdissue9(%rip), %rcx
    callq _associate_stub_returning_vals
    movl -84(%rbp), %eax
    leaq -92(%rbp), %rcx
    callq _issue9_stub
    movq %rbp, %rdi
    subq $80, %rdi
    callq _yalx_exit_returning_scope
    callq _current_root
    movq %rax, %r14
    addq $128, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdfoo
_main_Zomain_Zdfoo:
Lblk22:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movss Knnn.0(%rip), %xmm13
    movss %xmm13, 20(%rbp)
    movl $2, 24(%rbp)
    movl $1, 28(%rbp)
    addq $32, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zddoo
_main_Zomain_Zddoo:
Lblk23:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movl %edi, %eax
    addl %esi, %eax
    movl %eax, %ecx
    addl %edx, %ecx
    movl %ecx, 28(%rbp)
    addq $32, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zddisplay
_main_Zomain_Zddisplay:
Lblk24:
    pushq %rbp
    movq %rsp, %rbp
    subq $48, %rsp
    movq %rdi, -40(%rbp)
    leaq -40(%rbp), %rdi
    callq _yalx_Zplang_Zolang_Zdprintln_stub
    callq _current_root
    movq %rax, %r14
    addq $48, %rsp
    popq %rbp
    retq
_yalx_Zplang_Zolang_Zd_Z4init:
    retq
# CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "yalx/lang:lang"
Lkzs.1:
    .asciz "Hello, World"
# Number constants
.section __TEXT,__const
.p2align 4
Knnn.0:
    .long 0x3f8ccccd    # float.1.100000
.section __DATA,__data
.p2align 4
# Yalx-String constants
.global _main_Zomain_Lksz
_main_Zomain_Lksz:
    .long 2
    .long 0 # padding for struct lksz_header
    .quad Lkzs.0
    .quad Lkzs.1
.global _main_Zomain_Kstr
_main_Zomain_Kstr:
    .long 2
    .long 0 # padding for struct kstr_header
Kstr.0:
    .quad 0
Kstr.1:
    .quad 0
