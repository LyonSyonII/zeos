#include <asm.h>


#define CHECK_ERROR(sysid) \
      cmpl $0, %eax; /* mira si hi ha hagut error */ \
      jge err_fi##sysid; \
      pushl %eax; /* col·loquem el valor que toqui a errno */ \
      call set_errno; \
      addl $4, %esp; \
      movl $-1, %eax; /* indiquem a l'usuari que hi ha hagut un error */ \
      err_fi##sysid:

# -- 3.9.1 Writing the wrapper (Zeos.pdf:36) --
# 1) [done in wrapper]
# 2) eax <- syscall_id 
# 3) [not necessary?]
# 4) store return address (sys_ret##sysid, eg: sys_ret4 when write)
# 5) fake dynamic link
# 6) sysenter
# 7) remove temporal data
# 8) [done in wrapper]
#define SYSCALL(sysid) \
	movl $sysid, %eax; \
	push $sys_ret##sysid; \
	push %ebp; \
	movl %esp, %ebp; \
	sysenter; \
	sys_ret##sysid: pop %ebp; \
	addl $4, %esp; \
      CHECK_ERROR(sysid)

# Parameters: EDX, ECX, EBX, ESI, EDI, EBP

ENTRY(write)
      pushl %ebp              # dynamic link
      movl %esp, %ebp
      
      pushl %ebx              # save ebx; ecx and edx are volatile registers
      
                              # 1) parameter passing
      movl 8(%ebp), %edx      # edx = fd
      movl 12(%ebp), %ecx     # ecx = buffer
      movl 16(%ebp), %ebx     # ebx = size
      
      SYSCALL(4)              # write

      popl %ebx               # restore registers
      popl %ebp
      ret


ENTRY(gettime)
      pushl %ebp              # dynamic link
      movl %esp, %ebp
      
      SYSCALL(10)              # write
      
      popl %ebp
      ret