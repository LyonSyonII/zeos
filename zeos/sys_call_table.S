#include <asm.h>
#include <segment.h>

ENTRY (sys_call_table)
	.long sys_ni_syscall	//0
	.long sys_exit		//1
	.long sys_fork		//2
	.long sys_threadcreatewithstack	//3
	.long sys_write		//4
	.long sys_gotoxy	//5
	.long sys_changecolour	//6
	.long sys_clrscr	//7
	.long sys_ni_syscall	//8
	.long sys_ni_syscall	//9
	.long sys_gettime	//10
	.long sys_ni_syscall	//11
	.long sys_ni_syscall	//12
	.long sys_yield		//13
	.long sys_ni_syscall	//14
	.long sys_ni_syscall	//15
	.long sys_ni_syscall	//16
	.long sys_ni_syscall	//17
	.long sys_ni_syscall	//18
	.long sys_ni_syscall	//19
	.long sys_getpid	//20	
	.long sys_getkey	//21
	.long sys_ni_syscall	//22
	.long sys_ni_syscall	//23
	.long sys_ni_syscall	//24
	.long sys_ni_syscall	//25
	.long sys_ni_syscall	//26
	.long sys_ni_syscall	//27
	.long sys_semcreate	//28
	.long sys_semwait	//29
	.long sys_semsignal	//30
	.long sys_semdestroy	//31
	.long sys_memregget	//32
	.long sys_memregdel	//33
	.long sys_ni_syscall	//34
	.long sys_get_stats	//35
.globl MAX_SYSCALL
MAX_SYSCALL = (. - sys_call_table)/4
