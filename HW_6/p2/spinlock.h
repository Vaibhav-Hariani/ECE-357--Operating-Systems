#ifndef __SPINLOCK_H
#define __SPINLOCK_H
int spin_lock(volatile char *lock);
int spin_unlock(volatile char *lock);
#endif
