#include <libc.h>

int pid;

void test_spawn_maximum() {
  int children = 0;
  while (1) {
    int child = fork();
    if (child == 0) {
      block();
      exit(0);
    } else if (child > 0) {
      children += 1;
      printf("Spawned children number %d; %d in total\n", &child, &children);
    } else {
      print("Fork error: "); perror();
      exit(0);
    }
  }
}

/// - Fork Process 1 into Process 2.
/// - Scheduler executes Process 2.
/// - When `time == 150`, the Child process blocks itself.
/// - Parent executes until `time == 400`, 50 clock ticks more than it should (`quantum == 200`).
/// - Then the parent unblocks the Child, and the scheduler immediately changes to it.
/// - Scheduler executes Process 2 for the whole 200 clock ticks.
/// - When `time == 600`, scheduler changes to Parent.
/// - When `time == 605`, Parent exits. Idle is assigned as Child's parent and scheduler executes it immediately.
/// - When `time == 625`, Child exits, leaving no process in `readyqueue` and scheduler executes Idle forever.
void test_scheduling_multiple_processes() {
  char msg[] = "X: ";
  int child = fork();
  try_fork: switch (child) {
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
  
  int pid = getpid();

  int prev_time = 0;
  while(1) {
    int time = gettime();
    if (time == prev_time) {
      continue;
    }
    prev_time = time;

    int ppid = getppid();
    printf("%s%d; PID = %d; Parent PID = %d\n", msg, &time, &pid, &ppid);
    
    if (msg[0] == 'C') {
      if (time == 150) {
        println("Blocking Children\n");
        block();
      } else if (time >= 625) { 
        exit(0);
      }
    } else if (msg[0] == 'P') {
      if (child && time == 400) {
        int ret = unblock(child);
        printf("Unblocking Children(%d)\n", &ret);
        child = 0;
      } else if (time >= 605) { 
        exit(0);
      }
    }
  }
}

int __attribute__((__section__(".text.main"))) main(void) {
  // Next line, tries to move value 0 to CR3 register. This register is a
  // privileged one, and so it will raise an exception
  // __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) );

  println("\nHello ZeOS from user!");

  // test_spawn_maximum();
  test_scheduling_multiple_processes();
}
