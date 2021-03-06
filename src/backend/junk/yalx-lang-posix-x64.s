.section __TEXT,__text,regular,pure_instructions
.build_version macos, 11, 0 sdk_version 12, 1
# libc symbols:
.file 1 "libs/yalx/lang" "any.yalx"
.file 2 "libs/yalx/lang" "throwing.yalx"
.file 3 "libs/yalx/lang" "number.yalx"
.file 4 "libs/yalx/lang" "string.yalx"
.p2align 4, 0x90

# functions
.global _yalx_Zplang_Zolang_Zd_Z4init
_yalx_Zplang_Zolang_Zd_Z4init:
.cfi_startproc
Lblk40:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_Zdprintln
_yalx_Zplang_Zolang_Zdprintln:
.cfi_startproc
Lblk41:
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
.global _yalx_Zplang_Zolang_Zdunwind
_yalx_Zplang_Zolang_Zdunwind:
.cfi_startproc
Lblk42:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $96, %rsp
    movq %rbp, %rdi
    subq $48, %rdi
    movq %rbp, %rsi
    addq $16, %rsi
    movq $16, %rdx
    leaq _yalx_Zplang_Zolang_Zdunwind(%rip), %rcx
    callq _associate_stub_returning_vals
    callq _yalx_Zplang_Zolang_Zdunwind_stub
    movq %rbp, %rdi
    subq $48, %rdi
    callq _yalx_exit_returning_scope
    callq _current_root
    movq %rax, %r14
    addq $96, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdAny_Zdid
_yalx_Zplang_Zolang_ZdAny_Zdid:
.cfi_startproc
Lblk0:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $96, %rsp
    movq %rax, -56(%rbp)
    movq %rbp, %rdi
    subq $48, %rdi
    movq %rbp, %rsi
    addq $16, %rsi
    movq $16, %rdx
    leaq _yalx_Zplang_Zolang_ZdAny_Zdid(%rip), %rcx
    callq _associate_stub_returning_vals
    leaq -56(%rbp), %rax
    callq _yalx_Zplang_Zolang_ZdAny_Zdid_stub
    movq %rbp, %rdi
    subq $48, %rdi
    callq _yalx_exit_returning_scope
    callq _current_root
    movq %rax, %r14
    addq $96, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdAny_ZdhashCode
_yalx_Zplang_Zolang_ZdAny_ZdhashCode:
.cfi_startproc
Lblk1:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $96, %rsp
    movq %rax, -56(%rbp)
    movq %rbp, %rdi
    subq $48, %rdi
    movq %rbp, %rsi
    addq $16, %rsi
    movq $16, %rdx
    leaq _yalx_Zplang_Zolang_ZdAny_ZdhashCode(%rip), %rcx
    callq _associate_stub_returning_vals
    leaq -56(%rbp), %rax
    callq _yalx_Zplang_Zolang_ZdAny_ZdhashCode_stub
    movq %rbp, %rdi
    subq $48, %rdi
    callq _yalx_exit_returning_scope
    callq _current_root
    movq %rax, %r14
    addq $96, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdAny_ZdtoString
_yalx_Zplang_Zolang_ZdAny_ZdtoString:
.cfi_startproc
Lblk2:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $96, %rsp
    movq %rax, -56(%rbp)
    movq %rbp, %rdi
    subq $48, %rdi
    movq %rbp, %rsi
    addq $16, %rsi
    movq $16, %rdx
    leaq _yalx_Zplang_Zolang_ZdAny_ZdtoString(%rip), %rcx
    callq _associate_stub_returning_vals
    leaq -56(%rbp), %rax
    callq _yalx_Zplang_Zolang_ZdAny_ZdtoString_stub
    movq %rbp, %rdi
    subq $48, %rdi
    callq _yalx_exit_returning_scope
    callq _current_root
    movq %rax, %r14
    addq $96, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
_yalx_Zplang_Zolang_ZdAny_ZdisEmpty:
.cfi_startproc
Lblk3:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $96, %rsp
    movq %rax, -56(%rbp)
    movq %rbp, %rdi
    subq $48, %rdi
    movq %rbp, %rsi
    addq $16, %rsi
    movq $16, %rdx
    leaq _yalx_Zplang_Zolang_ZdAny_ZdisEmpty(%rip), %rcx
    callq _associate_stub_returning_vals
    leaq -56(%rbp), %rax
    callq _yalx_Zplang_Zolang_ZdAny_ZdisEmpty_stub
    movq %rbp, %rdi
    subq $48, %rdi
    callq _yalx_exit_returning_scope
    callq _current_root
    movq %rax, %r14
    addq $96, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdAny_Zdfinalize
_yalx_Zplang_Zolang_ZdAny_Zdfinalize:
.cfi_startproc
Lblk4:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    popq %rbp
    retq
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
_yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor:
.cfi_startproc
Lblk5:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    popq %rbp
    retq
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdString_ZdString_Z4constructor
_yalx_Zplang_Zolang_ZdString_ZdString_Z4constructor:
.cfi_startproc
Lblk6:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $48, %rsp
    movl %ecx, -4(%rbp)
    movl %edx, -8(%rbp)
    movl %esi, -12(%rbp)
    movq %rdi, -20(%rbp)
    movq %r8, -28(%rbp)
    movq -20(%rbp), %rdi
    addq $16, %rsp
    callq _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    subq $16, %rsp
    movl -4(%rbp), %eax
    movl -8(%rbp), %ecx
    movl -12(%rbp), %edx
    movq -20(%rbp), %rsi
    movq -28(%rbp), %rdi
    movl %edx, 16(%rsi)
    movl %ecx, 20(%rsi)
    movl %eax, 24(%rsi)
    movq %rdi, 32(%rsi)
    addq $48, %rsp
    popq %rbp
    retq
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdI32_ZdtoString
_yalx_Zplang_Zolang_ZdI32_ZdtoString:
.cfi_startproc
Lblk7:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $96, %rsp
    movq %rax, -56(%rbp)
    movq %rbp, %rdi
    subq $48, %rdi
    movq %rbp, %rsi
    addq $16, %rsi
    movq $16, %rdx
    leaq _yalx_Zplang_Zolang_ZdI32_ZdtoString(%rip), %rcx
    callq _associate_stub_returning_vals
    movq -56(%rbp), %rax
    callq _yalx_Zplang_Zolang_ZdI32_ZdtoString_stub
    movq %rbp, %rdi
    subq $48, %rdi
    callq _yalx_exit_returning_scope
    callq _current_root
    movq %rax, %r14
    addq $96, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdI32_ZdI32_Z4constructor
_yalx_Zplang_Zolang_ZdI32_ZdI32_Z4constructor:
.cfi_startproc
Lblk8:
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
    callq _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdNumber_Dki32_Dl_Z4constructor
    subq $16, %rsp
    addq $32, %rsp
    popq %rbp
    retq
    addq $32, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdU32_ZdtoString
_yalx_Zplang_Zolang_ZdU32_ZdtoString:
.cfi_startproc
Lblk9:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $96, %rsp
    movq %rax, -56(%rbp)
    movq %rbp, %rdi
    subq $48, %rdi
    movq %rbp, %rsi
    addq $16, %rsi
    movq $16, %rdx
    leaq _yalx_Zplang_Zolang_ZdU32_ZdtoString(%rip), %rcx
    callq _associate_stub_returning_vals
    movq -56(%rbp), %rax
    callq _yalx_Zplang_Zolang_ZdU32_ZdtoString_stub
    movq %rbp, %rdi
    subq $48, %rdi
    callq _yalx_exit_returning_scope
    callq _current_root
    movq %rax, %r14
    addq $96, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdU32_ZdU32_Z4constructor
_yalx_Zplang_Zolang_ZdU32_ZdU32_Z4constructor:
.cfi_startproc
Lblk10:
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
    callq _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdNumber_Dku32_Dl_Z4constructor
    subq $16, %rsp
    addq $32, %rsp
    popq %rbp
    retq
    addq $32, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI8
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI8:
.cfi_startproc
Lblk11:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movb %al, %cl
    andl $255, %ecx
    movb %cl, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU8
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU8:
.cfi_startproc
Lblk12:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movb %al, %cl
    andl $255, %ecx
    movb %cl, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI16
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI16:
.cfi_startproc
Lblk13:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movw %ax, %cx
    andl $65535, %ecx
    movw %cx, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU16
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU16:
.cfi_startproc
Lblk14:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movw %ax, %cx
    andl $65535, %ecx
    movw %cx, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI32
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI32:
.cfi_startproc
Lblk15:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movl %eax, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU32
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU32:
.cfi_startproc
Lblk16:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movl %eax, %ecx
    movl %ecx, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI64
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI64:
.cfi_startproc
Lblk17:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movslq %eax, %rcx
    movq %rcx, 24(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU64
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU64:
.cfi_startproc
Lblk18:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movl %eax, %ecx
    movq %rcx, 24(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoF32
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoF32:
.cfi_startproc
Lblk19:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movss %xmm0, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoF64
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoF64:
.cfi_startproc
Lblk20:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movsd %xmm0, 24(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdNumber_Dki32_Dl_Z4constructor
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdNumber_Dki32_Dl_Z4constructor:
.cfi_startproc
Lblk21:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl %esi, 16(%rdi)
    popq %rbp
    retq
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI8
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI8:
.cfi_startproc
Lblk22:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movb %al, %cl
    andl $255, %ecx
    movb %cl, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU8
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU8:
.cfi_startproc
Lblk23:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movb %al, %cl
    andl $255, %ecx
    movb %cl, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI16
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI16:
.cfi_startproc
Lblk24:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movw %ax, %cx
    andl $65535, %ecx
    movw %cx, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU16
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU16:
.cfi_startproc
Lblk25:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movw %ax, %cx
    andl $65535, %ecx
    movw %cx, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI32
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI32:
.cfi_startproc
Lblk26:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movl %eax, %ecx
    movl %ecx, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU32
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU32:
.cfi_startproc
Lblk27:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movl %eax, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI64
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI64:
.cfi_startproc
Lblk28:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movl %eax, %ecx
    movq %rcx, 24(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU64
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU64:
.cfi_startproc
Lblk29:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movl %eax, %ecx
    movq %rcx, 24(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoF32
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoF32:
.cfi_startproc
Lblk30:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movss %xmm0, 28(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoF64
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoF64:
.cfi_startproc
Lblk31:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl 16(%rdi), %eax
    movsd %xmm0, 24(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdNumber_Dku32_Dl_Z4constructor
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdNumber_Dku32_Dl_Z4constructor:
.cfi_startproc
Lblk32:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movl %esi, 16(%rdi)
    popq %rbp
    retq
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdThrowable_ZdtoString
_yalx_Zplang_Zolang_ZdThrowable_ZdtoString:
.cfi_startproc
Lblk33:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movq 16(%rdi), %rax
    movq %rax, 24(%rbp)
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdThrowable_ZdprintBacktrace
_yalx_Zplang_Zolang_ZdThrowable_ZdprintBacktrace:
.cfi_startproc
Lblk34:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $16, %rsp
    movq %rdi, -8(%rbp)
    leaq -8(%rbp), %rdi
    callq _yalx_Zplang_Zolang_ZdThrowable_ZdprintBacktrace_stub
    callq _current_root
    movq %rax, %r14
    addq $16, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdThrowable_ZdThrowable_Z4constructor
_yalx_Zplang_Zolang_ZdThrowable_ZdThrowable_Z4constructor:
.cfi_startproc
Lblk35:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $48, %rsp
    movq %rdi, %rax
    movq %rax, -8(%rbp)
    movq %rdx, -16(%rbp)
    movq %rsi, -24(%rbp)
    movq %rdi, -32(%rbp)
    movq -8(%rbp), %rdi
    addq $16, %rsp
    callq _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    subq $16, %rsp
    movq -16(%rbp), %rax
    movq -24(%rbp), %rcx
    movq -32(%rbp), %rdx
    movq %rcx, 16(%rdx)
    movq %rax, 24(%rdx)
    movq %rax, -8(%rbp)
    movq %rcx, -16(%rbp)
    movq %rdx, -24(%rbp)
    addq $0, %rsp
    callq _yalx_Zplang_Zolang_Zdunwind
    subq $0, %rsp
    movq -24(%rbp), %rax
    movq -40(%rbp), %r13
    movq %r13, 32(%rax)
    addq $48, %rsp
    popq %rbp
    retq
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
_yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor:
.cfi_startproc
Lblk36:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $48, %rsp
    movq %rdi, %rax
    movq %rdx, %rcx
    movq %rax, -8(%rbp)
    movq %rcx, -16(%rbp)
    movq %rdx, -24(%rbp)
    movq %rdi, -32(%rbp)
    movq -8(%rbp), %rdi
    movq -16(%rbp), %rdx
    addq $16, %rsp
    callq _yalx_Zplang_Zolang_ZdThrowable_ZdThrowable_Z4constructor
    subq $16, %rsp
    addq $48, %rsp
    popq %rbp
    retq
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdBadCastingException_ZdBadCastingException_Z4constructor
_yalx_Zplang_Zolang_ZdBadCastingException_ZdBadCastingException_Z4constructor:
.cfi_startproc
Lblk37:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $48, %rsp
    movq %rdi, %rax
    movq %rax, -8(%rbp)
    movq %rsi, -16(%rbp)
    movq %rdi, -24(%rbp)
    movq $0, %rdi
    leaq _yalx_Zplang_Zolang_ZdException$class(%rip), %rsi
    callq _ref_asserted_to
    movq -8(%rbp), %rcx
    movq -16(%rbp), %rdx
    movq %rax, -8(%rbp)
    movq %rcx, -16(%rbp)
    movq %rdx, -32(%rbp)
    movq -16(%rbp), %rdi
    movq -32(%rbp), %rsi
    movq -8(%rbp), %rdx
    addq $16, %rsp
    callq _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
    subq $16, %rsp
    addq $48, %rsp
    popq %rbp
    retq
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException_ZdArrayIndexOutOfBoundsException_Z4constructor
_yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException_ZdArrayIndexOutOfBoundsException_Z4constructor:
.cfi_startproc
Lblk38:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $48, %rsp
    movq %rdi, %rax
    movq %rax, -8(%rbp)
    movq %rsi, -16(%rbp)
    movq %rdi, -24(%rbp)
    movq $0, %rdi
    leaq _yalx_Zplang_Zolang_ZdException$class(%rip), %rsi
    callq _ref_asserted_to
    movq -8(%rbp), %rcx
    movq -16(%rbp), %rdx
    movq %rax, -8(%rbp)
    movq %rcx, -16(%rbp)
    movq %rdx, -32(%rbp)
    movq -16(%rbp), %rdi
    movq -32(%rbp), %rsi
    movq -8(%rbp), %rdx
    addq $16, %rsp
    callq _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor
    subq $16, %rsp
    addq $48, %rsp
    popq %rbp
    retq
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
.global _yalx_Zplang_Zolang_ZdBacktraceFrame_ZdBacktraceFrame_Z4constructor
_yalx_Zplang_Zolang_ZdBacktraceFrame_ZdBacktraceFrame_Z4constructor:
.cfi_startproc
Lblk39:
    pushq %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq $48, %rsp
    movq %rdi, %rax
    movq %rax, -8(%rbp)
    movq %rcx, -16(%rbp)
    movq %rdx, -24(%rbp)
    movq %rsi, -32(%rbp)
    movq %rdi, -40(%rbp)
    movl %r8d, -44(%rbp)
    movq -8(%rbp), %rdi
    addq $0, %rsp
    callq _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
    subq $0, %rsp
    movq -16(%rbp), %rax
    movq -24(%rbp), %rcx
    movq -32(%rbp), %rdx
    movq -40(%rbp), %rsi
    movl -44(%rbp), %edi
    movq %rdx, 16(%rsi)
    movq %rcx, 24(%rsi)
    movq %rax, 32(%rsi)
    movl %edi, 40(%rsi)
    addq $48, %rsp
    popq %rbp
    retq
    addq $48, %rsp
    popq %rbp
    retq
.cfi_endproc
# CString constants
.section __TEXT,__cstring,cstring_literals
Lkzs.0:
    .asciz "fun (yalx/lang:lang.Any)->(i32)"
Lkzs.1:
    .asciz "id"
Lkzs.2:
    .asciz "fun (yalx/lang:lang.Any)->(u32)"
Lkzs.3:
    .asciz "hashCode"
Lkzs.4:
    .asciz "fun (yalx/lang:lang.Any)->(string)"
Lkzs.5:
    .asciz "toString"
Lkzs.6:
    .asciz "fun (yalx/lang:lang.Any)->(u8)"
Lkzs.7:
    .asciz "isEmpty"
Lkzs.8:
    .asciz "fun (yalx/lang:lang.Any)->(void)"
Lkzs.9:
    .asciz "finalize"
Lkzs.10:
    .asciz "Any$constructor"
Lkzs.11:
    .asciz "Any"
Lkzs.12:
    .asciz "yalx/lang:lang.Any"
Lkzs.13:
    .asciz "fun (string)->(string)"
Lkzs.14:
    .asciz "fun (string)->(u32)"
Lkzs.15:
    .asciz "fun (string,u32,u32,u32,u8[])->(void)"
Lkzs.16:
    .asciz "String$constructor"
Lkzs.17:
    .asciz "size"
Lkzs.18:
    .asciz "capacity"
Lkzs.19:
    .asciz "cachedHashCode"
Lkzs.20:
    .asciz "buf"
Lkzs.21:
    .asciz "String"
Lkzs.22:
    .asciz "yalx/lang:lang.String"
Lkzs.23:
    .asciz "fun (yalx/lang:lang.I32)->(string)"
Lkzs.24:
    .asciz "fun (yalx/lang:lang.I32,i32)->(void)"
Lkzs.25:
    .asciz "I32$constructor"
Lkzs.26:
    .asciz "I32"
Lkzs.27:
    .asciz "yalx/lang:lang.I32"
Lkzs.28:
    .asciz "fun (yalx/lang:lang.U32)->(string)"
Lkzs.29:
    .asciz "fun (yalx/lang:lang.U32,u32)->(void)"
Lkzs.30:
    .asciz "U32$constructor"
Lkzs.31:
    .asciz "U32"
Lkzs.32:
    .asciz "yalx/lang:lang.U32"
Lkzs.33:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(i8)"
Lkzs.34:
    .asciz "toI8"
Lkzs.35:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(u8)"
Lkzs.36:
    .asciz "toU8"
Lkzs.37:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(i16)"
Lkzs.38:
    .asciz "toI16"
Lkzs.39:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(u16)"
Lkzs.40:
    .asciz "toU16"
Lkzs.41:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(i32)"
Lkzs.42:
    .asciz "toI32"
Lkzs.43:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(u32)"
Lkzs.44:
    .asciz "toU32"
Lkzs.45:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(i64)"
Lkzs.46:
    .asciz "toI64"
Lkzs.47:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(u64)"
Lkzs.48:
    .asciz "toU64"
Lkzs.49:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(f32)"
Lkzs.50:
    .asciz "toF32"
Lkzs.51:
    .asciz "fun (yalx/lang:lang.Number<i32>)->(f64)"
Lkzs.52:
    .asciz "toF64"
Lkzs.53:
    .asciz "fun (yalx/lang:lang.Number<i32>,i32)->(void)"
Lkzs.54:
    .asciz "Number<i32>$constructor"
Lkzs.55:
    .asciz "value"
Lkzs.56:
    .asciz "Number<i32>"
Lkzs.57:
    .asciz "yalx/lang:lang.Number<i32>"
Lkzs.58:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(i8)"
Lkzs.59:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(u8)"
Lkzs.60:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(i16)"
Lkzs.61:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(u16)"
Lkzs.62:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(i32)"
Lkzs.63:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(u32)"
Lkzs.64:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(i64)"
Lkzs.65:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(u64)"
Lkzs.66:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(f32)"
Lkzs.67:
    .asciz "fun (yalx/lang:lang.Number<u32>)->(f64)"
Lkzs.68:
    .asciz "fun (yalx/lang:lang.Number<u32>,u32)->(void)"
Lkzs.69:
    .asciz "Number<u32>$constructor"
Lkzs.70:
    .asciz "Number<u32>"
Lkzs.71:
    .asciz "yalx/lang:lang.Number<u32>"
Lkzs.72:
    .asciz "fun (yalx/lang:lang.Throwable)->(string)"
Lkzs.73:
    .asciz "fun (yalx/lang:lang.Throwable)->(void)"
Lkzs.74:
    .asciz "printBacktrace"
Lkzs.75:
    .asciz "fun (yalx/lang:lang.Throwable,string,yalx/lang:lang.Throwable)->(void)"
Lkzs.76:
    .asciz "Throwable$constructor"
Lkzs.77:
    .asciz "message"
Lkzs.78:
    .asciz "linked"
Lkzs.79:
    .asciz "backtrace"
Lkzs.80:
    .asciz "Throwable"
Lkzs.81:
    .asciz "yalx/lang:lang.Throwable"
Lkzs.82:
    .asciz "fun (yalx/lang:lang.Exception,string,yalx/lang:lang.Exception)->(void)"
Lkzs.83:
    .asciz "Exception$constructor"
Lkzs.84:
    .asciz "Exception"
Lkzs.85:
    .asciz "yalx/lang:lang.Exception"
Lkzs.86:
    .asciz "fun (yalx/lang:lang.BadCastingException,string)->(void)"
Lkzs.87:
    .asciz "BadCastingException$constructor"
Lkzs.88:
    .asciz "BadCastingException"
Lkzs.89:
    .asciz "yalx/lang:lang.BadCastingException"
Lkzs.90:
    .asciz "fun (yalx/lang:lang.ArrayIndexOutOfBoundsException,string)->(void)"
Lkzs.91:
    .asciz "ArrayIndexOutOfBoundsException$constructor"
Lkzs.92:
    .asciz "ArrayIndexOutOfBoundsException"
Lkzs.93:
    .asciz "yalx/lang:lang.ArrayIndexOutOfBoundsException"
Lkzs.94:
    .asciz "fun (yalx/lang:lang.BacktraceFrame,u64,string,string,u32)->(void)"
Lkzs.95:
    .asciz "BacktraceFrame$constructor"
Lkzs.96:
    .asciz "address"
Lkzs.97:
    .asciz "function"
Lkzs.98:
    .asciz "file"
Lkzs.99:
    .asciz "line"
Lkzs.100:
    .asciz "BacktraceFrame"
Lkzs.101:
    .asciz "yalx/lang:lang.BacktraceFrame"
.section __DATA,__data
.p2align 4
# classes:
.global _yalx_Zplang_Zolang_ZdAny$class
_yalx_Zplang_Zolang_ZdAny$class:
    .quad 0 # id
    .byte 0 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 16 # instance_size
    .space 4 # padding
    .quad 0 # super
    .quad Lkzs.11 # name
    .long 3 # name
    .space 4 # padding
    .quad Lkzs.12 # location
    .long 18 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 0 # n_fields
    .space 4 # padding
    .quad 0 # fields
    .quad _yalx_Zplang_Zolang_ZdAny$ctor # ctor
    .long 6 # n_methods
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny$methods # methods
    .long 5 # n_vtab
    .long 0 # n_itab
    .quad _yalx_Zplang_Zolang_ZdAny$vtab # vtab
    .quad 0 # itab
_yalx_Zplang_Zolang_ZdAny$methods:
    # Any::id
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.1 # name
    .long 2 # name
    .space 4 # padding
    .quad Lkzs.0 # prototype_desc
    .long 31 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny_Zdid # entry
    # Any::hashCode
    .long 1 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.3 # name
    .long 8 # name
    .space 4 # padding
    .quad Lkzs.2 # prototype_desc
    .long 31 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny_ZdhashCode # entry
    # Any::toString
    .long 2 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.5 # name
    .long 8 # name
    .space 4 # padding
    .quad Lkzs.4 # prototype_desc
    .long 34 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny_ZdtoString # entry
    # Any::isEmpty
    .long 3 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.7 # name
    .long 7 # name
    .space 4 # padding
    .quad Lkzs.6 # prototype_desc
    .long 30 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny_ZdisEmpty # entry
    # Any::finalize
    .long 4 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.9 # name
    .long 8 # name
    .space 4 # padding
    .quad Lkzs.8 # prototype_desc
    .long 32 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny_Zdfinalize # entry
_yalx_Zplang_Zolang_ZdAny$ctor:
    # Any::Any$constructor
    .long 5 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.10 # name
    .long 15 # name
    .space 4 # padding
    .quad Lkzs.8 # prototype_desc
    .long 32 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor # entry
_yalx_Zplang_Zolang_ZdAny$vtab:
    .quad _yalx_Zplang_Zolang_ZdAny_Zdfinalize
    .quad _yalx_Zplang_Zolang_ZdAny_ZdhashCode
    .quad _yalx_Zplang_Zolang_ZdAny_Zdid
    .quad _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
    .quad _yalx_Zplang_Zolang_ZdAny_ZdtoString
.global _yalx_Zplang_Zolang_ZdString$class
_yalx_Zplang_Zolang_ZdString$class:
    .quad 0 # id
    .byte 0 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 40 # instance_size
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny$class # super
    .quad Lkzs.21 # name
    .long 6 # name
    .space 4 # padding
    .quad Lkzs.22 # location
    .long 21 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 4 # n_fields
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString$fields # fields
    .quad _yalx_Zplang_Zolang_ZdString$ctor # ctor
    .long 3 # n_methods
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_yalx_Zplang_Zolang_ZdString$fields:
    # String::size
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.17 # name
    .long 4 # name
    .space 4 # padding
    .quad _builtin_classes+1008 # type
    .long 16 # offset_of_head
    .space 4 # padding
    # String::capacity
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.18 # name
    .long 8 # name
    .space 4 # padding
    .quad _builtin_classes+1008 # type
    .long 20 # offset_of_head
    .space 4 # padding
    # String::cachedHashCode
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.19 # name
    .long 14 # name
    .space 4 # padding
    .quad _builtin_classes+1008 # type
    .long 24 # offset_of_head
    .space 4 # padding
    # String::buf
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.20 # name
    .long 3 # name
    .space 4 # padding
    .quad _builtin_classes+3456 # type
    .long 32 # offset_of_head
    .space 4 # padding
_yalx_Zplang_Zolang_ZdString$methods:
    # String::toString
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.5 # name
    .long 8 # name
    .space 4 # padding
    .quad Lkzs.13 # prototype_desc
    .long 22 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString_ZdtoString # entry
    # String::hashCode
    .long 1 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.3 # name
    .long 8 # name
    .space 4 # padding
    .quad Lkzs.14 # prototype_desc
    .long 19 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString_ZdhashCode # entry
_yalx_Zplang_Zolang_ZdString$ctor:
    # String::String$constructor
    .long 2 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.16 # name
    .long 18 # name
    .space 4 # padding
    .quad Lkzs.15 # prototype_desc
    .long 37 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString_ZdString_Z4constructor # entry
.global _yalx_Zplang_Zolang_ZdI32$class
_yalx_Zplang_Zolang_ZdI32$class:
    .quad 0 # id
    .byte 1 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 24 # instance_size
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$class # super
    .quad Lkzs.26 # name
    .long 3 # name
    .space 4 # padding
    .quad Lkzs.27 # location
    .long 18 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 0 # n_fields
    .space 4 # padding
    .quad 0 # fields
    .quad _yalx_Zplang_Zolang_ZdI32$ctor # ctor
    .long 2 # n_methods
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdI32$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_yalx_Zplang_Zolang_ZdI32$methods:
    # I32::toString
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.5 # name
    .long 8 # name
    .space 4 # padding
    .quad Lkzs.23 # prototype_desc
    .long 34 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdI32_ZdtoString # entry
_yalx_Zplang_Zolang_ZdI32$ctor:
    # I32::I32$constructor
    .long 1 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.25 # name
    .long 15 # name
    .space 4 # padding
    .quad Lkzs.24 # prototype_desc
    .long 36 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdI32_ZdI32_Z4constructor # entry
.global _yalx_Zplang_Zolang_ZdU32$class
_yalx_Zplang_Zolang_ZdU32$class:
    .quad 0 # id
    .byte 1 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 24 # instance_size
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$class # super
    .quad Lkzs.31 # name
    .long 3 # name
    .space 4 # padding
    .quad Lkzs.32 # location
    .long 18 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 0 # n_fields
    .space 4 # padding
    .quad 0 # fields
    .quad _yalx_Zplang_Zolang_ZdU32$ctor # ctor
    .long 2 # n_methods
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdU32$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_yalx_Zplang_Zolang_ZdU32$methods:
    # U32::toString
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.5 # name
    .long 8 # name
    .space 4 # padding
    .quad Lkzs.28 # prototype_desc
    .long 34 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdU32_ZdtoString # entry
_yalx_Zplang_Zolang_ZdU32$ctor:
    # U32::U32$constructor
    .long 1 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.30 # name
    .long 15 # name
    .space 4 # padding
    .quad Lkzs.29 # prototype_desc
    .long 36 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdU32_ZdU32_Z4constructor # entry
.global _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$class
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$class:
    .quad 0 # id
    .byte 1 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 24 # instance_size
    .space 4 # padding
    .quad 0 # super
    .quad Lkzs.56 # name
    .long 11 # name
    .space 4 # padding
    .quad Lkzs.57 # location
    .long 26 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 1 # n_fields
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$fields # fields
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$ctor # ctor
    .long 11 # n_methods
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$fields:
    # Number<i32>::value
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.55 # name
    .long 5 # name
    .space 4 # padding
    .quad _builtin_classes+864 # type
    .long 16 # offset_of_head
    .space 4 # padding
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$methods:
    # Number<i32>::toI8
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.34 # name
    .long 4 # name
    .space 4 # padding
    .quad Lkzs.33 # prototype_desc
    .long 38 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI8 # entry
    # Number<i32>::toU8
    .long 1 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.36 # name
    .long 4 # name
    .space 4 # padding
    .quad Lkzs.35 # prototype_desc
    .long 38 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU8 # entry
    # Number<i32>::toI16
    .long 2 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.38 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.37 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI16 # entry
    # Number<i32>::toU16
    .long 3 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.40 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.39 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU16 # entry
    # Number<i32>::toI32
    .long 4 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.42 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.41 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI32 # entry
    # Number<i32>::toU32
    .long 5 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.44 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.43 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU32 # entry
    # Number<i32>::toI64
    .long 6 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.46 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.45 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoI64 # entry
    # Number<i32>::toU64
    .long 7 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.48 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.47 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoU64 # entry
    # Number<i32>::toF32
    .long 8 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.50 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.49 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoF32 # entry
    # Number<i32>::toF64
    .long 9 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.52 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.51 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdtoF64 # entry
_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl$ctor:
    # Number<i32>::Number<i32>$constructor
    .long 10 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.54 # name
    .long 23 # name
    .space 4 # padding
    .quad Lkzs.53 # prototype_desc
    .long 44 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dki32_Dl_ZdNumber_Dki32_Dl_Z4constructor # entry
.global _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$class
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$class:
    .quad 0 # id
    .byte 1 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 24 # instance_size
    .space 4 # padding
    .quad 0 # super
    .quad Lkzs.70 # name
    .long 11 # name
    .space 4 # padding
    .quad Lkzs.71 # location
    .long 26 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 1 # n_fields
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$fields # fields
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$ctor # ctor
    .long 11 # n_methods
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$fields:
    # Number<u32>::value
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.55 # name
    .long 5 # name
    .space 4 # padding
    .quad _builtin_classes+1008 # type
    .long 16 # offset_of_head
    .space 4 # padding
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$methods:
    # Number<u32>::toI8
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.34 # name
    .long 4 # name
    .space 4 # padding
    .quad Lkzs.58 # prototype_desc
    .long 38 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI8 # entry
    # Number<u32>::toU8
    .long 1 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.36 # name
    .long 4 # name
    .space 4 # padding
    .quad Lkzs.59 # prototype_desc
    .long 38 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU8 # entry
    # Number<u32>::toI16
    .long 2 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.38 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.60 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI16 # entry
    # Number<u32>::toU16
    .long 3 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.40 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.61 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU16 # entry
    # Number<u32>::toI32
    .long 4 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.42 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.62 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI32 # entry
    # Number<u32>::toU32
    .long 5 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.44 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.63 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU32 # entry
    # Number<u32>::toI64
    .long 6 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.46 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.64 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoI64 # entry
    # Number<u32>::toU64
    .long 7 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.48 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.65 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoU64 # entry
    # Number<u32>::toF32
    .long 8 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.50 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.66 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoF32 # entry
    # Number<u32>::toF64
    .long 9 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.52 # name
    .long 5 # name
    .space 4 # padding
    .quad Lkzs.67 # prototype_desc
    .long 39 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdtoF64 # entry
_yalx_Zplang_Zolang_ZdNumber_Dku32_Dl$ctor:
    # Number<u32>::Number<u32>$constructor
    .long 10 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.69 # name
    .long 23 # name
    .space 4 # padding
    .quad Lkzs.68 # prototype_desc
    .long 44 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdNumber_Dku32_Dl_ZdNumber_Dku32_Dl_Z4constructor # entry
.global _yalx_Zplang_Zolang_ZdThrowable$class
_yalx_Zplang_Zolang_ZdThrowable$class:
    .quad 0 # id
    .byte 0 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 40 # instance_size
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny$class # super
    .quad Lkzs.80 # name
    .long 9 # name
    .space 4 # padding
    .quad Lkzs.81 # location
    .long 24 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 3 # n_fields
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdThrowable$fields # fields
    .quad _yalx_Zplang_Zolang_ZdThrowable$ctor # ctor
    .long 3 # n_methods
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdThrowable$methods # methods
    .long 5 # n_vtab
    .long 0 # n_itab
    .quad _yalx_Zplang_Zolang_ZdThrowable$vtab # vtab
    .quad 0 # itab
_yalx_Zplang_Zolang_ZdThrowable$fields:
    # Throwable::message
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.77 # name
    .long 7 # name
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString$class # type
    .long 16 # offset_of_head
    .space 4 # padding
    # Throwable::linked
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.78 # name
    .long 6 # name
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdThrowable$class # type
    .long 24 # offset_of_head
    .space 4 # padding
    # Throwable::backtrace
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.79 # name
    .long 9 # name
    .space 4 # padding
    .quad _builtin_classes+3600 # type
    .long 32 # offset_of_head
    .space 4 # padding
_yalx_Zplang_Zolang_ZdThrowable$methods:
    # Throwable::toString
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.5 # name
    .long 8 # name
    .space 4 # padding
    .quad Lkzs.72 # prototype_desc
    .long 40 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdThrowable_ZdtoString # entry
    # Throwable::printBacktrace
    .long 1 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.74 # name
    .long 14 # name
    .space 4 # padding
    .quad Lkzs.73 # prototype_desc
    .long 38 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdThrowable_ZdprintBacktrace # entry
_yalx_Zplang_Zolang_ZdThrowable$ctor:
    # Throwable::Throwable$constructor
    .long 2 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.76 # name
    .long 21 # name
    .space 4 # padding
    .quad Lkzs.75 # prototype_desc
    .long 70 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdThrowable_ZdThrowable_Z4constructor # entry
_yalx_Zplang_Zolang_ZdThrowable$vtab:
    .quad _yalx_Zplang_Zolang_ZdAny_Zdfinalize
    .quad _yalx_Zplang_Zolang_ZdAny_ZdhashCode
    .quad _yalx_Zplang_Zolang_ZdAny_Zdid
    .quad _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
    .quad _yalx_Zplang_Zolang_ZdThrowable_ZdtoString
.global _yalx_Zplang_Zolang_ZdException$class
_yalx_Zplang_Zolang_ZdException$class:
    .quad 0 # id
    .byte 0 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 40 # instance_size
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdThrowable$class # super
    .quad Lkzs.84 # name
    .long 9 # name
    .space 4 # padding
    .quad Lkzs.85 # location
    .long 24 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 0 # n_fields
    .space 4 # padding
    .quad 0 # fields
    .quad _yalx_Zplang_Zolang_ZdException$ctor # ctor
    .long 1 # n_methods
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdException$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_yalx_Zplang_Zolang_ZdException$methods:
_yalx_Zplang_Zolang_ZdException$ctor:
    # Exception::Exception$constructor
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.83 # name
    .long 21 # name
    .space 4 # padding
    .quad Lkzs.82 # prototype_desc
    .long 70 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdException_ZdException_Z4constructor # entry
.global _yalx_Zplang_Zolang_ZdBadCastingException$class
_yalx_Zplang_Zolang_ZdBadCastingException$class:
    .quad 0 # id
    .byte 0 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 40 # instance_size
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdException$class # super
    .quad Lkzs.88 # name
    .long 19 # name
    .space 4 # padding
    .quad Lkzs.89 # location
    .long 34 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 0 # n_fields
    .space 4 # padding
    .quad 0 # fields
    .quad _yalx_Zplang_Zolang_ZdBadCastingException$ctor # ctor
    .long 1 # n_methods
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdBadCastingException$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_yalx_Zplang_Zolang_ZdBadCastingException$methods:
_yalx_Zplang_Zolang_ZdBadCastingException$ctor:
    # BadCastingException::BadCastingException$constructor
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.87 # name
    .long 31 # name
    .space 4 # padding
    .quad Lkzs.86 # prototype_desc
    .long 55 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdBadCastingException_ZdBadCastingException_Z4constructor # entry
.global _yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException$class
_yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException$class:
    .quad 0 # id
    .byte 0 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 40 # instance_size
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdException$class # super
    .quad Lkzs.92 # name
    .long 30 # name
    .space 4 # padding
    .quad Lkzs.93 # location
    .long 45 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 0 # n_fields
    .space 4 # padding
    .quad 0 # fields
    .quad _yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException$ctor # ctor
    .long 1 # n_methods
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException$methods:
_yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException$ctor:
    # ArrayIndexOutOfBoundsException::ArrayIndexOutOfBoundsException$constructor
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.91 # name
    .long 42 # name
    .space 4 # padding
    .quad Lkzs.90 # prototype_desc
    .long 66 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdArrayIndexOutOfBoundsException_ZdArrayIndexOutOfBoundsException_Z4constructor # entry
.global _yalx_Zplang_Zolang_ZdBacktraceFrame$class
_yalx_Zplang_Zolang_ZdBacktraceFrame$class:
    .quad 0 # id
    .byte 0 # constraint
    .space 3 # padding
    .long 8 # reference_size
    .long 48 # instance_size
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdAny$class # super
    .quad Lkzs.100 # name
    .long 14 # name
    .space 4 # padding
    .quad Lkzs.101 # location
    .long 29 # location
    .space 4 # padding
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .long 4 # n_fields
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdBacktraceFrame$fields # fields
    .quad _yalx_Zplang_Zolang_ZdBacktraceFrame$ctor # ctor
    .long 1 # n_methods
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdBacktraceFrame$methods # methods
    .long 0 # n_vtab
    .long 0 # n_itab
    .quad 0 # vtab
    .quad 0 # itab
_yalx_Zplang_Zolang_ZdBacktraceFrame$fields:
    # BacktraceFrame::address
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.96 # name
    .long 7 # name
    .space 4 # padding
    .quad _builtin_classes+1008 # type
    .long 16 # offset_of_head
    .space 4 # padding
    # BacktraceFrame::function
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.97 # name
    .long 8 # name
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString$class # type
    .long 24 # offset_of_head
    .space 4 # padding
    # BacktraceFrame::file
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.98 # name
    .long 4 # name
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdString$class # type
    .long 32 # offset_of_head
    .space 4 # padding
    # BacktraceFrame::line
    .long 0 # access|constraint
    .long 0 # n_annotations
    .quad 0 # reserved0
    .quad Lkzs.99 # name
    .long 4 # name
    .space 4 # padding
    .quad _builtin_classes+1008 # type
    .long 40 # offset_of_head
    .space 4 # padding
_yalx_Zplang_Zolang_ZdBacktraceFrame$methods:
_yalx_Zplang_Zolang_ZdBacktraceFrame$ctor:
    # BacktraceFrame::BacktraceFrame$constructor
    .long 0 # index
    .long 0 # access|is_native|is_override|...
    .long 0 # n_annotations
    .space 4 # padding
    .quad 0 # reserved0
    .quad Lkzs.95 # name
    .long 26 # name
    .space 4 # padding
    .quad Lkzs.94 # prototype_desc
    .long 65 # prototype_desc
    .space 4 # padding
    .quad _yalx_Zplang_Zolang_ZdBacktraceFrame_ZdBacktraceFrame_Z4constructor # entry
.section __DATA,__data
.p2align 4
# Yalx-String constants
.global _yalx_Zplang_Zolang_Lksz
_yalx_Zplang_Zolang_Lksz:
    .long 102
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
    .quad Lkzs.22
    .quad Lkzs.23
    .quad Lkzs.24
    .quad Lkzs.25
    .quad Lkzs.26
    .quad Lkzs.27
    .quad Lkzs.28
    .quad Lkzs.29
    .quad Lkzs.30
    .quad Lkzs.31
    .quad Lkzs.32
    .quad Lkzs.33
    .quad Lkzs.34
    .quad Lkzs.35
    .quad Lkzs.36
    .quad Lkzs.37
    .quad Lkzs.38
    .quad Lkzs.39
    .quad Lkzs.40
    .quad Lkzs.41
    .quad Lkzs.42
    .quad Lkzs.43
    .quad Lkzs.44
    .quad Lkzs.45
    .quad Lkzs.46
    .quad Lkzs.47
    .quad Lkzs.48
    .quad Lkzs.49
    .quad Lkzs.50
    .quad Lkzs.51
    .quad Lkzs.52
    .quad Lkzs.53
    .quad Lkzs.54
    .quad Lkzs.55
    .quad Lkzs.56
    .quad Lkzs.57
    .quad Lkzs.58
    .quad Lkzs.59
    .quad Lkzs.60
    .quad Lkzs.61
    .quad Lkzs.62
    .quad Lkzs.63
    .quad Lkzs.64
    .quad Lkzs.65
    .quad Lkzs.66
    .quad Lkzs.67
    .quad Lkzs.68
    .quad Lkzs.69
    .quad Lkzs.70
    .quad Lkzs.71
    .quad Lkzs.72
    .quad Lkzs.73
    .quad Lkzs.74
    .quad Lkzs.75
    .quad Lkzs.76
    .quad Lkzs.77
    .quad Lkzs.78
    .quad Lkzs.79
    .quad Lkzs.80
    .quad Lkzs.81
    .quad Lkzs.82
    .quad Lkzs.83
    .quad Lkzs.84
    .quad Lkzs.85
    .quad Lkzs.86
    .quad Lkzs.87
    .quad Lkzs.88
    .quad Lkzs.89
    .quad Lkzs.90
    .quad Lkzs.91
    .quad Lkzs.92
    .quad Lkzs.93
    .quad Lkzs.94
    .quad Lkzs.95
    .quad Lkzs.96
    .quad Lkzs.97
    .quad Lkzs.98
    .quad Lkzs.99
    .quad Lkzs.100
    .quad Lkzs.101
.global _yalx_Zplang_Zolang_Kstr
_yalx_Zplang_Zolang_Kstr:
    .long 102
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
Kstr.22:
    .quad 0
Kstr.23:
    .quad 0
Kstr.24:
    .quad 0
Kstr.25:
    .quad 0
Kstr.26:
    .quad 0
Kstr.27:
    .quad 0
Kstr.28:
    .quad 0
Kstr.29:
    .quad 0
Kstr.30:
    .quad 0
Kstr.31:
    .quad 0
Kstr.32:
    .quad 0
Kstr.33:
    .quad 0
Kstr.34:
    .quad 0
Kstr.35:
    .quad 0
Kstr.36:
    .quad 0
Kstr.37:
    .quad 0
Kstr.38:
    .quad 0
Kstr.39:
    .quad 0
Kstr.40:
    .quad 0
Kstr.41:
    .quad 0
Kstr.42:
    .quad 0
Kstr.43:
    .quad 0
Kstr.44:
    .quad 0
Kstr.45:
    .quad 0
Kstr.46:
    .quad 0
Kstr.47:
    .quad 0
Kstr.48:
    .quad 0
Kstr.49:
    .quad 0
Kstr.50:
    .quad 0
Kstr.51:
    .quad 0
Kstr.52:
    .quad 0
Kstr.53:
    .quad 0
Kstr.54:
    .quad 0
Kstr.55:
    .quad 0
Kstr.56:
    .quad 0
Kstr.57:
    .quad 0
Kstr.58:
    .quad 0
Kstr.59:
    .quad 0
Kstr.60:
    .quad 0
Kstr.61:
    .quad 0
Kstr.62:
    .quad 0
Kstr.63:
    .quad 0
Kstr.64:
    .quad 0
Kstr.65:
    .quad 0
Kstr.66:
    .quad 0
Kstr.67:
    .quad 0
Kstr.68:
    .quad 0
Kstr.69:
    .quad 0
Kstr.70:
    .quad 0
Kstr.71:
    .quad 0
Kstr.72:
    .quad 0
Kstr.73:
    .quad 0
Kstr.74:
    .quad 0
Kstr.75:
    .quad 0
Kstr.76:
    .quad 0
Kstr.77:
    .quad 0
Kstr.78:
    .quad 0
Kstr.79:
    .quad 0
Kstr.80:
    .quad 0
Kstr.81:
    .quad 0
Kstr.82:
    .quad 0
Kstr.83:
    .quad 0
Kstr.84:
    .quad 0
Kstr.85:
    .quad 0
Kstr.86:
    .quad 0
Kstr.87:
    .quad 0
Kstr.88:
    .quad 0
Kstr.89:
    .quad 0
Kstr.90:
    .quad 0
Kstr.91:
    .quad 0
Kstr.92:
    .quad 0
Kstr.93:
    .quad 0
Kstr.94:
    .quad 0
Kstr.95:
    .quad 0
Kstr.96:
    .quad 0
Kstr.97:
    .quad 0
Kstr.98:
    .quad 0
Kstr.99:
    .quad 0
Kstr.100:
    .quad 0
Kstr.101:
    .quad 0
