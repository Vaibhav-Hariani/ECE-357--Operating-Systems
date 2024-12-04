#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "sem.h"

int my_procnum;

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("NOT ENOUGH ARGUMENTS\n");
        return -1;
    }
    //Parent gets -1
    int my_procnum = -1;
    //Not doing any error handling here
    int num_pebbles = atoi(argv[1]);
    int num_moves = atoi(argv[2]);
    // 3 distinct shells
    struct sem* shell_A =
        mmap(0, sizeof(struct sem) * 3, PROT_READ | PROT_WRITE,
             MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    struct sem* shell_B = shell_A + 1;
    struct sem* shell_C = shell_A + 2;
    struct sem* shells[] = {shell_A, shell_B, shell_C};
    for(int i = 0; i < 3; i++){
        sem_init(shells[i],num_pebbles);
    }
    
}