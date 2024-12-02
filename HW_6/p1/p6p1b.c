#define _XOPEN_SOURCE 700
#include <signal.h>
#include <stdlib.h>
struct ll_head {
    struct ll_elem *first;
    /* and maybe other stuff */
};
struct ll_elem {
    struct ll_elem *fwd;
    /* and other stuff, such as the payload */
};
void ll_insert(struct ll_head *head, struct ll_elem *where,
               struct ll_elem *what) {
    //Straight from the notes
    sigset_t oldmask, newmask;
    sigfillset(&newmask); /* Set of all possible signals */
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    if (where) {
        what->fwd = where->fwd;
        where->fwd = what;
    } else {
        what->fwd = head->first;
        head->first = what;
    }

    //Straight from the notes
    sigprocmask(SIG_BLOCK, &oldmask, NULL);
}
