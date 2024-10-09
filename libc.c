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

