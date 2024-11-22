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

/// # `DEFAULT_QUANTUM = 200`
/// - Fork Process 1 into Process 2.
/// - Scheduler continues running Process 1.
/// - When `time == 200` the Scheduler changes the execution to the Child.
/// - When `time == 215` the Child blocks itself, and the Scheduler changes to the Parent.
/// - Parent executes until `time == 415`, 15 ticks more than it should (`quantum == 200`).
/// - Then the parent unblocks the Child, and the Scheduler immediately changes to it.
/// - Scheduler executes Process 2 for the whole 200 clock ticks.
/// - When `time == 615`, Scheduler changes to Parent.
/// - When `time == 620`, Parent exits. Idle is assigned as Child's parent and the Scheduler executes it immediately.
/// - When `time == 625`, Child exits, leaving no process in `readyqueue` and the Scheduler executes Idle forever.
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
      if (time == 215) {
        println("Blocking Children\n");
        block();
      } else if (time >= 625) { 
        exit(0);
      }
    } else if (msg[0] == 'P') {
      if (child && time == 415) {
        int ret = unblock(child);
        printf("Unblocking Children(%d)\n\n", &ret);
        child = 0;
      } else if (time >= 620) { 
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
  
  // should never reach
  while (1);
}