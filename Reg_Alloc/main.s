	.file	"main.c"
	.text
	.def	__main;	.scl	2;	.type	32;	.endef
	.section .rdata,"dr"
.LC0:
	.ascii "rb\0"
.LC1:
	.ascii "w\0"
.LC2:
	.ascii "output.txt\0"
	.text
	.globl	main
	.def	main;	.scl	2;	.type	32;	.endef
	.seh_proc	main
main:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$48, %rsp
	.seh_stackalloc	48
	.seh_endprologue
	movl	%ecx, 16(%rbp)
	movq	%rdx, 24(%rbp)
	call	__main
	movq	24(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	leaq	.LC0(%rip), %rdx
	movq	%rax, %rcx
	call	fopen
	movq	%rax, %rdx
	movq	.refptr.yyin(%rip), %rax
	movq	%rdx, (%rax)
	call	yyparse
	movq	.refptr.yyin(%rip), %rax
	movq	(%rax), %rax
	movq	%rax, %rcx
	call	fclose
	movq	.refptr.root(%rip), %rax
	movq	(%rax), %rax
	movq	%rax, %rcx
	call	print_cmd
	movl	$10, %ecx
	call	putchar
	movq	.refptr.root(%rip), %rax
	movq	(%rax), %rax
	movq	%rax, %rcx
	call	TAC_gen
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	call	print_L1_cmd_listbox
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	call	BB_gen
	movq	%rax, -16(%rbp)
	movl	$1, %ecx
	movq	__imp___acrt_iob_func(%rip), %rax
	call	*%rax
	movq	%rax, %r8
	leaq	.LC1(%rip), %rdx
	leaq	.LC2(%rip), %rcx
	call	freopen
	movq	-16(%rbp), %rax
	movq	%rax, %rcx
	call	print_L1_basic_blocks
	movl	$0, %eax
	addq	$48, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0"
	.def	fopen;	.scl	2;	.type	32;	.endef
	.def	yyparse;	.scl	2;	.type	32;	.endef
	.def	fclose;	.scl	2;	.type	32;	.endef
	.def	print_cmd;	.scl	2;	.type	32;	.endef
	.def	putchar;	.scl	2;	.type	32;	.endef
	.def	TAC_gen;	.scl	2;	.type	32;	.endef
	.def	print_L1_cmd_listbox;	.scl	2;	.type	32;	.endef
	.def	BB_gen;	.scl	2;	.type	32;	.endef
	.def	freopen;	.scl	2;	.type	32;	.endef
	.def	print_L1_basic_blocks;	.scl	2;	.type	32;	.endef
	.section	.rdata$.refptr.root, "dr"
	.globl	.refptr.root
	.linkonce	discard
.refptr.root:
	.quad	root
	.section	.rdata$.refptr.yyin, "dr"
	.globl	.refptr.yyin
	.linkonce	discard
.refptr.yyin:
	.quad	yyin
