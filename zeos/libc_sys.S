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

ENTRY(getpid)
      pushl %ebp              # dynamic link
      movl %esp, %ebp
      
      SYSCALL(20)             # getpid
      
      popl %ebp
      ret

ENTRY(getppid)
      pushl %ebp              # dynamic link
      movl %esp, %ebp
      
      SYSCALL(21)             # getppid
      
      popl %ebp
      ret

ENTRY(fork)
      pushl %ebp              # dynamic link
      movl %esp, %ebp

      SYSCALL(2)              # fork
      
      popl %ebp
      ret

ENTRY(exit)
      pushl %ebp
      movl %esp, %ebp
      
      SYSCALL(1)
      
      popl %ebp
      ret

ENTRY(block)
      pushl %ebp
      movl %esp, %ebp
      
      SYSCALL(15)             # assignat arbitrariament
      
      popl %ebp
      ret

ENTRY(unblock)
      pushl %ebp
      movl %esp, %ebp
      
      movl 8(%ebp), %edx      # edx = pid
      SYSCALL(16)             # assignat arbitrariament
      
      popl %ebp
      ret


# ################# #
# ### PARCIAL 1 ### #
# ################# #

ENTRY(read)
    pushl %ebp          # dynamic link
    movl %esp, %ebp
                        # no need to save edx
    movl 8(%ebp), %edx  # edx = char* b
    int $130            # SYSCALL custom for interrupt 130
    
    cmpl $0, %eax       # check if error
    jge read_err_fi
    pushl %eax          # pass error value to errno
    call set_errno      # set errno to provided value
    addl $4, %esp       # remove parameter from stack once call finished
    movl $-1, %eax      # set return value to -1 as specified
read_err_fi:
    popl %ebx           # restore registers
    ret