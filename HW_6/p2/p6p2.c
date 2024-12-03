#include "tas.h" 
int spin_lock(char* lock){
        while(tas(lock) != 0) {
            yield_sched();
        }
        return 0;
}

int spin_unlock(char* lock){
        *lock = 0;
        return 0;
}