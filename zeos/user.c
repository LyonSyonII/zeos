#include "types.h"
#include <errno.h>
#include <libc.h>
#include <queue.h>

char buff[24];

int pid;


DEFINE_QUEUE(charq, char);
IMPL_QUEUE(charq, char);

void test_keyboard(int block);
void test_screen(int block);
void test_fork(int block);

int __attribute__ ((__section__(".text.main"))) main(void) {
  printchar('\n');
  

  // player: 2
  // enemy: 8

  test_keyboard(0);
  // test_screen(0);
  test_fork(0);

  while (1);
}

void test_keyboard(int block) {
  char chars[] = {'1', '2', '3', '4', '5', '6'};
  keyboard_buffer kbuf = KBUF_NEW(sizeof(chars));
  for (int i = 0; i < sizeof(chars); i++) {
    kbuf_push(&kbuf, chars[i]);
  }
  
  int i = 0;
  KBUF_ITER(kbuf, value) {
    if (value != chars[i]) {
      printf("[test_keyboard] ERROR: Expected '%c', found '%c'\n", &chars[i], &value);
      exit(1);
    }
    i += 1;
  }
  println("[test_keyboard] Test successful!\n");
  
  int prevtime = -1;
  while (block) {
    char tecla;
    if (getKey(&tecla, 1 << 30) >= 0) {
      printchar(tecla);
    }
  }
}

void test_screen(int block) {
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
  clrscr((char*)map);
  
  while (block) {
    char tecla;
    if (getKey(&tecla, 10) >= 0) {
      printchar(tecla);
      continue;
    }

    printchar('X'); // Test gotoXY
    gotoXY(x, y++);
    changeColour(color, color>>4); // Test colours
    ++color;
    if (y >= 25) {
      y = 0;
      ++x;
    }
  }
}

void test_fork(int block) {
  int forks = 0;
  int ret = 1;
  while (ret > 0) {
    ret = fork();
    forks += 1;
  }
  if (ret == 0) {
    while (block);
    printf("Fork %d exiting\n", &forks);
    exit(0);
  }
  if (forks != 9) {
    printf("[test_fork] Expected 9 processes, found %d\n", &forks);
    exit(1);
  }
  if (errno != ENOMEM) {
    printf("[test_fork] Expected errno of ENOMEM, found %d\n", &errno);
    exit(1);
  }
  printf("[test_fork] Test successful!\n");

/*   switch (fork()) {
    case -1: {
      print("Fork failed with error: ");
      perror();
      exit(1);
    }
    case 0: {
      break;
    }
    default: {
      break;
    }
  } */

  while (block);
}