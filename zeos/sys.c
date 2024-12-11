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
    copy_data((void*)(long)(pag<<12), (void*)(long)((temp_logical)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, temp_logical);
    /* Deny access to the child's memory space */
    set_cr3(get_DIR(current()));
  }
  
  // Copy parent's allocated pages to child
  struct list_head* element;
  list_for_each(element, &current()->allocated_pages_list) {
    struct page_metadata* metadata = list_entry(element, struct page_metadata, list);
    if (metadata->marker != METADATA_MARKER) break;

    unsigned int metadata_page = (long)metadata >> 12;
    
    for (int i = 0; i < metadata->size; i++) {
      int frame = alloc_frame();
      // If error, revert process up to memory region that failed
      if (frame < 0) {
        // Free data frames
        for (int i = 0; i < NUM_PAG_DATA; i++) free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA + i));
        // Free extra allocated frames
        struct list_head* element2;
        list_for_each(element2, &current()->allocated_pages_list) {            
          metadata = list_entry(element2, struct page_metadata, list);
          metadata_page = (long)metadata >> 12;
          for (int j = 0; j < metadata->size; j++) {
            if (element2 == element && j == i) {
              list_add(lhcurrent, &freequeue);
              return -EAGAIN;
            }
            free_frame(get_frame(process_PT, metadata_page+j));
            del_ss_pag(process_PT, metadata_page+j);
          }
        }
      }

      int page = metadata_page+i;
      printkf("[sys_fork] Copying page %d\n", &page);

      set_ss_pag(process_PT, page, frame);
      set_ss_pag(parent_PT, temp_logical, frame);
      copy_data((void*)(long)(page<<12), (void*)(long)(temp_logical<<12), PAGE_SIZE);
      del_ss_pag(parent_PT, temp_logical);
      /* Deny access to the child's memory space */
      set_cr3(get_DIR(current()));
    }
  }

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;
  uchild->task.allocated_pages_list = current()->allocated_pages_list;

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

void thread_exit(struct task_struct* process) {
  page_table_entry *process_PT = get_PT(process);

  // Deallocate the stack of this thread
  int stack_end_page = process->stack_start_page + process->stack_num_pages;
  printkf("[sys_exit] Exiting thread PID = %d; TID = %d;\n", &process->PID, &process->TID);
  
  // Deallocate dynamic pages
  struct list_head *element, *n;
  list_for_each_safe(element, n, &process->allocated_pages_list) {
    printkf("[sys_exit] Going to first element of allocated list\n");
    struct page_metadata* metadata = list_entry(element, struct page_metadata, list);
    int start_page = (long)metadata >> 12;
    printkf("[sys_exit] Freeing dynamic pages from %d", &start_page);
    int end_page = start_page + metadata->size;
    printkf("to %d\n", &start_page, &end_page);
    if (metadata->marker != METADATA_MARKER) break;
    
    dealloc_pages(process, start_page, metadata->size);
  }
  
  printkf("[sys_exit] Freeing pages %d to %d\n", &process->stack_start_page, &stack_end_page);
  // Deallocate stack
  for (int i = process->stack_start_page; i < stack_end_page; i++) {
    free_frame(get_frame(process_PT, i));
    del_ss_pag(process_PT, i);
  }
  set_cr3(get_DIR(process));
  
  process->PID=-1;
  process->TID=-1;

  /* Free task_struct */
  list_add_tail(&process->list, &freequeue);
}

void sys_exit() {
  struct task_struct* process = current();
  printkf("[sys_exit] Exiting process PID = %d; TID = %d;\n", &process->PID, &process->TID);
  //page_table_entry *process_PT = get_PT(process);
  
  // If main process, exit all threads with same PID
  if (process->TID == 0) {
    // Iterate over all tasks, so even if a thread is blocked it's deleted correctly
    // First two (protected, idle) and last (protected) are reserved
    for (int i = 2; i < NR_TASKS-1; i++) {
      union task_union* thread = &protected_tasks[i];
      if (thread->task.PID != process->PID || thread->task.TID == 0) continue;
      list_del(&thread->task.list); // must be in freequeue or other list
      thread_exit(&thread->task);
    }
  }
  
  // Deallocate all the propietary physical pages and free task struct
  thread_exit(process);

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



// Si estem fora del rang en alguna coordenada canviarem el valor a la coordenada valida més proxima
int sys_gotoxy(int x, int y) {
  if (x >= NUM_COLUMNS || x < 0 || y >= NUM_ROWS || y < 0) return -EINVAL;

  setCursor(x, y);

  return 0;
}

int sys_changecolor(int fg, int bg) {
  if (fg < 0 || bg < 0) return -EINVAL;

  screenColor = (bg&0x0F)<<4 | (fg&0x0F);
  return 0;
}

int sys_clrscr(char *b) {
  if (b == NULL) {
    Word *screen = (Word*)0xb8000;
    for (int i = 0; i < NUM_ROWS; ++i) {
      for (int j = 0; j < NUM_COLUMNS; ++j) {
        *screen = 0x0000;
        ++screen;
      }
    }
    return 0;
  }

  if (!access_ok(VERIFY_READ, b, NUM_ROWS*NUM_COLUMNS*sizeof(Word))) {
    return -EFAULT;
  }

  copy_from_user(b, (Word*)0xb8000, NUM_COLUMNS*NUM_ROWS*sizeof(Word));

  return 0;
}

int sys_getkey(char* b, int timeout) {
  if (!access_ok(VERIFY_WRITE, b, sizeof(char))) return -EFAULT;
  
  if (kbuf_pop(&kbuf, b)) return 0;
  
  // TODO(fix): La llista de procesos bloquejats per timeout no està ordenada, i només desbloquejeu al primer sense comprovar si cal o no...
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
  sem->creator_TID = current()->TID;
  sem->count = initial_value;

  // printkf("[sys_semcreate] Returning sem to the user: 0x%p\n", sem);
  
  return sem;
}

int sem_ptr_ok(struct sem_t* s) {
  // User shouldn't be able to access `s`, if it can it's probably a security hole
  // Check if `s` points where it should 
  return s >= &semaphores[0] && s <= &semaphores[NR_TASKS];
}

// Decrement the semaphore’s counter and block the current thread if the counter is negative
int sys_semwait(struct sem_t* s) {
  // printkf("[sys_semwait] Received s: 0x%p\n", s);
  
  if (!sem_ptr_ok(s)) return -EFAULT;

  s->count -= 1;
  if (s->count < 0) {
    // printkf("[sys_semwait] Sem count is negative, blocking thread...\n");
    update_process_state_rr(current(), &s->blocked);
    sched_next_rr();
  }

  return 0;
}

// Increase the semaphores's counter and unblock the first blocked thread in the semaphore's queue
int sys_semsignal(struct sem_t* s) {
  // printkf("[sys_semwait] Received s: 0x%p\n", s);
  if (!sem_ptr_ok(s)) return -EFAULT;

  s->count += 1;
  if (list_empty(&s->blocked)) return 0;

  struct list_head* first = list_first(&s->blocked);
  struct task_struct* task = list_entry(first, struct task_struct, list);
  update_process_state_rr(task, &readyqueue);

  return 0;
}

// Destroy the semaphore (only the thread that created a semaphore can destroy it)
int sys_semdestroy(struct sem_t* s) {
  if (!sem_ptr_ok(s)) return -EFAULT;
  if (s->creator_TID != current()->TID) return -EINVAL;
  
  // unblock all threads
  struct list_head *entry, *n;
  list_for_each_safe(entry, n, &s->blocked) {
    struct task_struct* task = list_entry(entry, struct task_struct, list);
    update_process_state_rr(task, &readyqueue);
  }

  // TODO: Add marker to task to know when a semaphore has been destroyed and skip waiting?
  
  // free semaphore
  list_add_tail(&s->list, &semqueue);
  
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
  INIT_LIST_HEAD(&uchild->task.allocated_pages_list);
  
  // setup user and system stack
  unsigned long* user_stack = (unsigned long*)(long)(stack_page << 12);
  unsigned long USER_STACK_SIZE = N * 1024;
  user_stack[USER_STACK_SIZE - 3] = 0; // return address will never be reached
  user_stack[USER_STACK_SIZE - 2] = (unsigned long)function;
  user_stack[USER_STACK_SIZE - 1] = (unsigned long)parameter;  
  
  uchild->stack[KERNEL_STACK_SIZE - 5] = (unsigned long)wrapper; // eip
  uchild->stack[KERNEL_STACK_SIZE - 2] = (unsigned long)&user_stack[USER_STACK_SIZE - 3]; // esp
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

  // TODO: Metadata is written in the first allocated page until other method is found
  // allocate extra page for metadata
  int first_page = alloc_pages(current(), num_pages+1);
  struct page_metadata* metadata = (struct page_metadata*)(long)(first_page << 12);
  *metadata = new_page_metadata(num_pages+1);
  
  list_add_tail(&metadata->list, &current()->allocated_pages_list);
  // return skipping metadata page
  return (char*)(long)((first_page+1) << 12);
}

// This call deletes a previously allocated memory region m, releasing all its resources.
int sys_memregdel(char* m) {
  if (m == NULL) return -EFAULT;
  
  // [m] = { metadata, PAGE_SIZE * metadata->size }
  struct page_metadata* metadata = (struct page_metadata*)(m - PAGE_SIZE);
  if (!metadata_ptr_ok(metadata)) return -EFAULT;
  
  list_del(&metadata->list);
  dealloc_pages(current(), (long)(metadata) >> 12, metadata->size);
  return 0;
}