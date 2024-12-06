#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sem.h"
#define NSHELLS 3
int my_procnum;
int sig_counter;

void sigusr1_handler(int signo) { sig_counter++; }

void game(struct sem* start, struct sem* end, int num_moves) {
  fprintf(stderr, "VCPU %d process started: pid %d \n", my_procnum, getpid());
  for (int i = 0; i < num_moves; i++) {
    sem_wait(start, my_procnum);
    sem_inc(end);
  }
  fprintf(stderr, "Child %d (pid %d) done: signal handler invoked %d times \n",
          my_procnum, getpid(), sig_counter);
}

// Built this to be as general as possible
int main(int argc, char** argv) {
  if (argc < 3) {
    printf("NOT ENOUGH ARGUMENTS\n");
    return -1;
  }
  signal(SIGUSR1, sigusr1_handler);
  // Parent gets -1
  my_procnum = -1;
  int sig_counter = 0;
  // Not doing any error handling here
  int num_pebbles = atoi(argv[1]);
  int num_moves = atoi(argv[2]);
  // 3 distinct shells
  struct sem* shell_A =
      mmap(0, sizeof(struct sem) * NSHELLS, PROT_READ | PROT_WRITE,
           MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  struct sem* shell_B = shell_A + 1;
  struct sem* shell_C = shell_A + 2;
  // struct sem* shells[] = {shell_A, shell_B, shell_C};
  for (int i = 0; i < NSHELLS; i++) {
    sem_init(shell_A + i, num_pebbles);
  }

  for (int i = 0; i < NSHELLS; i++) {
    for (int j = i + 1; j < NSHELLS; j++) {
      int proc = fork();
      switch (proc) {
        case 0:
          my_procnum = 2 * (i + j) - 2;
          game(shell_A + i, shell_A + j, num_moves);
          return 0;
          break;
        default:
          break;
      };
      // Fork a second process, which makes the opposite moves here
      proc = fork();
      switch (proc) {
        case 0:
          my_procnum = 2 * (i + j) - 1;
          game(shell_A + j, shell_A + i, num_moves);
          return 0;
          break;
        default:
          break;
      };
    }
  }
  fprintf(stderr, "Main has spawned all processes: Waiting \n");
  // Should be NSHELLS factorial
  for (int i = 0; i < 6; i++) {
    int childpid = wait(NULL);
  }
  fprintf(stderr, "All children complete! Tabling stats \n");
  fprintf(stderr, "Sem # \t val \t Sleeps \t Wakes \n");
  for (int i = 0; i < NSHELLS; i++) {
    struct sem shell = *(shell_A + i);
    fprintf(stderr, "%d \t %d \n", i, shell.count);
    for (int i = 0; i < 6; i++) {
      fprintf(stderr, " VCPU %d \t \t %d \t %d \n", i,
              shell.instrumentation[i].sleep, shell.instrumentation[i].woken);
    }
  }
}