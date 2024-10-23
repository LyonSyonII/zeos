#include <asm.h>
#include <segment.h>


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

    movl 8(%ebp), %eax # eax = &new
    addl $0x1000, %eax # %eax = &new.stack[1024]

    movl %eax, tss+4 # tss.esp0 = &new.stack[1024]
    
    pushl %eax
    pushl $0x175
    call writeMsr # Msr[0x175] = &new.stack[1024]
    addl $8, %esp

    movl 8(%ebp), %eax
    pushl 4(%eax) # push new.task.dir_pages_baseAddr
    call set_cr3 # cr3 = new task dir table
    addl $4, %esp

    movl %esp, %eax
    andl $0xfffff000, %eax # &current
    movl %ebp, 8(%eax) # ebp -> current.kernel_esp
    
    movl 8(%ebp), %eax # &new.task_struct -> %eax
    movl 8(%eax), %esp # esp <- new.kernel_esp

    popl %ebp
    ret