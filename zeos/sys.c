/*
 * sys.c - Syscalls implementation
 */
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

int sys_getpid()
{
	return current()->PID;
}

int undo_fork(int step, union task_union *tu, int error) {
  switch (step) {
    case 0:

    case 1:

    case 2:

      list_add(&tu->task.list, &freequeue);
  }
  return error;
}


int sys_fork()
{
  int PID=-1;

  // creates the child process
  if (list_empty(&freequeue)) return -EPERM; //CANVIAR
  union task_union *child_tu = list_entry(list_first(&freequeue), union task_union, task.list);

  union task_union *parent_tu = current();

  copy_data((void*)parent_tu, (void*)child_tu, 4096);
  
  allocate_DIR(&child_tu->task);

  if (set_user_pages(&child_tu->task) < 0) return ENOMEM;

  page_table_entry *cpte = get_PT(&child_tu->task);
  
  page_table_entry *ppte = get_PT(&parent_tu->task);

  /*
  for (int pag = 0; pag < NUM_PAG_CODE; ++pag) {
    set_ss_pag(ppte, pag+PAG_LOG_INIT_CODE, cpte->bits.pbase_addr + PAG_LOG_INIT_CODE + );
  }*/


  for (int pag = 0; pag < NUM_PAG_CODE; ++pag) {
    set_ss_pag(cpte, pag + PAG_LOG_INIT_CODE, ppte->bits.pbase_addr + sizeof(page_table_entry)*(pag+PAG_LOG_INIT_CODE));
  }


  int temp_frames[NUM_PAG_DATA];

  for (int pag = 0; pag < NUM_PAG_DATA; ++pag) {
    if ((temp_frames[pag] = alloc_frame()) < 0) {
      
    }
    set_ss_pag(ppte, temp_frames[pag], cpte->bits.pbase_addr + sizeof(page_table_entry)*(pag+PAG_LOG_INIT_DATA));
  }



  return PID;
}

void sys_exit()
{  
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