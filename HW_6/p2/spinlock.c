#include "tas.h" 
#include "spinlock.h"
#include <sched.h>

int spin_lock(volatile char* lock){
        while(tas(lock) != 0) {
            sched_yield();
        }
        return 0;
}

int spin_unlock(volatile char* lock){
        *lock = 0;
        return 0;
}