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
int print(char* buffer) {
  return write(STDOUT, buffer, strlen(buffer));
}
// Prints the provided character
int printchar(char c) {
  return write(STDOUT, &c, 1);
}
// Prints the provided integer
int printint(int i) {
  char itoa_buff[11];
  itoa(i, itoa_buff);
  return print(itoa_buff);
}
// Prints the provided integer with a newline at the end
int printintln(int i) {
  int written, err;
  if ((written = printint(i)) < 0) return written;
  if ((err = printchar('\n')) < 0) return err;
  return written + 1;
}
// Prints the provided buffer with a newline at the end
int println(char* buffer) {
  int written, err;
  if ((written = print(buffer)) < 0) return written;
  if ((err = printchar('\n')) < 0) return err;
  return written + 1;
}

// Prints the provided integer in hexadecimal
void printhex(int i) {
  char buf[11];
  itox(i, buf);
  print(buf);
}

// Prints the provided integer in hexadecimal plus a newline
void printhexln(int i) {
  printhex(i);
  printchar('\n');
}

void printptr(const void* ptr) {
  if (ptr == NULL) {
    print("NULL");
    return;
  }
  printhex((int)(long)ptr);
}

void printptrln(void* ptr) {
  printptr(ptr);
  printchar('\n');
}

void __printf(char* template, const void* args[]) {
  int i = 0, arg = 0;
  char c;
  while ( (c = template[i]) ) {
    i += 1;
    if (c != '%') {
      printchar(c);
      continue;
    }
    switch (template[i]) {
      case 'd':
        printint(*(int*)args[arg]);
        break;
      case 'c':
        printchar(*(char*)args[arg]);
        break;
      case 'p':
        printptr(args[arg]);
        break;
      case 'x':
        printhex(*(int*)args[arg]);
        break;
      case 's':
        print((char*)args[arg]);
        break;
      case '%':
        printchar('%');
        break;
      default:
        printf("%%ERROR in arg %d%%", &arg);
        break;
    }
    arg += 1;
    i += 1;
  }
}