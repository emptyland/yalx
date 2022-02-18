	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 12, 0	sdk_version 12, 1
	.globl	_bar                            ; -- Begin function bar
	.p2align	2
_bar:                                   ; @bar
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #16                     ; =16
	.cfi_def_cfa_offset 16
	str	w0, [sp, #12]
	str	w1, [sp, #8]
	ldr	w8, [sp, #12]
	ldr	w9, [sp, #8]
	mul	w0, w8, w9
	add	sp, sp, #16                     ; =16
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_foo                            ; -- Begin function foo
	.p2align	2
_foo:                                   ; @foo
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48                     ; =48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32                    ; =32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	stur	w0, [x29, #-4]
	stur	w1, [x29, #-8]
	stur	w2, [x29, #-12]
	str	s0, [sp, #16]
	str	s1, [sp, #12]
	str	s2, [sp, #8]
	ldur	w0, [x29, #-4]
	ldur	w1, [x29, #-8]
	bl	_bar
	ldur	w8, [x29, #-4]
	add	w8, w0, w8
	ldur	w9, [x29, #-8]
	add	w8, w8, w9
	scvtf	s0, w8
	ldur	w8, [x29, #-12]
	scvtf	s1, w8
	ldr	s2, [sp, #16]
	fmul	s1, s1, s2
	fsub	s0, s0, s1
	ldr	s1, [sp, #12]
	fadd	s0, s0, s1
	ldr	s1, [sp, #8]
	fadd	s0, s0, s1
	fcvtzs	w0, s0
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48                     ; =48
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
