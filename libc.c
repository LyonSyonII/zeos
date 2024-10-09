/*
 * libc.c 
 */

#include <libc.h>
#include <errno.h>
#include <types.h>

int errno;

//Array amb una llista dels missatges que imprimir amb perror (hem sembla bastant cutre pero funciona i guess)
//Nomes estan els errors que poden sortir del write()
char *errno_message[128] = {
  "", "", "", "", "", "", "", "",
  "", "Bad file number\n", "", "", "", "Permission denied\n", "Bad address\n", "",
  "", "", "", "", "", "", "Invalid argument\n", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "Function not implemented\n", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "",
};


void set_errno(int value) {
  errno = -value;
}

void perror() {
  char *s = errno_message[errno];
  write(1, s, strlen(s));
}


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

int strlen(const char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}


/// Custom methods

// Prints the provided buffer
int print(const char* buffer) {
  return write(1, buffer, strlen(buffer));
}
// Prints the provided character
int printchar(char c) {
  return write(1, &c, 1);
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
int println(const char* buffer) {
  int written, err;
  if ((written = print(buffer)) < 0) return written;
  if ((err = printchar('\n')) < 0) return err;
  return written + 1;
}
// Prints the provided buffer and the number of bytes printed
void printlntest(const char* buffer) {
  int written = print(buffer);
  if (written < 0) return;
  
  print(" (");
  printint(written);
  println(" bytes)");
}