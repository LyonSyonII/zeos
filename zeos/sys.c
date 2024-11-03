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
  
  struct list_head *lh = list_first(&freequeue);
  list_del(lh); //Delete from freequeue

  union task_union *child_tu = list_entry(lh, union task_union, task.list);

  struct task_struct *parent_ts = current();
  
  //b)
  copy_data((void*)parent_ts, (void*)child_tu, sizeof(union task_union));

  //c)
  allocate_DIR(&child_tu->task);

  //d)
  int frames[NUM_PAG_DATA];

  for (int i = 0; i < NUM_PAG_DATA; ++i) {
    int ret = alloc_frame();
    if (ret < 0) {
      for (i -= 1; i >= 0; --i) free_frame(frames[i]);
      return undo_fork(2, child_tu, -1); //ERROR: NO_FREE_MEMORY
    } else {
      frames[i] = ret;
    }
  }

  page_table_entry *cpte = get_PT(&child_tu->task);
  page_table_entry *ppte = get_PT(parent_ts);  
  
  //e)

  for (int pag = 0; pag < NUM_PAG_CODE; ++pag) { //"copia" codi
    set_ss_pag(cpte, PAG_LOG_INIT_CODE + pag, ppte[PAG_LOG_INIT_CODE + pag].bits.pbase_addr);
  }


  for (int pag = 0; pag < NUM_PAG_DATA; ++pag) { //inicialitzades pagines de data
    set_ss_pag(cpte,PAG_LOG_INIT_DATA + pag, frames[pag]);
  }



  //f)


  //A) 

  
  //Trobem pagines lliures consecutives suficients com per copiar la data (es el que faria si funciones)
  int first_free_page = USER_FIRST_PAGE + NUM_PAG_DATA + NUM_PAG_CODE;
  int nfree_pages = 0;
  while (nfree_pages < NUM_PAG_DATA && first_free_page < TOTAL_PAGES) {
    int trobat = 0;
    while (ppte[first_free_page + nfree_pages++].entry == 0 && !trobat) {
      if (nfree_pages > NUM_PAG_DATA) trobat = 1;
    }
    if (!trobat) {
      first_free_page += nfree_pages;
      nfree_pages = 0;
    }
  }


  if (TOTAL_PAGES < first_free_page) return -1; //No hi ha suficients pagines lliures consecutives com per copiar la data

  //Inicialitzem entrades temporals a la taula del pare
  for (int pag = 0; pag < NUM_PAG_DATA; ++pag) {
    set_ss_pag(ppte, first_free_page + pag, frames[pag]);
    
    //B) Copiem les pagines de data del pare
    copy_data((void*)((PAG_LOG_INIT_DATA + pag)<<12), (void*)((first_free_page + pag)<<12), PAGE_SIZE);
    
    //C) Eliminem entrades temporals de la taula del pare
    del_ss_pag(ppte, first_free_page + pag);
  }

  //flush TLB
  set_cr3(parent_ts->dir_pages_baseAddr);


  //g) Assign PID (per exemple o fem aixi) REVISAR MOLT FORT
  PID = nextPID();

  child_tu->task.PID = PID;


  //h) ?? (not sure de si hem de fer aixo o alguna altre cosa)
  child_tu->task.kernel_esp = ((KERNEL_ESP(child_tu) - sizeof(union task_union))&0xfffff000) + (get_ebp()&0x00000fff);

  //i) preparem l'stack per cridar ret_from_fork al executar task_switch

  child_tu->task.kernel_esp -= 4;

  child_tu->stack[(((child_tu->task.kernel_esp)%sizeof(union task_union))/sizeof(DWord)) + 1] = (DWord)ret_from_fork;
  child_tu->stack[(((child_tu->task.kernel_esp)%sizeof(union task_union))/sizeof(DWord))] = KERNEL_ESP(child_tu);

  //Afegim el proces fill a la readyqueue
  list_add_tail(&child_tu->task.list, &readyqueue);

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

    //if (!access_ok(LECTURA, buffer, size)) return -EFAULT;//EFAULT
    
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