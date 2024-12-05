/*
 * sys.c - Syscalls implementation
 */
#include "types.h"
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>
#include <keyboard.h>
#include <interrupt.h>


#define LECTURA 0
#define ESCRIPTURA 1

void * get_ebp();

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES; 
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS; 
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;
int global_TID=1;

int ret_from_fork()
{
  return 0;
}

int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;
  
  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);
  
  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  for (pag=0; pag<NUM_PAG_DATA; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);
      
      /* Return error */
      return -EAGAIN; 
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  unsigned int temp_logical = TOTAL_PAGES-1;
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, temp_logical, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((temp_logical)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, temp_logical);
    /* Deny access to the child's memory space */
    set_cr3(get_DIR(current()));
  }
  // set_cr3(get_DIR(current()));
  
  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return uchild->task.PID;
}

#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes) {
char localbuffer [TAM_BUFFER];
int bytes_left;
int ret;
	
	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{  
  int i;

  page_table_entry *process_PT = get_PT(current());

  // Deallocate all the propietary physical pages
  for (i=0; i<NUM_PAG_DATA; i++)
  {
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
  }
  
  /* Free task_struct */
  list_add_tail(&(current()->list), &freequeue);
  
  current()->PID=-1;
  
  /* Restarts execution of the next process */
  sched_next_rr();
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}




// empty

// Si estem fora del rang en alguna coordenada canviarem el valor a la coordenada valida més proxima
int sys_gotoxy(int x, int y) {
  if (x >= NUM_COLUMNS) x = NUM_COLUMNS - 1;
  else if (x < 0) x = 0;

  if (y >= NUM_ROWS) y = NUM_ROWS - 1;
  else if (y < 0) y = 0;

  setCursor(x, y);

  return 0;
}

//No se si fer-ho que transformi rgb al que tenim?
int sys_changecolour(int fg, int bg) {
  screenColor = (bg&0x0F)<<4 | (fg&0x0F);
  return 0;
}

int sys_clrscr(char *b) {
  //int inc;
  //printkint((int)get_ebp() - (int)current());
  //Word emptyChar = 0x0000;
  //Word newScreen[25][80];
  if (access_ok(VERIFY_READ, b, NUM_ROWS*NUM_COLUMNS*sizeof(Word))) { // si el punter es valid
    
    copy_from_user(b, (Word*)0xb8000, NUM_COLUMNS*NUM_ROWS*sizeof(Word));
    /*
    //copy_from_user(b, newScreen, NUM_COLUMNS*NUM_ROWS*sizeof(Word));
    //inc = 2;
    int sizeRow = NUM_COLUMNS*sizeof(Word);
    for (int i = 0; i < NUM_ROWS; ++i) {
      char row[sizeRow];
      copy_from_user(&b[sizeRow*i], row, sizeRow);
      for (int j = 0; j < sizeRow; j += sizeof(Word)) {
        printc_colour(row[j], row[j + 1]);
      }
    }*/
  } else { // si no default pantalla buida
    Word *screen = (Word*)0xb8000;
    for (int i = 0; i < NUM_ROWS; ++i) {
      for (int j = 0; j < NUM_COLUMNS; ++j) {
        *screen = 0x0000;
        ++screen;
      }
    }
    /*//inc = 0;
    for (int i = 0; i < 25; ++i) {
      for (int j = 0; j < 80; ++j) {
        newScreen[i][j] = 0x0000;
      }
    }*/
  }
  
  //int act = 0;
/*  setCursor(0, 0);
  
  // implementacions varies
  
  
  for (int i = 0; i < 25; ++i) { 
    for (int j = 0; j < 80; ++j) {
      printc_color((Byte)(newScreen[i][j]&0xFF), (Byte)(newScreen[i][j]>>8&0xFF));
    }
  }
  */
  //for (int act = 0; act < NUM_COLUMNS*NUM_ROWS; ++act) printc_color((Byte)newScreen[act]&0xFF, (Byte)((newScreen[act]>>8))&0xFF);
/*
  for (int act = 0; act < NUM_COLUMNS*NUM_ROWS*sizeof(Word); act += sizeof(Word)) {
    printc_color(b[act], b[act + 1]);
  }
  
  for (int i = 0; i < 25; ++i) {
    for (int j = 0; j < 80; ++j) {
      printc_color(b[act], b[act + 1]);
      act += inc;
    }
  }*/


  return 0;
}

int sys_getkey(char* b, int timeout) {
  if (!access_ok(VERIFY_WRITE, b, sizeof(char))) return -EFAULT;
  
  if (kbuf_pop(&kbuf, b)) return 0;
  
  // ordenar llista per timeout (ens deixa com ho tenim pero no li mola)
  current()->p_stats.blocked_ticks = timeout*TICKS_PER_SECOND;
  update_process_state_rr(current(), &keyboard_blocked);
  sched_next_rr();
  
  if (kbuf_pop(&kbuf, b)) return 0;

  return -ETIME;
}

int sys_semcreate() {
  return 0;
}

int sys_semwait() {
  return 0;
}

int sys_semsignal() {
  return 0;
}

int sys_semdestroy() {
  return 0;
}


int aux_thread() {
  printkhex((unsigned long)(get_PT(current())[286].entry));
  return 0;
}

int sys_threadcreatewithstack(void (*function)(void* arg), int N, void* parameter, void* wrapper) {
  struct task_struct* parent = current();
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;
  
  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(parent, uchild, sizeof(union task_union));
  
  /* new pages dir */
  // allocate_DIR((struct task_struct*)uchild);
  
  
  page_table_entry *process_PT = get_PT(&uchild->task);
  page_table_entry *parent_PT = get_PT(parent);
  
  process_PT->bits.pbase_addr = parent_PT->bits.pbase_addr;
  
  
  int stack_page = PAG_LOG_INIT_DATA+NUM_PAG_DATA; // +1 pq no se solapi amb l'stack del proces pare i "funcioni", un cop vagi s'ha de treure
  printkf("[KERNEL] parent: 0x%p; new: 0x%p\n", parent, &uchild->stack);
  printkf("[KERNEL] Searching page from %d\n", &stack_page);
  int found = 0;
  // Last page is reserved by sys_fork
  while (found < N && stack_page < TOTAL_PAGES-1) {
    if (parent_PT[stack_page].entry == 0) found += 1;
    else found = 0;
    stack_page += 1;
  }
  // no available consecutive pages, abort
  if (found < N) return -ENOMEM;

  printkf("[KERNEL] Found pages until %d\n", &stack_page);
  // set pag to start of region
  stack_page -= N;
  printkf("[KERNEL] New pages start %d\n", &stack_page);
  
  // region found, alloc pages
  for (int i = 0; i < N; i++) {
    int frame = alloc_frame();
    if (frame > 0) {
      int page = stack_page+i;
      printkf("[KERNEL] Assigned page %d to frame %d\n", &page, &frame);
      set_ss_pag(process_PT, stack_page+i, frame);
      continue;
    }
    
    // not enough physical pages, abort
    while (i > 0) {
      i -= 1;
      free_frame(get_frame(process_PT, stack_page+i));
      del_ss_pag(process_PT, stack_page+i);
    }
    set_cr3(get_DIR(parent));
    return -ENOMEM;
  }

  uchild->task.TID=++global_TID;
  uchild->task.state=ST_READY;
  
  // setup user and system stack
  unsigned long* user_stack = (unsigned long*)(long)(stack_page << 12);
  int USER_STACK_SIZE = N * 1024;
  user_stack[USER_STACK_SIZE - 3] = 0; // return address will never be reached
  user_stack[USER_STACK_SIZE - 2] = (unsigned long)function;
  user_stack[USER_STACK_SIZE - 1] = (unsigned long)parameter;  
  
  uchild->stack[KERNEL_STACK_SIZE - 5] = (unsigned long)wrapper; // eip
  uchild->stack[KERNEL_STACK_SIZE - 2] = (unsigned long)&user_stack[USER_STACK_SIZE - 3]; // esp
  uchild->task.register_esp = (int)(long)&uchild->stack[KERNEL_STACK_SIZE - 18]; // ebp

  // TODO: Modify kernel structures to account for allocated region (access_ok)
  // OPTION 1: Create a `struct task_threads protected_task_threads[NR_TASKS+2]`, where common attributes between threads are stored (allocated_size, num_threads)
  // OPTION 2: Reserve a page accessible from all threads with these attributes
  
  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return 0;
}

int sys_memregget() {
  return 0;
}

int sys_memregdel() {
  return 0;
}