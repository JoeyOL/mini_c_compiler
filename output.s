.section	.data
FLOAT_CONST_0:
	.double	1
.section	.data
	.text
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
BLOCK_0:
	movl	$1, %r10d
	movl	%r10d, -12(%rbp)
WHILE_START_0:
	movl	-12(%rbp), %r10d
	movslq	%r10d, %r10
	movl	$3, %r11d
	movslq	%r11d, %r11
	cmpq	%r11, %r10
	jg	WHILE_END_0
BLOCK_1:
	movl	-12(%rbp), %r10d
	movslq	%r10d, %r10
	cvtsi2sd	%r10, %xmm8
	movsd	%xmm8, %xmm0
	call	printfloat
	movl	-12(%rbp), %r10d
	movl	$1, %r11d
	addl	%r11d, %r10d
	movl	%r10d, -12(%rbp)
	jmp	WHILE_START_0
WHILE_END_0:
main_end:
	addq	$16, %rsp
	popq	%rbp
	ret
