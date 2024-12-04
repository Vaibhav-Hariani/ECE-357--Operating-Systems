#define _XOPEN_SOURCE 700

#include "sem.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "spinlock.h"

void sem_init(struct sem *s, int count) {
    s->count = count;
    s->proc_count = 0;
    //Not sure if this is even necessary as mmap guarantees empty memory
    for(int i = 0; i < MAX_PROCS; i++){
        s->IDs[i] = 0;
    }
    spin_unlock(s->lock);
}

int sem_try(struct sem *s) {
    spin_lock(s->lock);
    if (s > 0) {
        s--;
        spin_unlock(s->lock);
        return 0;
    }
    spin_unlock(s->lock);
    return 1;
}

void sem_wait(struct sem *s, int vproc) {
    sigset_t full_mask;
    sigset_t suspend_mask;
    sigemptyset(&full_mask);
    sigemptyset(&suspend_mask);
    sigaddset(&full_mask,SIGUSR1);
    sigemptyset(&suspend_mask);
    while (sem_try(s) != 0) {
        spin_lock(s->lock);
        sigprocmask(SIG_BLOCK, &full_mask, NULL);
        s->IDs[vproc] = getpid();
        s->instrumentation[vproc].sleep++;
        s->proc_count++;
        spin_unlock(s->lock);
        sigsuspend(&suspend_mask);
        s->instrumentation[vproc].wake++;
    }
}

void sem_inc(struct sem *s){
    spin_lock(s->lock);
    s->count++;
    if(s->count > 0){
        for(int i = 0; i < MAX_PROCS; i++){
            if(s->IDs[i] == 1){
                kill(s->IDs[i],SIGUSR1);
            }
        }
    }
    spin_unlock(s->lock);
}