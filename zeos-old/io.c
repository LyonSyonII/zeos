/*
 * io.c - 
 */

#include "sched.h"
#include "utils.h"
#include <io.h>

#include <types.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

Byte x, y=19;

/* Read a byte from 'port' */
Byte inb (unsigned short port)
{
  Byte v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
  return v;
}

void printc(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
    x = 0;
    y=(y+1)%NUM_ROWS;
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | 0x0200;
	Word *screen = (Word *)0xb8000;
	screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
      x = 0;
      y=(y+1)%NUM_ROWS;
    }
  }
}

void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=x;
  cy=y;
  x=mx;
  y=my;
  printc(c);
  x=cx;
  y=cy;
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}

// custom defined

void __itoa(int a, char *b)
{
  int i = 0, i1 = 0;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return; }
  
  if (a < 0) {
    b[0] = '-';
    a *= -1;
    i = 1;
    i1 = 1;
  }

  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

void printkln(char* string) {
  printk(string);
  printc('\n');
}

// Prints the provided integer
void printkint(int i) {
  char itoa_buff[11];
  __itoa(i, itoa_buff);
  printk(itoa_buff);
}

void printkintln(int i) {
  printkint(i);
  printc('\n');
}

// Prints the provided integer in hexadecimal
void printkhex(int i) {
  char buf[11];
  itox(i, buf);
  printk(buf);
}

// Prints the provided integer in hexadecimal plus a newline
void printkhexln(int i) {
  printkhex(i);
  printc('\n');
}

void printkptr(const void* ptr) {
  if (ptr == NULL) {
    printk("NULL");
    return;
  }
  printkhex((int)ptr);
}

void printkptrln(const void* ptr) {
  printkptr(ptr);
  printc('\n');
}


void __attribute__ ((noinline)) __dummy(const char* template, const void* args[]) {
  return;
}

void __printkf(const char* template, const void* args[]) {
  int i = 0, arg = 0;
  char c;
  while ( (c = template[i]) ) {
    i += 1;
    if (c != '%') {
      printc(c);
      continue;
    }
    switch (template[i]) {
      case 'd':
        printkint(*(int*)args[arg]);
        break;
      case 'p':
        printkptr(args[arg]);
        break;
      case 'x':
        printkhex(*(int*)args[arg]);
        break;
      case 's':
        printk((char*)args[arg]);
        break;
      default:
        printk("%ERROR in arg %"); printkint(arg);
        break;
    }
    arg += 1;
    i += 1;
  }
}

void dbg_task(struct task_struct* task) {
  const int err = -1;

  printkf("task {\n\
  PID: %d\n\
  addr: %p\n\
  dir_pages_baseAddr: %p\n\
  kernel_esp: %x\n\
  list: %p\n\
\n\
  children: %p\n\
  parent_list: %p\n\
  parent: %p\n\
  parent_PID: %d\n\
}\n",
    &task->PID,
    task,
    task->dir_pages_baseAddr,
    &task->kernel_esp,
    &task->list,
    
    &task->children,
    task->parent_list.next,
    task->parent,
    task->parent ? &task->parent->PID : &err
  );
}