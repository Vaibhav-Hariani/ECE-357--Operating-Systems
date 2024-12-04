#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "spinlock.h"

#define NUM_PROC 64
#define NUM_ITERS 1e3
int my_procnum;

int test_spinlock(char* lock, double* c) {
  // c is pointer to mmap region
  // printf("In child \n");
  for (int i = 0; i < NUM_ITERS; i++) {
    spin_lock(lock);
    double i = *c;
    i = i + 1;
    *c = i;
    printf("%f\n", *c);
    spin_unlock(lock);
  }
};

int no_lock(double* c) {
  // c is pointer to mmap region
  for (int i = 0; i < NUM_ITERS; i++) {
    // Broke this up to improve the odds of an race condition failure
    double i = *c;
    i = i + 1;
    *c = i;
    printf("%f\n", *c);
  }
};

int main() {
  // Parent process
  my_procnum = 0;
  char* spinlock_open = mmap(0, sizeof(char), PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_SHARED, -1, 0);;
  double* c = mmap(0, 5 * sizeof(double), PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  int fd = 0;
  int children[NUM_PROC];
  for (int i = 1; i < NUM_PROC; i++) {
    int proc = fork();
    switch (proc) {
      case 0:
        my_procnum = i;
        test_spinlock(spinlock_open, c);
        // no_lock(c);
        return 0;
        break;
      default:
        children[i] = proc;
        break;
    };
  }  
  int* status;
  //Wait for all children
  for (int i = 1; i < NUM_PROC; i++) {
    wait(status);
  }
  //This value should be (NUM_PROC -1) * NUM_ITERS 
  printf("c is %f\n", *c);
  printf("c should be %f\n", (NUM_PROC - 1) * NUM_ITERS);
  return 0;
}