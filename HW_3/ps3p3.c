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

typedef struct processed_line {
    char* cmd;
    char* args[1000];  // Array of pointers to each argument.
    // Assuming a maximum of 1000 arguments (which might be a little overkill).
    char* redirs[5];  // Array of pointers to each redirect command.
    int num_args;
    int num_redirs;
} data;

void perform_redir(char* redir_cmd) {
    int final_dir;
    int input_dir;
    char* fname;
    switch (redir_cmd[0]) {
        case '<':
            final_dir = STDIN_FILENO;
            input_dir = open(&redir_cmd[1], O_RDONLY);
            break;
        case '>':
            final_dir = STDOUT_FILENO;
            if (redir_cmd[1] == '>') {
                input_dir = open(&redir_cmd[2], O_WRONLY | O_CREAT | O_APPEND, 0755);
            } else {
                input_dir = open(&redir_cmd[1], O_WRONLY | O_CREAT | O_TRUNC, 0755);
            }
            break;
        case '2':
            final_dir = STDERR_FILENO;
            if (redir_cmd[2] == '>') {
                input_dir = open(&redir_cmd[3], O_WRONLY | O_CREAT | O_APPEND, 0755);
            } else {
                input_dir = open(&redir_cmd[2], O_WRONLY | O_CREAT | O_TRUNC, 0755);
            }
        default: 
            //Can assume this is not a valid descriptor
            break;
    }

    dup2(input_dir, final_dir);
}

int process_cmd(data cmd_with_args) {
    // First, I/O Redirection
    errno = 0;
    for (int i = 0; i < cmd_with_args.num_redirs; i++) {
        char* redir_cmd = cmd_with_args.redirs[i];
        perform_redir(redir_cmd);
        if (errno != 0) {
            fprintf(stderr, "during redirection of file %s, encountered error: %s \n",redir_cmd, strerror(errno));
            exit(1);
            return 1;
        }
    }
    // fprintf(stderr, "Executing Command %s with args ", cmd_with_args.cmd);
    // for(int i = 0; i < cmd_with_args.num_args; i++) {
    //     fprintf(stderr, "\t %s", cmd_with_args.args[i]);
    // }
    // fprintf(stderr, "\t and with redirs", cmd_with_args.cmd);
    // for(int i = 0; i < cmd_with_args.num_redirs; i++) {
    //     fprintf(stderr, "\t %s", cmd_with_args.redirs[i]);
    // }

    // Execute command
    int i = execvp(cmd_with_args.cmd, cmd_with_args.args);
    if (i == -1) {
        fprintf(stderr, "call to %s call failed: %s \n", cmd_with_args.cmd, strerror(errno));
    }
    exit(127);
    return 127;
}

int main(int argc, char** argv) {
    extern int errno;
    errno = 0;
    int num_lines = 1;
    int i = 0;
    // If an argument is not specified, read from stdin (fd 1)
    FILE* in_file = stdin;
    if (argc != 1) {
        // fprintf(stderr, argv[1]);
        in_file = fopen(argv[1], "r");
        char c;
        if (errno != 0) {
            fprintf(stderr, "Failed to open file\n");
            return 0;
        }
        //Need to count number of lines in file to prevent rollover
        //I Don't know why this was happening with getline?
        //But this is the only way I could prevent it
        for (c = getc(in_file); c != EOF; c = getc(in_file))
            if (c == '\n') {
                num_lines++;
            }
            fclose(in_file);
            in_file = fopen(argv[1], "r");
    }

    char* line;

    int last_call = 0;
    int last_pid = 0;

    clock_t real_start, real_end;
    // For timing
    struct timeval user_start, user_end, sys_start, sys_end;

    size_t len = 0;
    size_t nread;
    // This is how I figured I could handle reading one line versus an entire file.
    while (i <= num_lines && (nread = getline(&line, &len, in_file) != -1)) {
        i++;
        //Skip blank lines
        if (line[0] == '\n' || line[0]== '#') {
            continue;
        }
        //Reset the errno for every line
        errno = 0;
        data line_data = {0};
        // Ignore all lines that start with #

        // Tokenizing and processing the call
        int on_args = -1;
        char* raw = strtok(line, " \t\n");
        line_data.cmd = raw;
        //First arg should be function itself
        line_data.args[0] = raw;
        line_data.num_args++;
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
            raw = strtok(0, " \t\n");
        }
        // The custom cd, pwd, and exit
        errno = 0;
        if (strcmp(line_data.cmd, "cd") == 0) {
            char* dir = getenv("HOME");
            if (line_data.num_args > 1) {
                dir = line_data.args[1];
            }
            chdir(dir);

        } else if (strcmp(line_data.cmd, "pwd") == 0) {
            char buf[2000];
            size_t size = 2000;
            getcwd(buf, size);
            printf("%s\n", buf);

        } else if (strcmp(line_data.cmd, "exit") == 0) {
            int ret = last_call;
            if (line_data.num_args > 1) {
                ret = atoi(line_data.args[1]);
            }
            exit(ret);

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
                    // The fork failed
                    fprintf(stderr, "Fork command failed \n");
                    // return -1;
                default:
                    wait(&last_call);
                    getrusage(RUSAGE_CHILDREN, &usage);
                    user_end = usage.ru_utime;
                    sys_end = usage.ru_stime;
                    real_end = clock();
                    if (WIFEXITED(last_call)) {
                        last_call = WEXITSTATUS(last_call);
                        fprintf(stderr, "Process %d exited with value %d\n", last_pid, last_call);
                    } else if (WIFSIGNALED(last_call)){
                        last_call = WTERMSIG(last_call);
                        fprintf(stderr, "Process %d exited with signal %d\n", last_pid, last_call);
                    } 
                    double real_time = (double) 1000 * (real_end - real_start) / CLOCKS_PER_SEC;
                    double user_time = (double)(user_end.tv_sec * 1000) + (user_end.tv_usec / 1000) - (user_start.tv_sec * 1000) - (user_start.tv_usec / 1000);
                    double sys_time = (double)(sys_end.tv_sec * 1000) + (sys_end.tv_usec / 1000) - (sys_start.tv_sec * 1000) - (sys_start.tv_usec / 1000);
                    fprintf(stderr, "statistics: \t user: %0.2fms, sys: %0.2fms, real: %0.2fms\n", user_time, sys_time, real_time);
            }
        }
        if (errno != 0) {
                fprintf(stderr, "Error executing %s: %s\n", line_data.cmd, strerror(errno));
        }
    }
    fclose(in_file);
    exit(last_call);
    return last_call;
}
