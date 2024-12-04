#include "types.h"
#include <libc.h>
#include <queue.h>

char buff[24];

int pid;


DEFINE_QUEUE(charq, char);
IMPL_QUEUE(charq, char);

void print_queue(keyboard_buffer kbuf) {
  KBUF_ITER(kbuf, value) {
    printf("'%c' ", &value);
  }
  printchar('\n');
}

int __attribute__ ((__section__(".text.main"))) main(void) {
  printchar('\n');

  keyboard_buffer kbuf = KBUF_NEW(4);

  // kbuf_push(&kbuf, '1');
  // print_queue(kbuf);
  // kbuf_push(&kbuf, '2');
  // print_queue(kbuf);
  // kbuf_push(&kbuf, '3');
  // print_queue(kbuf);
  // kbuf_push(&kbuf, '4');
  // print_queue(kbuf);
  // kbuf_push(&kbuf, '5');
  // print_queue(kbuf);
  // kbuf_push(&kbuf, '6');
  // 
  // print_queue(kbuf);
  

  // player: 2
  // enemy: 8
  char map[25][80*sizeof(Word)];
  for (int i = 0; i < 25; ++i) {
    Byte col = 0;
    for (int j = 0; j < 160; j += 2) {
      map[i][j] = j/2;
      map[i][j + 1] = col++;
    }
  }

  int prevtime = -1;

  int x = 100, y = 0;
  Byte color = 0;
  clrscr(map);
  
  while(1) {
    // int time = gettime();
    // if (time == prevtime) continue;
    // prevtime = time;

    char tecla;
    if (getKey(&tecla, 10) >= 0) {
      printchar(tecla);
    } else {
      printchar('X'); // Test gotoXY
      gotoXY(x, y++);
      changeColour(color, color>>4); // Test colours
      ++color;
      if (y >= 25) {
        y = 0;
        ++x;
      }
    }

    // int seconds = memRegDel("a");
    // printf("ticks: %d;\n", &time);
  }
  while (1); // definitivament no m'he estat ratllant perque sortia un page fault a l'acabar el main
}
