#include <asm.h>

#define SYSCALL(sysnum) \
	leal syscall_ret, %eax; \
	push %eax; \
	movl $sysnum, %eax; \
	push %ebp; \
	movl %esp, %ebp; \
	sysenter; \
	syscall_ret: pop %ebp; \
	addl $4, %esp

# Parameters: EDX, ECX, EBX, ESI, EDI, EBP

ENTRY(write)
      pushl %ebp              # dynamic link
      movl %esp, %ebp
      
      pushl %ebx              # save ebx; ecx and edx are volatile registers
      
      movl 8(%ebp), %edx      # edx = fd
      movl 12(%ebp), %ecx     # ecx = buffer
      movl 16(%ebp), %ebx     # ebx = size
      
      SYSCALL(4)              # write

      # TODO: Handle error

      popl %ebx
      popl %ebp
      ret
