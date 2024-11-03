#include <asm.h>
#include <segment.h>


ENTRY (get_ebp)
    movl %ebp, %eax
    ret


ENTRY (save_esi_edx_ebx)
    popl %eax # @ret -> %eax
    
    pushl %esi # guardem registres
    pushl %edx
    pushl %ebx

    pushl %eax # @ret -> stack
    ret


ENTRY (restore_esi_edx_ebx)
    popl %eax # @ret -> %eax
    
    popl %ebx # carreguem registres
    popl %edx
    popl %esi

    pushl %eax # @ret -> stack
    ret


ENTRY (inner_task_switch)
    pushl %ebp
    movl %esp, %ebp

    movl %esp, %eax
    andl $0xfffff000, %eax # &current
    movl %ebp, 8(%eax) # ebp -> current.kernel_esp

    movl 8(%ebp), %eax # &new.task_struct -> %eax
    movl 8(%eax), %esp # esp <- new.kernel_esp

    popl %ebp    
    ret