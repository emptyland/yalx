.section __TEXT,__text,regular,pure_instructions
.p2align 2

.text
.global _entry
_entry:
    pushq %rbp
    movq %rsp, %rbp
    xorq %rax, %rax
    popq %rbp
    ret

.global _asm_stub1
_asm_stub1:
    pushq %rbp
    movl %edi, %eax
    addl %esi, %eax
    popq %rbp
    ret

.global _asm_stub2
_asm_stub2:
    pushq %rbp
    movl 8(%rdi), %eax
    popq %rbp
    ret
