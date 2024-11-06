/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <types.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct task_struct {
  int PID;			/* Process ID. This MUST be the first field of the struct. */
  page_table_entry * dir_pages_baseAddr;
  DWord kernel_esp;
  struct list_head list; /* Anchor to the `ready` queue. */

  struct list_head children; /* List of children of this process. */
  struct list_head parent_list; /* Anchor to the parent's children list.  */
  struct task_struct* parent; /* Parent of this process. */

  enum state_t state; /* Scheduling state of the process. */
  int quantum; /* Default quantum assigned to the process. */
  int pending_unblocks; /* If > 0, this process is blocked. */
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */


#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

void task_switch(union task_union*t);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

/* Headers for the scheduling policy */
void schedule();
void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();

// custom code

extern TSS tss;

extern void save_esi_edx_ebx();
extern void inner_task_switch(union task_union * new);
extern void restore_esi_edx_ebx();


extern struct task_struct *idle_task;
extern int remaining_quantum;

/// Free spaces in the tasks list.
extern struct list_head freequeue;
extern struct list_head readyqueue;

void init_freequeue();

int get_new_PID();

int* get_ebp();

#endif  /* __SCHED_H__ */