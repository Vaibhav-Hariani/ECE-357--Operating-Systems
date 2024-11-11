#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
extern int errno;

int grep_cmd(int* grep_in, int* grep_out, char* pattern){
    //
    if(dup2(grep_in,STDIN_FILENO) < 0){
        fprintf(stderr, "Could not Complete dup2 for stdin: %s", strerror(errno));
        exit(1);
    }
    if(dup2(grep_out,STDOUT_FILENO) < 0){
        fprintf(stderr, "Could not Complete dup2 for stdout: %s", strerror(errno));
        exit(1);
    }

    int i = execl("grep", "grep", "pattern", NULL);
    if(i < 0){
        fprintf(stderr, "Exec call failed:  %s", strerror(errno));
        exit(1);
    }
}

int more_cmd(int* more_in){
    //
    if(dup2(more_in,STDIN_FILENO) < 0){
        fprintf(stderr, "Could not Complete dup2 for stdin: %s", strerror(errno));
        exit(1);
    }
    int i = execl("more", "more", NULL);
    if(i < 0){
        fprintf(stderr, "Exec call failed:  %s", strerror(errno));
        exit(1);
    }
}


int main(int argc, char** argv) {
    //Pattern is the second argument
    errno = 0;
    char* pattern = argv[1];
    for(int i = 2; i < argc; i++){
        int infile = open(argv[i], O_RDONLY);
        int grep_i[2];
        int more_i[2];

        if(errno < 0){
            fprintf(stderr, "Failed to open file: Reason %s\n", strerror(errno));
            exit(1);
        }
        if(pipe(grep_i) < 0 || pipe(more_i) < 0){
            fprintf(stderr, "Failed to create pipes: %s\n", strerror(errno));
            exit(1);
        }
        int grep = fork();
        switch(grep){
            case 0:
                //Clean stdin/stdout
                close(infile);
                grep_cmd(grep_i,more_i,pattern);
                break;
            case -1:
                fprintf(stderr, "Failed to create grep child: %s\n", strerror(errno));
                exit(1);
                break;
        }
        //grep will never reach here courtesy of the exit clause
        //While this might be programatically wrong, I don't want to have nested switch statements        
        int more = fork();
        switch(more){
            case 0:
                //Clean stdin/stdout
                close(infile);
                more_cmd(more_i);
                break;
            case -1:
                fprintf(stderr, "Failed to create grep child: %s\n", strerror(errno));
                exit(1);
                break;
        }
        
   

    }
}