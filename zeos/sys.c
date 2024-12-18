/*
 * sys.c - Syscalls implementation
 */
#include "list.h"
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
  if (fd!=1 && fd!=2) return -EBADF; 
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
int global_TID=0;

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
  for (pag=0; pag<NUM_PAG_KERNEL; pag++) {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++) {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  unsigned int temp_logical = TOTAL_PAGES-1;
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++) {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, temp_logical, get_frame(process_PT, pag));
    copy_data((void*)(long)(pag<<12), (void*)(long)(temp_logical<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, temp_logical);
    /* Deny access to the child's memory space */
    set_cr3(get_DIR(current()));
  }
  
  /* Copy all of parent's used pages */
  for (pag = PAG_LOG_INIT_DATA + NUM_PAG_DATA; pag < TOTAL_PAGES; pag++) {
    // if unused do not copy
    if (parent_PT[pag].entry == 0) continue;
    
    int frame = alloc_frame();
    if (frame > 0) {
      printkf("[KERNEL] Assigned page %d to frame %d\n", &pag, &frame);
      set_ss_pag(process_PT, pag, frame);
      set_ss_pag(parent_PT, temp_logical, frame);
      copy_data((void*)(long)(pag<<12), (void*)(long)(temp_logical << 12), PAGE_SIZE);
      del_ss_pag(parent_PT, temp_logical);
      set_cr3(get_DIR(current()));
    } else {
      // Free pages up to the one that failed
      dealloc_pages(&uchild->task, PAG_LOG_INIT_DATA+NUM_PAG_DATA, pag-1, 1);
      // Dealloc task_struct
      list_add_tail(lhcurrent, &freequeue);
      return -EAGAIN;
    }
  }  
  
  // volatile int* a = 0; *a;
  
  uchild->task.PID=++global_PID;
  uchild->task.TID=0;
  uchild->task.first_allocated_page=NULL;

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
    if (fd == 2) { 
      ret = sys_write_bochs(localbuffer, TAM_BUFFER); 
    } else { 
      ret = sys_write_console(localbuffer, TAM_BUFFER);
    }
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
    if (fd == 2) { 
      ret = sys_write_bochs(localbuffer, bytes_left); 
    } else { 
      ret = sys_write_console(localbuffer, bytes_left);
    }
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void thread_exit(struct task_struct* process) {
  page_table_entry* process_PT = get_PT(process);
  
  // Deallocate the stack of this thread
  int stack_end_page = process->stack_start_page + process->stack_num_pages - 1;
  printkf("[sys_exit] Exiting thread PID = %d; TID = %d;\n", &process->PID, &process->TID);
  printkf("[sys_exit] Freeing stack pages %d to %d\n", &process->stack_start_page, &stack_end_page);
  dealloc_pages(process, process->stack_start_page, process->stack_num_pages, 1);

  // Deallocate dynamic pages
  for (int pag = PAG_LOG_INIT_DATA+NUM_PAG_DATA; pag < TOTAL_PAGES-1; pag++) {
    page_table_entry* entry = &process_PT[pag];
    // If not allocated or user-accessible, skip
    if (!entry->bits.present || entry->bits.user) continue;
    
    // Get metadata from page and ensure it's correct
    struct page_metadata* metadata = (struct page_metadata*)(long)(pag << 12);
    if (!metadata_ptr_ok(metadata)) continue;
    
    // If page is not from this process, skip
    if (metadata->parent_PID != process->PID) continue;
    if (metadata->parent_TID != process->TID) continue;

    // Free pages from this allocation
    int start_page = (long)metadata >> 12;
    int end_page = start_page + metadata->size - 1;
    printkf("[sys_exit] Freeing dynamic pages from %d to %d\n", &start_page, &end_page);
    dealloc_pages(process, start_page, metadata->size, 0);
    // skip deallocated pages
    pag = end_page;
  }
  
  process->PID=-1;
  process->TID=-1;
  
  /* Free task_struct */
  list_add_tail(&process->list, &freequeue);
}

void sys_exit() {
  struct task_struct* process = current();
  printkf("[sys_exit] Exiting process PID = %d; TID = %d;\n", &process->PID, &process->TID);
  
  // If main process, exit all threads with same PID
  if (process->TID == 0) {
    // Iterate over all tasks, so even if a thread is blocked it's deleted correctly
    // First two (protected, idle) are reserved
    for (int i = 2; i < NR_TASKS+1; i++) {
      struct task_struct* thread = &protected_tasks[i].task;
      if (thread->PID != process->PID) continue;
      
      // Free task_struct (must be in freequeue or other list)
      if (thread != process) list_del(&thread->list);
      list_add_tail(&thread->list, &freequeue);
      thread->PID=-1;
      thread->TID=-1;
    }
    // Free all pages
    dealloc_pages(process, PAG_LOG_INIT_DATA, TOTAL_PAGES-PAG_LOG_INIT_DATA, 1);
  } else {
    // Deallocate all the propietary physical pages and free task struct
    thread_exit(process);
  }

  // Restarts execution of the next process
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



int sys_gotoxy(int x, int y) {
  if (x >= NUM_COLUMNS || x < 0 || y >= NUM_ROWS || y < 0) return -EINVAL;

  setCursor(x, y);

  return 0;
}

int sys_changecolor(int fg, int bg) {
  if (fg < 0  || fg > 0x0F || bg < 0 || bg > 0x0F) return -EINVAL;

  screenColor = (bg&0x0F)<<4 | (fg&0x0F);
  return 0;
}

int sys_clrscr(char *b) {
  Word *screen = (Word*)0xb8000;
  if (b == NULL) {
    for (int cnt = 0; cnt < NUM_COLUMNS*NUM_ROWS; ++cnt) {
      *screen = 0x0000;
      ++screen;
    }
    return 0;
  }

  if (!access_ok(VERIFY_READ, b, NUM_ROWS*NUM_COLUMNS*sizeof(Word))) {
    return -EFAULT;
  }

  return copy_from_user(b, screen, NUM_COLUMNS*NUM_ROWS*sizeof(Word));
}

int sys_getkey(char* b, int timeout) {
  if (!access_ok(VERIFY_WRITE, b, sizeof(char))) return -EFAULT;
  
  if (kbuf_pop(&kbuf, b)) return 0;
  
  current()->p_stats.blocked_ticks = timeout*TICKS_PER_SECOND;
  update_process_state_rr(current(), &keyboard_blocked);
  sched_next_rr();
  
  if (kbuf_pop(&kbuf, b)) return 0;

  return -ETIME;
}

// Create an initial semaphore with an initial counter of initial_value; 
// 
// The returned `sem_t` is unusable from user space.
struct sem_t* sys_semcreate(int initial_value) {
  if (list_empty(&semqueue)) return NULL;
  
  struct list_head* first = list_first(&semqueue);
  list_del(first);
  struct sem_t* sem = list_entry(first, struct sem_t, list);
  
  INIT_LIST_HEAD(&sem->blocked);
  sem->id = ++global_semaphore_id;
  sem->creator_TID = current()->TID;
  sem->count = initial_value;
  
  // printkf("[sys_semcreate] Created semaphore with id = %d, count = %d\n", &sem->id, &sem->count);
  return (struct sem_t*)(long)global_semaphore_id;
}

struct sem_t* get_sem_from_user_ptr(struct sem_t* s) {
  unsigned long sem_id = (unsigned long)(s);
  // User shouldn't be able to access `s`, if it can it's probably a security hole
  // Check if `s` points where it should 
  for (int i = 1; i < NR_TASKS+1; i++) {
    if (semaphores[i].id == sem_id) return &semaphores[i];
  }
  return NULL;
}

// Decrement the semaphore’s counter and block the current thread if the counter is negative
int sys_semwait(struct sem_t* s) {
  // convert id to sem ptr
  s = get_sem_from_user_ptr(s);
  if (s == NULL) return -EFAULT;

  // printkf("[sys_semwait] Waiting for semaphore with id = %d, count = %d\n", &s->id, &s->count);

  s->count -= 1;
  if (s->count < 0) {
    int sem_id = s->id;
    // printkf("[sys_semwait] Sem count is negative, blocking thread...\n");
    update_process_state_rr(current(), &s->blocked);
    sched_next_rr();
    // If woken up by semDestroy, return -1
    if (s->id != sem_id) return -EAGAIN;
  }
  return 0;
}

// Increase the semaphores's counter and unblock the first blocked thread in the semaphore's queue
int sys_semsignal(struct sem_t* s) {
  s = get_sem_from_user_ptr(s);
  if (s == NULL) return -EFAULT;

  s->count += 1;
  if (list_empty(&s->blocked)) return 0;

  struct list_head* first = list_first(&s->blocked);
  struct task_struct* task = list_entry(first, struct task_struct, list);
  update_process_state_rr(task, &readyqueue);

  return 0;
}

// Destroy the semaphore (only the thread that created a semaphore can destroy it)
int sys_semdestroy(struct sem_t* s) {
  s = get_sem_from_user_ptr(s);
  if (s == NULL) return -EFAULT;
  if (s->creator_TID != current()->TID) return -EINVAL;
  
  // unblock all threads
  struct list_head *entry, *n;
  list_for_each_safe(entry, n, &s->blocked) {
    struct task_struct* task = list_entry(entry, struct task_struct, list);
    update_process_state_rr(task, &readyqueue);
  }
  
  // free semaphore
  s->id = -1;
  list_add_tail(&s->list, &semqueue);
  
  return 0;
}

int sys_threadcreatewithstack(void (*function)(void* arg), int N, void* parameter, void* wrapper) {
  if (!access_ok(VERIFY_READ, function, sizeof(function))/*  || !access_ok(VERIFY_WRITE, parameter, sizeof(parameter)) */)
    return -EFAULT;

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

  // parent and child both share directory
  page_table_entry *parent_PT = get_PT(parent);
  page_table_entry *process_PT = get_PT(&uchild->task);
  process_PT->bits.pbase_addr = parent_PT->bits.pbase_addr;
  
  // allocate N consecutive pages
  int stack_page = alloc_pages(&uchild->task, N);
  if (stack_page < 0) { 
    list_add(lhcurrent, &freequeue);
    return stack_page; 
  }
  
  uchild->task.TID=++global_TID;
  uchild->task.stack_num_pages=N;
  uchild->task.stack_start_page=stack_page;
  uchild->task.state=ST_READY;
  uchild->task.first_allocated_page = NULL;
  
  // setup user and system stack
  unsigned long* user_stack = (unsigned long*)(long)(stack_page << 12);
  unsigned long USER_STACK_SIZE = N * 1024;
  // user_stack[USER_STACK_SIZE - 3] = 0; // return address will never be reached
  user_stack[USER_STACK_SIZE - 2] = (unsigned long)function;
  user_stack[USER_STACK_SIZE - 1] = (unsigned long)parameter;  
  
  uchild->stack[KERNEL_STACK_SIZE - 5] = (unsigned long)wrapper; // eip
  uchild->stack[KERNEL_STACK_SIZE - 2] = (unsigned long)&user_stack[USER_STACK_SIZE - 2]; // esp
  uchild->task.register_esp = (int)(long)&uchild->stack[KERNEL_STACK_SIZE - 18]; // ebp

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return 0;
}

// Allocates num_pages pages of physical memory and maps them to a consecutive region in the user address space. 
// Returns the initial logical address assigned to the region.
// This memory region is inherited by child processes (fork) and other threads.
char* sys_memregget(int num_pages) {
  if (num_pages <= 0) return NULL;
  
  // allocate extra page for metadata
  int first_page = alloc_pages(current(), num_pages+1);
  if (first_page < 0) return NULL;
  
  struct page_metadata* metadata = (struct page_metadata*)(long)(first_page << 12);
  *metadata = new_page_metadata(num_pages+1);
  page_table_entry* process_PT = get_PT(current());
  // disallow user accessing metadata
  process_PT[first_page].bits.user = 0;
  
  // return skipping metadata page
  return (char*)(long)((first_page+1) << 12);
}

// This call deletes a previously allocated memory region m, releasing all its resources.
int sys_memregdel(char* m) {
  if (m == NULL) return -EFAULT;
  if (!access_ok(VERIFY_WRITE, m, PAGE_SIZE)) return -EFAULT;
  
  // [m] = { metadata, PAGE_SIZE * metadata->size }
  struct page_metadata* metadata = (struct page_metadata*)(m - PAGE_SIZE);
  if (!metadata_ptr_ok(metadata)) return -EFAULT;
  dealloc_pages(current(), (long)(metadata) >> 12, metadata->size, 1);
  
  return 0;
}