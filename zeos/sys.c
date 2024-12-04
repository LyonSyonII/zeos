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
  setCursor(0, 0);
  int act = 0;
  if (access_ok(VERIFY_READ, b, NUM_ROWS*NUM_COLUMNS*sizeof(Word))) { // si el punter es valid
    //copy_from_user(b, newScreen, NUM_COLUMNS*NUM_ROWS*sizeof(Word));
    //inc = 2;
    int sizeRow = NUM_COLUMNS*sizeof(Word);
    for (int i = 0; i < NUM_ROWS; ++i) {
      char row[sizeRow];
      copy_from_user(&b[sizeRow*i], row, sizeRow);
      for (int j = 0; j < sizeRow; j += sizeof(Word)) {
        printc_colour(row[j], row[j + 1]);
      }
    }
  } else { // si no default pantalla buida
    for (int i = 0; i < NUM_ROWS; ++i) {
      for (int j = 0; j < NUM_COLUMNS; ++j) {
        printc_colour(0, 0);
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

  // EN PRINCIPI ARREGLAT AMB EXECUTAR IMMEDIATAMENT
  // proces 1 : bloquejat
  // cliquem tecla
  // desbloquejem proces 1
  // continuem a proces 2
  // proces 2 : entra a getkey
  // es menja la tecla
  // proces 1 : desbloquejat pero no te tecla i retornara erroniament un -1

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

int sys_threadcreatewithstack(void (*function)(void* arg), int N, void* parameter, void* wrapper) {
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
  // allocate_DIR((struct task_struct*)uchild);
  

  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  // ALLOCATE STACK
  new_ph_pag = alloc_frame();
  if (new_ph_pag <= 0) return -ENOMEM;
  set_ss_pag(process_PT, PAG_LOG_INIT_DATA+NUM_PAG_DATA, new_ph_pag);
  
  // set_cr3(get_DIR(current()));

  uchild->task.TID=++global_TID;
  uchild->task.state=ST_READY;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)function;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return 0;
}


int sys_threadcreatewithstackk(void (*function)(void* arg), int N, void* parameter, void* wrapper) {
  // return function must exist
  if (function == NULL) return -EINVAL;
  // thread must have at least one stack page
  if (N <= 0) return -EINVAL;
  // abort if no task struct available
  if (list_empty(&freequeue)) return -ENOMEM;
  
  struct task_struct* parent = current();

  // get task struct from the freequeue
  struct list_head* new_lh = list_first(&freequeue);
  list_del(new_lh);
  union task_union* new_tu = (union task_union*)list_head_to_task_struct(new_lh);
  
  // copy whole stack from parent
  copy_data(parent, new_tu, sizeof(union task_union));
  
  // search Page Table for N consecutive pages
  // (TOTAL_PAGES-1) is reserved by sys_fork
  page_table_entry* PT = get_PT(parent);
  int pag = PAG_LOG_INIT_DATA+NUM_PAG_DATA;
  printkf("[KERNEL] parent: 0x%p; new: 0x%p\n", parent, &new_tu->stack);
  printkf("[KERNEL] Searching page from %d\n", &pag);
  int found = 0;
  while (found < N && pag < TOTAL_PAGES-1) {
    if (PT[pag].entry == 0) found += 1;
    else found = 0;
    pag += 1;
  }
  // no available consecutive pages, abort
  if (found < N) return -ENOMEM;
  printkf("[KERNEL] Found pages until %d\n", &pag);
  // set pag to start of region
  pag -= N;
  printkf("[KERNEL] New pages start %d\n", &pag);

  // region found, alloc pages
  for (int i = 0; i < N; i++) {
    int frame = alloc_frame();
    if (frame > 0) {
      int page = pag+i;
      printkf("[KERNEL] Assigned page %d to frame %d\n", &page, &frame);
      set_ss_pag(PT, pag+i, frame);
      continue;
    }
    
    // not enough physical pages, abort
    while (i > 0) {
      i -= 1;
      free_frame(get_frame(PT, pag+i));
      del_ss_pag(PT, pag+i);
    }
    set_cr3(get_DIR(parent));
    return -ENOMEM;
  }
  
  // new thread shares directory with parent
  // DATA, SYSTEM and CODE pages are shared
  // new_tu->task.dir_pages_baseAddr = parent->dir_pages_baseAddr;
  
  // Initialize unique fields
  init_stats(&new_tu->task.p_stats);
  global_TID += 1;
  new_tu->task.TID = global_TID;
  
  int base_addr = (pag+1)<<12;
  
  new_tu->stack[KERNEL_STACK_SIZE-5] = (unsigned long)wrapper; // eip
  new_tu->stack[KERNEL_STACK_SIZE-2] = base_addr - 2*sizeof(void *); // esp
  new_tu->task.register_esp = (unsigned long int)&new_tu->stack[KERNEL_STACK_SIZE-18 /* stack offset */];
  
  unsigned long* stack = &new_tu->stack[KERNEL_STACK_SIZE];
  void** base = (void**)(base_addr - sizeof(void*));
  printkf("Stack: %p; Base: %p;\n", stack, base);
  // new_tu->stack[KERNEL_STACK_SIZE] = (long) parameter;
  // new_tu->stack[KERNEL_STACK_SIZE-1] = (long) function;
  *(void**)(base_addr - sizeof(void*)) = parameter;
  *(void**)(base_addr - 2*sizeof(void*)) = function;
  
  new_tu->task.state = ST_READY;
  // list_add_tail(&new_tu->task.list, &readyqueue);
  
  printkf("[KERNEL] Forcing task switch\n");
  list_add(&new_tu->task.list, &readyqueue);
  force_task_switch();
  
  return 0;
}


int sys_memregget() {
  return 0;
}

int sys_memregdel() {
  return 0;
}