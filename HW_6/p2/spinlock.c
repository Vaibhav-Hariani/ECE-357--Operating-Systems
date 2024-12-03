#include "tas.h" 
#include "spinlock.h"
int spin_lock(volatile char* lock){
        while(tas(lock) != 0) {
            yield_sched();
        }
        return 0;
}

int spin_unlock(volatile char* lock){
        *lock = 0;
        return 0;
}