#define _XOPEN_SOURCE 700
#include <signal.h>
#include <stdlib.h>
#include <tas.h>
struct ll_head {
    struct ll_elem *first;
    char* spinlock; //Spinlock
    /* and maybe other stuff */
};
struct ll_elem {
    char* spinlock; //Spinlock
    struct ll_elem *fwd;
    /* and other stuff, such as the payload */
};
void ll_insert(struct ll_head *head, struct ll_elem *where,
               struct ll_elem *what) {
    while(TAS(&what -> spinlock) != 0);

    if (where) {
        while (TAS(&where->spinlock) !=0);
        what->fwd = where->fwd;
        where->fwd = what;
        where -> spinlock = 0;
    } else { 
        while (TAS(&head->spinlock) !=0);
        what->fwd = head->first;
        head->first = what;
        head->spinlock= 0;
    }
    what -> spinlock = 0;
}
