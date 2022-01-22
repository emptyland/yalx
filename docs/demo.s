	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 11, 0	sdk_version 12, 1
	.globl	_foo                            ## -- Begin function foo
	.p2align	4, 0x90
_foo:                                   ## @foo
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	movl	-4(%rbp), %eax
	addl	-8(%rbp), %eax
	popq	%rbp
	retq
	.cfi_endproc
                                        ## -- End function
	.section	__DATA,__data
	.globl	_bazs                           ## @bazs
	.p2align	4
_bazs:
	.long	1                               ## 0x1
	.long	2                               ## 0x2
	.long	2                               ## 0x2
	.long	3                               ## 0x3

	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"Name"

	.section	__DATA,__data
	.globl	_boo                            ## @boo
	.p2align	3
_boo:
	.long	100                             ## 0x64
	.long	200                             ## 0xc8
	.quad	_bazs
	.quad	L_.str

.subsections_via_symbols
