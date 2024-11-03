/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include "entry.h"
#include "list.h"
#include "types.h"
#include <sched.h>
#include <mm.h>
#include <io.h>


union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));


struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void)
{
	struct list_head *lh = list_first(&freequeue); //Agafem la primera entrada de la freequeue
	list_del(lh); //Borrem aquesta entrada de la freequeue
	
	union task_union *tu = list_entry(lh, union task_union, task.list); //agafem la task union que correspon
	tu->task.PID = 0; //assignem PID corresponent

	allocate_DIR(&tu->task); //assignem un nou directori on guardar les adreces

	tu->stack[1023] = (DWord)cpu_idle; // @return
	tu->stack[1022] = 0; // ebp = 0
	tu->task.kernel_esp = (DWord)&tu->stack[1022]; // assignem la posició del esp que apunta a dalt de tot de la pila de sistema

	idle_task = &tu->task; //col·loquem a idle_task l'adreça del task_struct de idle
}

void init_task1(void)
{
	struct list_head *lh = list_first(&freequeue); //Agafem la primera entrada de la freequeue
	list_del(lh); //Borrem aquesta entrada de la freequeue
	
	union task_union *task1 = list_entry(lh, union task_union, task.list); //agafem la task_union que correspon
	task1->task.PID = 1; //assignem PID que toca
	
	allocate_DIR(&task1->task); //assignem taula de directoris
	set_user_pages(&task1->task); //Assignem les pagines fisiques necessaries per guardar dades i codi del process
	
	tss.esp0 = KERNEL_ESP(task1); //escribim a TSS l'adreça del stack
	task1->task.kernel_esp = KERNEL_ESP(task1);
	writeMsr(0x175, KERNEL_ESP(task1)); //escribim a Msr 0x175 l'adreça del stack
	
	set_cr3(task1->task.dir_pages_baseAddr); //Col·loquem a cr3 l'adreça de la taula de directoris del process
}


void init_sched()
{
	init_freequeue();
	INIT_LIST_HEAD(&readyqueue);
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}


//custom code


void task_switch(union task_union*t) {
	save_esi_edx_ebx();

	printk("pre_inner ");
	printkint(current()->PID);

/*  movl 8(%ebp), %eax # eax = &new
    addl $0x1000, %eax # %eax = &new.stack[1024]

    movl %eax, tss+4 # tss.esp0 = &new.stack[1024]

    pushl %eax
    pushl $0x175
    call writeMsr # Msr[0x175] = &new.stack[1024]
    addl $8, %esp

    movl 8(%ebp), %eax
    pushl 4(%eax) # push new.task.dir_pages_baseAddr
    call set_cr3 # cr3 = new task dir table
    addl $4, %esp */
	tss.esp0 = (DWord)&t->stack[1024];
	writeMsr(0x175, (DWord)&t->stack[1024]);
	set_cr3(t->task.dir_pages_baseAddr);

	inner_task_switch(t);

	printk("post_inner ");
	printkint(current()->PID);
	
	restore_esi_edx_ebx();
}

void idle_switch() {
	task_switch((union task_union*)idle_task);
}

void switch_to_next_task() {
	if (list_empty(&readyqueue)) {
		return idle_switch();
	}
	struct list_head *first = list_first(&readyqueue);
	list_del(first);
	union task_union *first_entry = list_entry(first, union task_union, task.list);
	if (current() != idle_task) {
		list_add_tail(&current()->list, &readyqueue);
	}
	task_switch(first_entry);
}

int get_new_PID() {
  	static int PID = 1;
	PID += 1;
	return PID;
}

struct task_struct *idle_task;
struct list_head freequeue;
struct list_head readyqueue;

void init_freequeue() {
	INIT_LIST_HEAD(&freequeue);
	for (int i = 0; i < NR_TASKS; i++) {
		list_add_tail(&task[i].task.list, &freequeue);
	}
}