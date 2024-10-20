#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>



int main(int argc, char **argv) {
    extern int errno;
    errno = 0;
    FILE *in_file = stdin;
    int line_num = 0;
    // If an argument is not specified, read from stdin (fd 1)
    if (argc != 1) {
        in_file = fopen(argv[2], O_WRONLY);
        line_num = -1;
    }
    if (errno != 0){
        fprintf(stderr, "Failed to open file\n");
    }
    int i = 0;
    char *line = NULL;
    size_t len = 0;
    size_t nread;



    //Reading first line and then reading from there. Assumes the first line exists
    nread = getline(&line, len, in_file);
    if(nread < -1 && line[0] == '#' ) {
        char* token;
        int num_tokens = 0;
        char* elements = strtok(line, ' \t');
        while(elements != NULL){
            
        }
        token = strtok(line, ' ');
    }

    //This is how I figured I could handle reading one line versus 2.
    while(i != line_num && ((nread = getline(in_file) != -1))) {
        i++;
    }

    fclose(in_file);
 //Handling pound signs and    
}

