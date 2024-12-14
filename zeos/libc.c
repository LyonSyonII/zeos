/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

int errno;
int REGS[7]; // Space to save REGISTERS

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

// integer to hexadecimal
void itox(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%16)+'0';
    if (b[i] > '9') {
      b[i] = b[i] - ('9'+1) + 'A';
    }
    a=a/16;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

void perror()
{
  char buffer[256];

  itoa(errno, buffer);

  write(1, buffer, strlen(buffer));
}


// custom methods


// Prints the provided buffer
int print(char* buffer, int fd) {
  return write(fd, buffer, strlen(buffer));
}
// Prints the provided character
int printchar(char c, int fd) {
  return write(fd, &c, 1);
}
// Prints the provided integer
int printint(int i, int fd) {
  char itoa_buff[11];
  itoa(i, itoa_buff);
  return print(itoa_buff, fd);
}
// Prints the provided integer with a newline at the end
int printintln(int i, int fd) {
  int written, err;
  if ((written = printint(i, fd)) < 0) return written;
  if ((err = printchar('\n', fd)) < 0) return err;
  return written + 1;
}
// Prints the provided buffer with a newline at the end
int println(char* buffer, int fd) {
  int written, err;
  if ((written = print(buffer, fd)) < 0) return written;
  if ((err = printchar('\n', fd)) < 0) return err;
  return written + 1;
}

// Prints the provided integer in hexadecimal
void printhex(int i, int fd) {
  char buf[11];
  itox(i, buf);
  print(buf, fd);
}

// Prints the provided integer in hexadecimal plus a newline
void printhexln(int i, int fd) {
  printhex(i, fd);
  printchar('\n', fd);
}

void printptr(const void* ptr, int fd) {
  if (ptr == NULL) {
    print("NULL", fd);
    return;
  }
  print("0x", fd);
  printhex((int)(long)ptr, fd);
}

void printptrln(void* ptr, int fd) {
  printptr(ptr, fd);
  printchar('\n', fd);
}

void __printf(char* template, const void* args[], int fd) {
  int i = 0, arg = 0;
  char c;
  while ( (c = template[i]) ) {
    i += 1;
    if (c != '%') {
      printchar(c, fd);
      continue;
    }
    switch (template[i]) {
      case 'd':
        printint(*(int*)args[arg], fd);
        break;
      case 'c':
        printchar(*(char*)args[arg], fd);
        break;
      case 'p':
        printptr(args[arg], fd);
        break;
      case 'x':
        printhex(*(int*)args[arg], fd);
        break;
      case 's':
        print((char*)args[arg], fd);
        break;
      case '%':
        printchar('%', fd);
        break;
      default:
        __printf("%%ERROR in arg %d%%", (const void*[]){&arg}, fd);
        break;
    }
    arg += 1;
    i += 1;
  }
}