.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
# libc symbols:
.global _memcpy,_memset
.file 1 "tests/40-code-gen-sanity/src/main" "main.yalx"
# External symbols:
.global _yalx_Zplang_Zolang_Zd_Z4init
.p2align 4, 0x90

# functions
.global _main_Zomain_Zd_Z4init
_main_Zomain_Zd_Z4init:
Lblk0:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq _yalx_Zplang_Zolang_Zd_Z4init(%rip), %rax
    addq $32, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdmain
_main_Zomain_Zdmain:
Lblk1:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    addq $32, %rsp
    popq %rbp
    retq
.global _main_Zomain_Zdissue1
_main_Zomain_Zdissue1:
Lblk2:
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
Lblk3:
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
Lblk4:
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
Lblk5:
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
Lblk6:
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
.global _main_Zomain_Zdfoo
_main_Zomain_Zdfoo:
Lblk7:
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
Lblk8:
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
_yalx_Zplang_Zolang_Zd_Z4init:
    retq
# Number constants
.section __TEXT,__literal8,8byte_literals
.p2align 4
Knnn.0:
    .long 0x3f8ccccd    # float.1.100000
.section __DATA,__data
.p2align 4