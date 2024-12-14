#include "libc.h"
#include "queue.h"

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define TASK(task) (void(*)(void*))(task)

// Player
const char P = 2;
// Blank
const char B = ' ';

typedef struct {
  keyboard_buffer buf;
} keyboard_params;
typedef struct {
  struct sem_t* sem;
  Word* buf;
} screen_params;

void task_screen(screen_params* params);
void task_keyboard(keyboard_params* params);

int __attribute__ ((__section__(".text.main"))) main(void) {
  // Word* alloc = memRegGet(1);

  keyboard_params keyboard = {
    .buf = KBUF_NEW(10),
  };
  screen_params screen = {
    .buf = (Word*)memRegGet(1),
    .sem = semCreate(0)
  };
  char input = 0;
  int x = 40;
  int y = 12;

  clrscr(NULL);
  threadCreateWithStack(TASK(task_screen), 1, &screen);
  threadCreateWithStack(TASK(task_keyboard), 1, &keyboard);
  
  write_xy(P, x, y, FG_BLUE, BG_BLACK, screen.buf);
  semSignal(screen.sem);

  while (1) {
    if (!kbuf_pop(&keyboard.buf, &input)) continue;
    
    write_xy(B, x, y, FG_BLACK, BG_BLACK, screen.buf);
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
    write_xy(P, x, y, FG_BLUE, BG_BLACK, screen.buf);
    semSignal(screen.sem);
  }
}



void task_screen(screen_params* params) {
  println("[screen] Spawned Screen Thread", FD_BOCHS);
  while (1) {
    semWait(params->sem);
    // println("[screen] Updating screen", FD_BOCHS);
    clrscr((char*)params->buf);
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
      printf("[keybrd] Error receiving character %d\n", &ret);
    }
  }
}