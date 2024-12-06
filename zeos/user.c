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
void test_semaphore();

int __attribute__ ((__section__(".text.main"))) main(void) {
  printchar('\n');

  // player: 2
  // enemy: 8

  // test_keyboard(0);
  // test_screen(0);
  // test_threads(2);
  // test_fork(4);
  test_semaphore();

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
void wait_with_callback(int ticks, void callback(int time)) {
  int prev = -1;
  while (ticks > 0) {
    int time = gettime();
    if (prev == time) continue;
    callback(time);
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

/// Test Thread Function, prints the address of its argument
void ttf(void* arg) {
  printf("[test_thread] Thread #%p spawned\n", arg);
  while (1); // Exit not needed, wrapper is used
}

/// Tests spawning `times` threads.
/// THIS FUNCTION NEVER RETURNS
void test_threads(int times) {
  for (int i = 1; i <= times; ++i) {
    int ret = threadCreateWithStack(ttf, 2, (void*)(long)i);
    if (ret < 0) {
      print("[test_thread] Could not spawn thread: "); perror();
      exit();
    }
    printf("[test_thread] Created thread #%d\n\n", &i);
  }
  
  // TODO: A wrapper for the main function is needed? How will it free itself?
  while (1);
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


void stt(struct sem_t* sem) {
  printf("[stt] Thread callback entered, doing something...\n");
  
  void (*callback)(int) = ({
    void __fn__ (int time) { printf("[stt] time: %d\n", &time); }
    __fn__;
  });

  wait_with_callback(20, callback);
  // wait(30);
  semSignal(sem);
  
  while (1);
}

void test_semaphore() {
  struct sem_t* sem = semCreate(0);
  printf("[test-semaphore] Sem created with address: %p\n", sem);
  
  threadCreateWithStack((void*)stt, 1, sem);
  printf("[test-semaphore] Thread spawned\n");
  
  semWait(sem);
  printf("[test-semaphore] Main thread unblocked!\n");

  semDestroy(sem);
  
  while (1);
}