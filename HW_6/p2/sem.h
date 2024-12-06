#ifndef ___SEM_H
#define  ___SEM_H
#define MAX_PROCS 6

struct data {
        int sleep;
        int woken;
};

struct sem {
    int count;
    int proc_count;
    int IDs[MAX_PROCS];
    struct data instrumentation[MAX_PROCS];
    char lock;
};
// Initialize the semaphore *s with the initial count. Initialize
// any underlying data structures. sem_init should only be called
// once in the program (per semaphore).
void sem_init(struct sem *s, int count);

// Attempt to perform the "P" operation (atomically decrement
// the semaphore). If this operation would block, return 0,
// otherwise return 1.
int sem_try(struct sem *s);

// Perform the P operation, blocking until successful.
void sem_wait(struct sem *s, int vproc);

// Perform the V operation. If any other tasks were sleeping
// on this semaphore, wake them by sending a SIGUSR1 to their
// process id
void sem_inc(struct sem *s);
#endif
