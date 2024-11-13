/*
 * sys.c - Syscalls implementation
 */
#include "keyboard.h"
#include "list.h"
#include <errno.h>
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <types.h>

#include <clock.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -EACCES; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid() {
	return current()->PID;
}

// Get Parent PID
int sys_getppid() {
  if (current()->parent == NULL) return -ESRCH;
  return current()->parent->PID;
}

int ret_from_fork() {
  return 0;
}

int sys_fork() {
  // creates the child process
  if (list_empty(&freequeue)) return -ENOMEM; //CANVIAR
  
  // get the first entry and remove it from the queue
  struct list_head *child_task_list_head = list_first(&freequeue);
  list_del(child_task_list_head);
  union task_union *child_task = list_entry(child_task_list_head, union task_union, task.list);
  struct task_struct *parent_task = current();
  
  // copy whole stack from parent to child
  copy_data((void*)parent_task, (void*)child_task, sizeof(union task_union));
  
  // allocate new directory for child
  if (allocate_DIR(&child_task->task) < 0) {
    list_add(child_task_list_head, &freequeue);
    return -ENOMEM;
  }

  // get parent and child Page Table addresses
  page_table_entry *child_PT = get_PT(&child_task->task);
  page_table_entry *parent_PT = get_PT(parent_task);

/*   // copy kernel pages
  for (int pag = 0; pag < NUM_PAG_KERNEL; ++pag) {
    // PAG_LOG_INIT_KERNEL == 1
    child_PT[1+pag].entry = parent_PT[1+pag].entry;
  } */

  // copy code pages
  for (int pag = 0; pag < NUM_PAG_CODE; ++pag) {
    child_PT[PAG_LOG_INIT_CODE+pag].entry = parent_PT[PAG_LOG_INIT_CODE+pag].entry;
  }
  
  // copy data pages
  for (int pag = 0; pag < NUM_PAG_DATA; ++pag) {
    int new_ph_pag = alloc_frame();
    if (new_ph_pag < 0) {
      list_add(child_task_list_head, &freequeue);
      return dealloc_user_pages(child_PT, 0, pag, -ENOMEM);
    }
    
    // assign page to child
    // child_data[pag] = new_pag;
    set_ss_pag(child_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    // map child page to parent empty page
    // parent_data[EMPTY_DATA_PAG] = child_data[pag];
    int EMPTY_DATA_PAG = PAG_LOG_INIT_DATA + NUM_PAG_DATA + NUM_PAG_CODE + pag;
    set_ss_pag(parent_PT, EMPTY_DATA_PAG, new_ph_pag);
    
    // copy data from parent page to mapped child page
    // *parent_data[EMPTY_DATA_PAG] = *parent_data[pag];
    int* parent_data_page = (int*)( (PAG_LOG_INIT_DATA + pag) << 12 );
    int* mapped_data_page = (int*) ( EMPTY_DATA_PAG << 12 );
    copy_data(parent_data_page, mapped_data_page, PAGE_SIZE);
    
    // delete mapped page from parent
    del_ss_pag(parent_PT, EMPTY_DATA_PAG);
  }
  // flush TLB
  set_cr3(get_DIR(parent_task));
  
  // set address of the kernel's stack space
  child_task->task.kernel_esp = ((KERNEL_ESP(child_task) - sizeof(union task_union))&0xfffff000) + ((DWord)get_ebp()&0x00000fff);
  child_task->task.kernel_esp -= 4;
  
  // set return address
  child_task->stack[(((child_task->task.kernel_esp)%sizeof(union task_union))/sizeof(DWord)) + 1] = (DWord)ret_from_fork;
  child_task->stack[(((child_task->task.kernel_esp)%sizeof(union task_union))/sizeof(DWord))] = KERNEL_ESP(child_task);
  
  // set new PID
  child_task->task.PID = get_new_PID();
  // set parent of new
  child_task->task.parent = parent_task;
  // initialize children list of new
  INIT_LIST_HEAD(&child_task->task.children);
  // add new to parent's list
  list_add_tail(&child_task->task.parent_list, &parent_task->children);
  dbg("[PID %d] Added children %d\n", &parent_task->PID, &child_task->task.PID);

  // add to ready queue
  list_add_tail(&child_task->task.list, &readyqueue);

  return child_task->task.PID;
}

void sys_exit() {
  struct task_struct* task = current();
  dbg("[PID %d] Exiting process\n", &task->PID);

  free_user_pages(task);
  task->PID = -1;
  
  if (task->parent != NULL) {
    // remove children from parent's list
    list_del(&task->parent_list);
    task->parent = NULL;
  }
  
  // add remaining children to idle process
  struct list_head *element, *n;
  list_for_each_safe(element, n, &task->children) {
    struct task_struct* children = list_entry(element, struct task_struct, parent_list);
    list_del(element); // remove from old list
    list_add_tail(element, &idle_task->children); // add to new list
    children->parent = idle_task; // update parent
    dbg("[PID %d] Parent killed, moved to Idle\n", &children->PID);
  }
  dbg("\n");
  
  update_process_state_rr(task, &freequeue);
  sched_next_rr();
}

void sys_block() {
  struct task_struct* task = current();
  if (task->pending_unblocks > 0) {
    task->pending_unblocks -= 1;
    return;
  }
  update_process_state_rr(task, &blocked);
  sched_next_rr();
}

int sys_unblock(int pid) {
  struct task_struct* parent = current();
  struct task_struct* child_task = NULL;
  
  struct list_head* pos;
  list_for_each(pos, &parent->children) {
    struct task_struct* tmp = children_head_to_task_struct(pos);
    if (tmp->PID == pid) {
      child_task = tmp;
      break;
    }
  }
  // No child found with PID = pid
  if (child_task == NULL) return -ECHILD;
  
  if (child_task->state != ST_BLOCKED) child_task->pending_unblocks += 1;
  else update_process_state_rr(child_task, &readyqueue);
  
  return 0;
}

int sys_write(int fd, char * buffer, int size) {
    
    // CHECKS //
    int check = check_fd(fd, ESCRIPTURA);
    if (check < 0) return check;
    
    if (buffer == NULL) return -EINVAL; //EINVAL

    if (size < 0) return -EINVAL; //EINVAL

    if (!access_ok(LECTURA, buffer, size)) return -EFAULT;//EFAULT
    
    //Com que no tenim malloc, utilitzem un buffer per anar
    //copiant chunks del missatge d'usuari

    // WRITE //
    int written_chars = 0;

    char buff[128];
    int left = size;
    int current = 0;
    while (left > 128) {
      copy_from_user(&buffer[current], buff, 128);
      written_chars += sys_write_console(buff, 128);
      current += 128;
      left -= 128;
    }

    copy_from_user(&buffer[current], buff, left);
    written_chars += sys_write_console(buff, left);
    
    //Tornem numero total de caracters escrits
    return written_chars;
}

// `SYSCALL(10)`
// 
// Returns the number of clock ticks elapsed since the OS has booted.
int sys_gettime() {
  return get_clock_ticks();
}


//#################//
//### PARCIAL 1 ###//
//#################//
int sys_read(char* b) {
  dbg("[sys_read] received buffer addr %p\n", b);
  // Check if buffer is in address space of process
  if (!access_ok(ESCRIPTURA, b, 1)) return -EFAULT;
  
  dbg("[sys_read] Blocked from sys_read\n");
  block_for_keyboard();
  dbg("[sys_read] Unblocked!\n");
  dbg("[sys_read] Char received from keyboard: '%c'\n", &char_read);
  *b = char_read; // set read character
  dbg("[sys_read] Returning...\n");
  return 0;
}