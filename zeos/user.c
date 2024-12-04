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
void test_threads(int times);
void test_fork(int times);

int __attribute__ ((__section__(".text.main"))) main(void) {
  printchar('\n');

  // player: 2
  // enemy: 8

  // test_keyboard(0);
  // test_screen(0);
  test_threads(3);
  // test_fork(4);

  println("Finished tests!");
  
  while (1);
}

void wait(int ticks) {
  int prev = -1;
  while (ticks > 0) {
    int time = gettime();
    if (prev == time) continue;
    prev = time;
    ticks -= 1;
  }
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
      exit();
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

int accessible = 93;
void test_threads_function(void* argument) {
  int arg = (int)(long)argument;
  printf("[test_thread] Thread #%d spawned\n", &arg);
  if (accessible != 93) {
    printf("[test_thread] Thread #%d: expected global variable 93, found %d\n", &accessible);
    exit(1);
  }
  while (1);
}
void test_threads(int times) {
  int created_threads = 0;
  
  int ret = threadCreateWithStack(test_threads_function, 1, (void*)(long)created_threads);
  if (ret < 0) {
    print("[test_thread] Could not spawn thread: "); perror();
    exit(1);
  }
  printf("Created thread #%d\n\n", &created_threads);
  
  created_threads += 1;
  ret = threadCreateWithStack(test_threads_function, 1, (void*)(long)created_threads);
  if (ret < 0) {
    print("[test_thread] Could not spawn thread: "); perror();
    exit(1);
  }
  printf("Created thread #%d\n\n", &created_threads);

  created_threads += 1;
  ret = threadCreateWithStack(test_threads_function, 1, (void*)(long)created_threads);
  if (ret < 0) {
    print("[test_thread] Could not spawn thread: "); perror();
    exit(1);
  }
  printf("Created thread #%d\n\n", &created_threads);
}

void test_fork(int times) {
  if (times == 0) {
    println("[test_fork] All tests succeded!\n\n");
    return;
  }
  printf("[test_fork] Starting test #%d\n", &times);

  int forks = 0;
  int ret = 1;
  while (ret > 0) {
    ret = fork();
    if (ret > 0) forks += 1;
  }
  if (ret == 0) {
    // wait some time to allow for other processes to create
    wait(20);
    printf("Fork %d exiting\n", &forks);
    exit();
  }
  if (forks != 8) {
    printf("[test_fork] Expected 8 processes, found %d\n", &forks);
    exit();
  }
  if (errno != ENOMEM) {
    printf("[test_fork] Expected errno of ENOMEM, found %d\n", &errno);
    exit();
  }
  // wait some time to allow for other processes to exit
  wait(40);
  printf("[test_fork] Test #%d, successful!\n\n", &times);

  test_fork(times-1);
}