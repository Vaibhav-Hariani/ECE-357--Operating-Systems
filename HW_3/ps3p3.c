#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

typedef struct processed_line {
    char* cmd;
    char* args[1000];  // Array of pointers to each argument.
    // Assuming a maximum of 1000 arguments (which might be a little overkill).
    char* redirs[5];  // Array of pointers to each redirect command.
    int num_args;
    int num_redirs;
} data;


void perform_redir(char* redir_cmd){
    int final_dir;
    int input_dir;
    char* fname;
    switch(redir_cmd[0]){
        case '<':
            final_dir = STDIN_FILENO;
            input_dir = open(&redir_cmd[1], O_RDONLY);
            break;
        case '>':
            final_dir = STDOUT_FILENO;
            if(redir_cmd[1] == '>'){
                input_dir = open(&redir_cmd[2], O_WRONLY|O_CREAT|O_APPEND);
            } else {
                input_dir = open(&redir_cmd[1], O_WRONLY|O_CREAT|O_TRUNC);                
            }
            break;
        case '2':
            final_dir = STDERR_FILENO;
            if(redir_cmd[2] == '>'){
                input_dir = open(&redir_cmd[3], O_WRONLY|O_CREAT|O_APPEND);
            } else {
                input_dir = open(&redir_cmd[2], O_WRONLY|O_CREAT|O_TRUNC);                
            }
            break;
    }

    dup2(input_dir, final_dir);
}

int process_cmd(data cmd_with_args) {
    //First, I/O Redirection
    errno = 0;
    for(int i = 0; i < cmd_with_args.num_redirs; i++){
        char* redir_cmd = cmd_with_args.redirs[i];
        perform_redir(redir_cmd);
        if(errno != 0){
            fprintf(stderr, "during redirection, failure: %s \n", strerror(errno));
            exit(1);
            return 1;
        }
    }

    //Execute command
    int i = execvp(cmd_with_args.cmd, cmd_with_args.args);
    if(i == -1){
        fprintf(stderr, "exec call failed: %s \n", strerror(errno));
    }
    exit(127);
    return 127;
}


int main(int argc, char** argv) {
    extern int errno;
    errno = 0;
    int line_num = 1;

    // If an argument is not specified, read from stdin (fd 1)
    FILE* in_file = stdin;
    if (argc != 1) {
        in_file = fopen(argv[2], "w");
        line_num = -1;
    }
    if (errno != 0) {
        fprintf(stderr, "Failed to open file\n");
    }

    int i = 0;
    char* line;

    int last_call = 0;
    int last_pid = 0;

    clock_t real_start, real_end;
    //For timing
    struct timeval user_start, user_end, sys_start, sys_end;

    size_t len = 0;
    size_t nread;
    // This is how I figured I could handle reading one line versus an entire file.
    while (i != line_num && ((nread = getline(&line, &len, in_file) != -1))) {
        //Want this data "wiped" after every call
        data line_data = {0};
        i++;
        // Ignore all lines that start with #
        if (line[0] == '#') {
            continue;
        }
        // Tokenizing and processing the call
        int on_args = -1;
        char* raw = strtok(line, " \t");
        line_data.cmd = raw;
        // This tokenizes the newline, everything after the first element ()
        while (raw != NULL) {
            if (raw[0] == '<' || raw[0] == '>' || raw[1] == '<' || raw[1] == '>') {
                on_args = 0;
            }
            switch (on_args) {
                case 0:
                    line_data.redirs[line_data.num_redirs] = raw;
                    line_data.num_redirs++;
                    break;
                case 1:
                    line_data.args[line_data.num_args] = raw;
                    line_data.num_args++;
                    break;
                default:
                    // switch to on args
                    on_args = 1;
                    break;
            }
            raw = strtok(line, " \t");
        }
        //The custom commands
        // These couldn't really be replicated,
        // So they're just defined here
        errno = 0;
        if (strcmp(line_data.cmd, "cd") == 0) {
            char* dir = "$HOME";
            if (line_data.num_args > 0) {
                dir = line_data.args[0];
            }
            chdir(dir);
        } else if (strcmp(line_data.cmd, "pwd") == 0) {
            char* buf;
            size_t size;
            getcwd(buf, size);
            printf("%s", buf);
        } else if (strcmp(line_data.cmd, "exit") == 0) {
            int ret = last_call;
            if (line_data.num_args > 0) {
                ret = atoi(line_data.args[0]);
            }
            exit(ret);

        if(errno != 0){
            fprintf(stderr, "Error executing %s: %s\n",line_data.cmd, strerror(errno));
        }

        //Command we don't know
        } else {
            struct rusage usage;

            getrusage(RUSAGE_SELF, &usage);
            user_start = usage.ru_utime;
            sys_start = usage.ru_stime;
            real_start = clock();

            last_pid = fork();
            switch (last_pid) {
                case 0:
                    return process_cmd(line_data);
                    break;
                case -1:
                    //The fork failed
                    fprintf(stderr, "Fork command failed \n");
                    // return -1;
                default:
                    wait(&last_call);
                    rusage(RUSAGE_CHILDREN, &usage);
                    user_end = usage.ru_utime;
                    sys_end = usage.ru_stime;
                    real_end = clock();
                    if(WIFEXITED(last_call)){
                        fprintf(stderr, "Process %d exited with value %d\n", last_pid, WEXITSTATUS(last_call));
                    } else {
                        fprintf(stderr, "Process %d exited normally \n", last_pid, WEXITSTATUS(last_call));
                    }
                    fprintf(stderr, "statistics: ")

            }
        }
    }
    fclose(in_file);
    exit(last_call);
    return 0;
}

