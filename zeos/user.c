#include <errno.h>
#include <libc.h>
#include <queue.h>

#define exit() exit(1)

char buff[24];

int pid;


void test_keyboard(int block);
void test_screen(int block);
void test_threads(char times, int terminate);
void test_fork(int times);
void test_semaphore();
int test_alloc();

int __attribute__ ((__section__(".text.main"))) main(void) {
  printchar('\n');
  // player: 2
  // enemy: 8
  
  // test_keyboard(0);
  // test_screen(0);
  test_fork(4);
  test_threads(4, 0); // terminate = 1 per testejar exit al thread principal
  test_semaphore();
  if (!test_alloc()) exit();

  println("Finished tests!\n\n");
  exit();
  
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
      printf("[test-keyboard] ERROR: Expected '%c', found '%c'\n", &chars[i], &value);
      exit();
    }
    i += 1;
  }
  println("[test-keyboard] Test successful!\n");
  
  int prevtime = -1;
  while (block) {
    char tecla;
    if (getKey(&tecla, 60) >= 0) {
      printchar(tecla);
    } else {
      printchar('.');
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

  wait(500);

  clrscr((char*)0);
  
  while (block) {
    char tecla;
    if (getKey(&tecla, 10) >= 0) {
      printchar(tecla);
      continue;
    }

    printchar('X'); // Test gotoXY
    gotoXY(x, y++);
    changeColor(color, color>>4); // Test colors
    ++color;
    if (y >= 25) {
      y = 0;
      ++x;
    }
  }
}

/// Test Thread Function, prints the address of its argument
void ttf(void* arg) {
  int thread_id = (long)arg>>4;
  int stack_size = (long)arg&0xF;
  int first_page = ((long)&arg >> 12) - stack_size + 1;
  int last_page = first_page + stack_size - 1;
  char* first_addr = (char*)(long)(first_page << 12);

  printf("\n[test-thread] Thread #%d spawned with stack size %d\n", &thread_id, &stack_size);
  // Check that it can access its memory region
  printf("[test-thread] Test thread #%d accessing pages from %d to %d\n", &thread_id, &first_page, &last_page);
  int t = 0;
  for (int i = 0; i < stack_size; i++) {
    t += *(1024 * i + first_addr);        // first address
    t += *(1024 * i + first_addr + 1023); // last address
  }
  printf("[test-thread] Accessed result is: %d\n", &t);
  printf("[test-thread] Thread #%d exiting...\n", &thread_id);
  
  // while (1) {} // Comment to test wrapper
  // Exit not needed, wrapper is used
}
/// Tests spawning `times` threads.
void test_threads(char threads, int terminate) {
  if (threads < 0) return;
  
  int stack_size = 5;

  for (int i = 1; i <= threads; ++i) {
    int ret = threadCreateWithStack(ttf, stack_size, (void*)(long)((i << 4) | stack_size));
    if (ret < 0) {
      print("[test-thread] Could not spawn thread: "); perror();
      exit();
    }
    printf("[test-thread] Created thread #%d\n", &i);
  }
  // Wait for threads to exit
  wait(50);
  printf("[test-thread] Test successful!\n\n\n");
  if (terminate) exit();
}

void test_fork(int times) {
  if (times == 0) {
    println("[test-fork] All tests succeded!\n\n");
    return;
  }
  printf("[test-fork] Starting test #%d\n", &times);

  int forks = 0;
  int ret = 1;
  while (ret > 0) {
    ret = fork();
    if (ret > 0) forks += 1;
  }
  if (ret == 0) {
    // wait some time to allow for other processes to create
    wait(20);
    printf("\n[test-fork] Fork %d exiting\n", &forks);
    exit();
  }
  if (forks != 8) {
    printf("[test-fork] Expected 8 processes, found %d\n", &forks);
    exit();
  }
  if (errno != ENOMEM) {
    printf("[test-fork] Expected errno of ENOMEM, found %d\n", &errno);
    exit();
  }
  // wait some time to allow for other processes to exit
  yield();
  wait(80);
  printf("[test-fork] Test #%d, successful!\n\n", &times);

  test_fork(times-1);
}


void stt(struct sem_t* sem) {
  printf("[stt] Thread callback entered, doing something...\n");
  
  // miraculosament aixo crea una funcio anonima
  void (*callback)(int) = ({
    void __fn__ (int time) { printf("[stt] time: %d\n", &time); }
    __fn__;
  });

  wait_with_callback(20, callback);
  
  semSignal(sem);

  printf("[test-semaphore] Thread exited\n");
  
  // while (1);
}

void test_semaphore() {
  struct sem_t* sem = semCreate(0);
  printf("[test-semaphore] Sem created with address: %p\n", sem);
  
  threadCreateWithStack((void*)stt, 1, sem);
  printf("[test-semaphore] Thread spawned\n");
  
  semWait(sem);
  printf("[test-semaphore] Main thread unblocked!\n\n");

  semDestroy(sem);

  sem = semCreate(1);
  int pid = fork();
  // make parent wait
  if (pid != 0) yield();
  semWait(sem);
  
  printf("[test-semaphore] I'm continuing, fork result: %d\n", &pid);
  wait(50);
  semSignal(sem);
  
  if (pid == 0) {
    printf("[test-semaphore] Child: wait ended\n", &pid);
    exit();
  }
  printf("[test-semaphore] Parent: wait ended\n", &pid);
  semDestroy(sem);
  printf("[test-semaphore] Test successful!\n\n\n");
}

void tat(void* arg) {
  char* alloc = arg;
  int thread = alloc[0];
  printf("[test-alloc] Spawned thread #%d\n", &thread);
  printf("[test-alloc] Testing accessing parent's allocation: '%s'\n", &alloc[1]);
  printf("[test-alloc] Testing creating allocation\n");
  char* alloc2 = memRegGet(1);
  alloc2[0] = 'J';
  alloc2[1] = 'a';
  alloc2[2] = 'j';
  alloc2[3] = 'a';
  alloc2[4] = 0;
  printf("[test-alloc] Testing accessing allocation: '%s'\n", alloc2);

  printf("[test-alloc] Testing freeing memory from fork of thread\n");
  int child = fork();
  if (child == 0) {
    printf("[test-alloc] Freeing memory from thread's fork\n");
  } else {
    yield();
    printf("[test-alloc] Freeing thread's memory\n");
  }
  memRegDel(alloc2);
  printf("[test-alloc] Freed thread's memory\n[test-alloc] Freeing parent's\n");
  memRegDel(alloc);
  if (child == 0) {
    printf("[test-alloc] Memory successfully freed from thread's fork\n");
    exit();
  }
  printf("[test-alloc] Thread #%d test complete!\n", &thread);
}
int test_alloc() {
  int n = 3;
  printf("[test-alloc] Allocating %d pages...\n", &n);
  char* alloc = memRegGet(n);
  if (alloc == NULL) {
    printf("[test-alloc] Error allocating %d pages: ", &n); perror();
    return 0;
  }
  alloc[0] = 'H';
  alloc[1] = 'o';
  alloc[2] = 'l';
  alloc[3] = 'a';
  alloc[4] = 0;

  // uncomment to test disallowing user to access metadata page (should page fault)
  // int* _metadata = ((((long)alloc >> 12) << 12) - 4096); 
  // if (*_metadata == 0xDEADBED) {
  //   printf("WRONG\n");
  //   while(1);
  // }

  printf("[test-alloc] Printing allocated string: '%s'\n", alloc);
  
  printf("[test-alloc] Deallocating pages...\n", &n);
  if (memRegDel(alloc) < 0) {
    printf("[test-alloc] Error deallocating %d pages: ", &n); perror();
    printchar('\n');
    return 0;
  }

  printf("[test-alloc] Testing allocation persistent on threads\n");
  char* alloc2 = memRegGet(n);
  alloc2[0] = 1;
  alloc2[1] = 'D';
  alloc2[2] = 'o';
  alloc2[3] = 0;
  threadCreateWithStack(tat, 2, alloc2);
  yield();
  
  printf("[test-alloc] Testing allocation persistent on fork\n");
  char* alloc4 = memRegGet(n);
  alloc4[3] = 'F';
  alloc4[4] = 'a';
  char* alloc5 = memRegGet(n);
  alloc5[93] = 'R';
  alloc5[94] = 'e';
  
  printf("[test-alloc] Starting fork\n");
  
  switch (fork()) {
    case -1: {
      printf("[test-alloc] Error on fork: "); perror();
      return 0;
    };
    case 0: {
      printf("[test-alloc] Checking if children has access to a copy of the allocated pages\n");
      if (alloc4[3] != 'F') {
        printf("[test-alloc] Expected alloc[3] = 'F', found %c\n", &alloc4[3]);
        return 0;
      }
      if (alloc4[4] != 'a') {
        printf("[test-alloc] Expected alloc[4] = 'a', found %c\n", &alloc4[4]);
        return 0;
      }
      alloc4[5] = '\0';
      printf("[test-alloc] Printing child's string: %s\n", &alloc4[3]);
      printf("[test-alloc] Deallocating child pages\n");
      memRegDel(alloc4);      // comment to test `exit()` deallocating pages
      exit();
    };
    default: {
      yield();               // uncomment to test exiting before child
      printf("[test-alloc] Deallocating parent pages\n");
      memRegDel(alloc4);     // uncomment to test `exit()` not deallocating pages already freed           
      printf("[test-alloc] Test successful!\n\n\n");
    }
  }
  return 1;
}