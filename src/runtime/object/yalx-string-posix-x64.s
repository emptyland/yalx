//----------------------------------------------------------------------------------------------------------------------
//string object methods
//----------------------------------------------------------------------------------------------------------------------
.section __TEXT,__text,regular,pure_instructions
.p2align 2

.global _y3zlang_string_hashCode

// argument[0]
// return address
// saved rbp
//                 <- rsp and rbp now
_y3zlang_string_hashCode:
    pushq %rbp
    movq %rsp, %rbp

    movq 16(%rbp), %rdi // first argument(string *)
    movl 16(%rdi), %eax // get hash_code

    movl %eax, 12(%rbp) // setup return value
    movl $1, %eax

    popq %rbp
    ret


.global _y3zlang_string_isEmpty
// lang.string.isEmpty(): bool
_y3zlang_string_isEmpty:
    pushq %rbp
    movq %rsp, %rbp

    movq 16(%rbp), %rdi // first argument(string *)
    movl 20(%rdi), %eax // get len
    cmpl $0, %eax
    je empty_branch
    movl $0, 12(%rbp)
    jmp done
empty_branch:
    movl $1, 12(%rbp)
done:
    movl $1, %eax

    popq %rbp
    ret


.global _y3zlang_string_toString
// lang.string.toString(): string
_y3zlang_string_toString:
    pushq %rbp
    movq %rsp, %rbp

    nop

    movl $1, %eax
    popq %rbp
    ret
