/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <types.h>

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
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

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}

int sys_write(int fd, char * buffer, int size) {
    int check = check_fd(fd, ESCRIPTURA);
    if (check < 0) return check;
    
    // TODO: Correct error code for nullptr
    if (buffer == NULL) return -22; //EINVAL

    if (size < 0) return -22; //EINVAL

    if (!access_ok(LECTURA, buffer, size)) return -14;//EFAULT
    
    //Com que no tenim malloc, utilitzem un buffer per anar
    //copiant chunks del missatge d'usuari

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