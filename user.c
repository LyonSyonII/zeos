#include <libc.h>

char buff[24];

int pid;

// int addASM(int a, int b);

int print(const char* buffer) {
  return write(1, buffer, strlen(buffer));
}
int printc(char c) {
  return write(1, &c, 1);
}
int println(const char* buffer) {
  int written, err;
  if ((written = print(buffer)) < 0) return written;
  if ((err = printc('\n')) < 0) return err;
  return written + 1;
}

void printlntest(const char* buffer) {
  char itoa_buff[10];

  int written = print(buffer);
  if (written < 0) return;

  itoa(written, itoa_buff);
  print(" (");
  print(itoa_buff);
  println(" bytes)");
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  // char* p = 0;
  // *p = 'x';
  // int result = addASM(99, 85);
  //printlntest("test de com de llarg podem fer el missatge sense que exploti i tal perque bueno i si volem escriure un missatge molt llarg i tal que? i si ho he entes be hem d'utilitzar la funcio aquesta de copy_from_user i copy_to_user encara que funcioni sense aixo i he ficat un parche molt cutre perque no tenim malloc per fer-ho be");

  printlntest("Hello ZeOS from user!");
  printlntest("\nIf you read this message, I'm alive");

  //crida que falla (fd incorrecte)
  write(0, "alo", 3);
  perror();

  while(1) { }
}
