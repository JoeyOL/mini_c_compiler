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
	movl	$2, %r10d
	movl	$3, %r11d
	movl	$5, %r12d
	imull	%r12d, %r11d
	addl	%r11d, %r10d
	movl	$8, %r11d
	movl	$3, %r12d
	movl	%r11d, %eax
	cqto
	idivl	%r12d
	movl	%eax, %r11d
	subl	%r11d, %r10d
	movl	%r10d, -12(%rbp)
	movl	-12(%rbp), %r10d
	movl	%r10d, %eax
	jmp	main_end
main_end:
	addq	$16, %rsp
	popq	%rbp
	ret
