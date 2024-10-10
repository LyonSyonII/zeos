#include <libc.h>

int pid;

// Prints the provided buffer and the number of bytes printed.
void printlntest(const char* buffer) {
  int written = print(buffer);
  if (written < 0) return;
  
  print(" (");
  printint(written);
  println(" bytes)");
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  // Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception
  // __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) );


  
  /// WRITE ///
  int written = 0;

  printlntest("\nHello ZeOS from user!");
  printlntest("\nIf you read this message, I'm alive");
  // printlntest("text molt llarg per veure que copy_from_user funciona: Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus interdum, nunc vitae convallis dignissim, eros est faucibus tellus, non bibendum nisl lacus quis eros. Mauris eu felis vehicula, ornare ante quis, tincidunt tortor. Nam velit sem, convallis id posuere quis, finibus non purus. Vivamus eget mollis lectus. Proin ullamcorper consectetur risus, vitae mollis augue efficitur at. Praesent molestie nec turpis vitae lobortis. Pellentesque vehicula lacus quis ipsum blandit pulvinar. Aenean est risus, faucibus in feugiat sed, laoreet a eros. Integer congue metus in vestibulum eleifend. Ut eget egestas nunc, quis vestibulum urna. Suspendisse in risus vel nibh rhoncus aliquet. Praesent hendrerit est sit amet arcu porttitor posuere. Nullam libero turpis, semper at lectus nec, volutpat finibus magna. Duis elementum mauris ex, ac aliquet urna interdum convallis. Nullam massa nulla, pretium efficitur congue vitae, pulvinar quis dui. Fusce ultrices laoreet augue ut ultrices. Cras eu diam placerat, finibus lectus ac, ultrices purus. Cras elementum leo vitae venenatis accumsan. Curabitur accumsan pretium sagittis. Vivamus consectetur lobortis nulla, ut lacinia nunc congue at. Aliquam diam felis, congue ut condimentum eu, venenatis sed enim. Fusce sit amet erat ac neque porta hendrerit pulvinar at nulla. Maecenas gravida tincidunt magna ut auctor. Proin ac efficitur.");
  
  // Uncomment to test PAGE FAULT
  // char* p = 0; *p = 'x';

  // Crida que falla (fd incorrecte)
  written = write(0, "alo", 3); 
  if (written < 0) perror();

  // Test per null pointer
  written = write(1, (char*)0, 3);
  if (written < 0) perror();

  // Test per mida negativa
  written = write(1, "alo2", -1);
  if (written < 0) perror();

  /// GETTIME ///
  while(1) {
      // Descomenta per imprimir el temps
      // printintln(gettime());
  }
}
