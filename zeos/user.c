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
  
  char msg[] = "X: ";
  try_fork: switch (fork()) {
    case -1: {
      print("Fork Error: ");
      perror();
      goto try_fork;
    }
    case 0: {
      msg[0] = 'C';
      break;
    }
    default: {
      msg[0] = 'P';
      break;
    }
  }

  /// GETTIME ///
  int prev_time = 0;
  while(1) {
    int time = gettime();
    if (time == prev_time) {
      continue;
    }
    prev_time = time;
    print(msg);
    printintln(time);
    
    if (msg[0] == 'P' && time > 500) {
      exit(1);
    } else if (time > 1000) {
      exit(1);
    }
    // Descomenta per imprimir el temps
    // printintln(gettime());
    // printintln(getpid());
    // println("loop");
  }
}
