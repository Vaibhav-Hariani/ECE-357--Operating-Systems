#define _XOPEN_SOURCE 700

#include "sem.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "spinlock.h"
#include<stdio.h>

void sem_init(struct sem *s, int count) {
    s->count = count;
    s->proc_count = 0;
    //Not sure if this is even necessary as mmap guarantees empty memory
    for(int i = 0; i < MAX_PROCS; i++){
        s->IDs[i] = 0;
    }
}
//DOES NOT LOCK: LOCKING NEEDS TO BE HANDLED BEFORE
int sem_try(struct sem *s) {
    // spin_lock(&(s->lock));
    if (s->count > 0) {
        s->count--;
        // spin_unlock(&(s->lock));
        return 0;
    }
    // spin_unlock(&(s->lock));
    return 1;
}

void sem_wait(struct sem *s, int vproc) {
    sigset_t full_mask;
    sigset_t suspend_mask;
    sigemptyset(&full_mask);
    sigemptyset(&suspend_mask);
    sigaddset(&full_mask,SIGUSR1);
    sigprocmask(SIG_BLOCK, &full_mask, NULL);
    spin_lock(&(s->lock));
    while (sem_try(s) != 0) {
        // spin_lock(&(s->lock));
        s->IDs[vproc] = getpid();
        s->instrumentation[vproc].sleep++;
        s->proc_count++;
        spin_unlock(&(s->lock));
        sigsuspend(&suspend_mask);
        //Should not be necessary as each process manages its own instrumentation[vproc]
        spin_lock(&(s->lock));
        s->instrumentation[vproc].woken++;
        // spin_unlock(&(s->lock));
    }
    spin_unlock(&(s->lock));
}

void sem_inc(struct sem *s){
    spin_lock(&(s->lock));
    int val = s->count; 
    s->count++;
    if(val > 0){
        for(int i = 0; i < MAX_PROCS; i++){
            if(s->IDs[i] != 0){
                kill(s->IDs[i],SIGUSR1);
                s->IDs[i] = 0;
            }
        }
        s->proc_count = 0;
    }
    spin_unlock(&(s->lock));
}