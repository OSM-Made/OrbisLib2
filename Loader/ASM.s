.intel_syntax noprefix
.text

.global    kernelRdmsr
kernelRdmsr:
    mov    ecx, edi
    rdmsr
    shl    rdx, 32
    or    rax, rdx
    ret

.global    cpu_enable_wp
cpu_enable_wp:
  mov rax, cr0
  or rax, 0x10000
  mov cr0, rax
  ret

.global    cpu_disable_wp
cpu_disable_wp:
  mov rax, cr0
  and rax, ~0x10000
  mov cr0, rax
  ret

.att_syntax prefix
.globl syscall
syscall:
	movq $0, %rax
	movq %rcx, %r10
	syscall
	jb err
	retq
err:
	pushq %rax
	callq __error
	popq %rcx
	movl %ecx, 0(%rax)
	movq $0xFFFFFFFFFFFFFFFF, %rax
	movq $0xFFFFFFFFFFFFFFFF, %rdx
	retq
