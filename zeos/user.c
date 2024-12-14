#include "libc.h"
#include "queue.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define TASK(task) (void(*)(void*))(task)

typedef struct {
  keyboard_buffer buf;
} keyboard_params;

void task_screen();
void task_keyboard(keyboard_params* keyboard);


int __attribute__ ((__section__(".text.main"))) main(void) {
  keyboard_params keyboard = {
    .buf = KBUF_NEW(50),
  };
  char input = 0;
  int x = 40;
  int y = 12;

  clrscr(NULL);
  threadCreateWithStack(TASK(task_screen), 1, NULL);
  threadCreateWithStack(TASK(task_keyboard), 1, &keyboard);
  
  gotoXY(x, y);
  printchar(2, FD_SCREEN);

  while (1) {
    if (!kbuf_pop(&keyboard.buf, &input)) continue;
    
    gotoXY(x, y);
    printchar(' ', FD_SCREEN);
    switch (input) {
      case 'w': {
        y = max(y-1, 0);
        break;
      }
      case 'r': {
        y = min(y + 1, 24);
        break;
      }
      case 'a': {
        x -= 1;
        if (x < 0) x = 79;
        break;
      }
      case 's': {
        x += 1;
        if (x > 79) x = 0;
        break;
      }
    }
    
    gotoXY(x, y);
    printchar(2, FD_SCREEN);
  }
}



void task_screen() {
  println("[screen] Spawned Screen Thread", FD_BOCHS);
  while (1) {

  }
}

void task_keyboard(keyboard_params* params) {
  println("[keybrd] Spawned Keyboard", FD_BOCHS);
  char c;
  while (1) {
    int ret = getKey(&c, 1 << 30);
    if (ret >= 0) {
      kbuf_push(&params->buf, c);
      // printf("[keybrd] Received character '%c'\n", &c);
    } else {
      // printf("[keybrd] Error receiving character %d\n", &ret);
    }
  }
}