#include <libc.h>

char buff[24];

int pid;

// int addASM(int a, int b);

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  // char* p = 0;
  // *p = 'x';
  // int result = addASM(99, 85);
  //printlntest("test de com de llarg podem fer el missatge sense que exploti i tal perque bueno i si volem escriure un missatge molt llarg i tal que? i si ho he entes be hem d'utilitzar la funcio aquesta de copy_from_user i copy_to_user encara que funcioni sense aixo i he ficat un parche molt cutre perque no tenim malloc per fer-ho be");
  
  /// WRITE ///
  int written = 0;

  printlntest("Hello ZeOS from user!");
  printlntest("\nIf you read this message, I'm alive");

  //crida que falla (fd incorrecte)
  written = write(0, "alo", 3); 
  if (written < 0) perror();

  //Test per null pointer
  written = write(1, (char*)0, 3);
  if (written < 0) perror();

  /// GETTIME ///
  printintln(gettime());
  printintln(gettime());
  printintln(gettime());
  printintln(gettime());
  printintln(gettime());

  while(1) { }
}
