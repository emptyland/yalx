//----------------------------------------------------------------------------------------------------------------------
// any object methods
//----------------------------------------------------------------------------------------------------------------------
.section __TEXT,__text,regular,pure_instructions
.p2align 2

.global _y3zlang_any_id
// lang.any.hashCode(): u32
_y3zlang_any_id:
    pushq %rbp
    movq %rsp, %rbp

    movq 16(%rbp), %rdi // first argument(any *)
    shrq $2, %rdi
    movl %edi, %eax // get hash_code

    movl %eax, 12(%rbp) // setup return value
    movl $1, %eax

    popq %rbp
    ret


.global _y3zlang_any_hashCode
// lang.any.hashCode(): u32
_y3zlang_any_hashCode:
    pushq %rbp
    movq %rsp, %rbp

    movq 16(%rbp), %rdi // first argument(any *)
    shrq $2, %rdi
    movl %edi, %eax // get hash_code

    movl %eax, 12(%rbp) // setup return value
    movl $1, %eax

    popq %rbp
    ret


.global _y3zlang_any_toString,_yalx_any_to_string
// lang.any.toString(): string
_y3zlang_any_toString:
    pushq %rbp
    movq %rsp, %rbp

    movq 16(%rbp), %rdi // first argument(any *)
    callq _yalx_any_to_string
    movq %rax, 16(%rbp)
    movl $1, %eax

    popq %rbp
    ret

.global _y3zlang_any_isEmpty
// lang.any.isEmpty(): bool
_y3zlang_any_isEmpty:
    pushq %rbp
    movq %rsp, %rbp

    movq 16(%rbp), %rdi // first argument(any *)
    movl $0, 12(%rbp) // return false
    movl $1, %eax

    popq %rbp
    ret

.global _y3zlang_any_finalize
// lang.any.finalize(): unit
_y3zlang_any_finalize:
    pushq %rbp
    movq %rsp, %rbp

    nop
    movl $0, %eax

    popq %rbp
    ret
