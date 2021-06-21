//----------------------------------------------------------------------------------------------------------------------
//string object methods
//----------------------------------------------------------------------------------------------------------------------


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
