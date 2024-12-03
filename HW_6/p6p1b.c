struct ll_head {
  struct ll_elem *first;
  /* and maybe some other stuff*/
};

struct ll_elem {
  struct ll_elem *fwd;
  /* and other stuff, e.g payload*/
};

void ll_insert(struct ll_head *head, struct ll_elem *where,
               struct ll_elem *what) {
  if (where) {
    what->fwd = where->fwd;
    where->fwd = what;
  } else {
    // Wasn't sure if this was a bug so I replaced it
    what->fwd = head->first;
    head->first = what;
  }
}