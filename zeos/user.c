#include <libc.h>

int pid;

// Prints the provided buffer and the number of bytes printed.
void printlntest(const char *buffer) {
  int written = print(buffer);
  if (written < 0)
    return;

  print(" (");
  printint(written);
  println(" bytes)");
}

int __attribute__((__section__(".text.main"))) main(void) {
  // Next line, tries to move value 0 to CR3 register. This register is a
  // privileged one, and so it will raise an exception
  // __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) );

  /// WRITE ///
  int written = 0;

  printlntest("\nHello ZeOS from user!");

  // Uncomment to test PAGE FAULT
  // char* p = 0; *p = 'x';

  // Test per getpid (en teoria funciona)
  print("PID: ");
  printintln(getpid());

  // Crida que falla (fd incorrecte)
  // written = write(0, "alo", 3);
  // if (written < 0) perror();

  // Test per null pointer
  // written = write(STDOUT, (char*)0, 3);
  // if (written < 0) perror();

  // Test per mida negativa
  // written = write(STDOUT, "alo2", -1);
  // if (written < 0) perror();

  switch (fork()) {
    case 0: {
      println("Child");
      break;
    }
    case -1: {
      print("Fork Error: ");
      perror();
      break;
    }
    default: {
      println("Parent");
      break;
    }
  }

  /// GETTIME ///
  while(1) {
    // Descomenta per imprimir el temps
    // printintln(gettime());
    // printintln(getpid());
    // println("loop");
  }
}
