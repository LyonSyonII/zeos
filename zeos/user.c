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

  
  int prevtime = -1;

  int x = 100, y = 0;
  while(1) {
    // int time = gettime();
    // if (time == prevtime) continue;
    // prevtime = time;

    char tecla;
    if (getKey(&tecla, 3) >= 0) {
      printchar(tecla);
    } else {
      printchar('.');
      gotoXY(x, y++);
      if (y >= 25) {
        y = 0;
        ++x;
      }
    }

    // int seconds = memRegDel("a");
    // printf("ticks: %d;\n", &time);
  }
}
